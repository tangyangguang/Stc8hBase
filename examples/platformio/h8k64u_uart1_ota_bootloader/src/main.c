#include "proto_ota_frame.h"
#include "stc8h_boot_stub.h"
#include "stc8h_iap_ota_params.h"
#include "stc8h_iap_program.h"
#include "stc8h_ota.h"
#include "stc8h_ota_params_store.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#ifndef H8K64U_OTA_LOCAL_ADDR
#define H8K64U_OTA_LOCAL_ADDR 0x22u
#endif

#define H8K64U_OTA_STATUS_PAYLOAD_LEN 8u
#define H8K64U_OTA_IAP_CONTR_SWRST 0x20u

typedef void (*h8k64u_ota_app_entry_t)(void);

static STC8H_XDATA stc8h_ota_params_store_t params_store;
static STC8H_XDATA stc8h_ota_context_t ota_ctx;
static STC8H_XDATA stc8h_u8 rx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static STC8H_XDATA stc8h_u8 tx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static stc8h_u16 rx_len;
static stc8h_u16 rx_expected_len;
static stc8h_u16 last_seq = 0xFFFFu;

static const stc8h_ota_backend_t app_backend = {
    stc8h_iap_program_erase_sector,
    stc8h_iap_program_write,
    stc8h_iap_program_read,
    STC8H_IAP_PROGRAM_SECTOR_SIZE
};

static stc8h_u16 boot_get_le16(const stc8h_u8 *bytes)
{
    return (stc8h_u16)(((stc8h_u16)bytes[1] << 8) | bytes[0]);
}

static void boot_safe_outputs_off(void)
{
}

static void boot_jump_to_app(void)
{
    EA = 0u;
    SP = 0x07u;
    ((h8k64u_ota_app_entry_t)STC8H_BOOT_APP_BASE)();
}

static void boot_software_reset(void)
{
    EA = 0u;
    IAP_CONTR = H8K64U_OTA_IAP_CONTR_SWRST;
    while (1) {
    }
}

static void boot_jump_existing_app_if_allowed(void)
{
    static STC8H_XDATA stc8h_ota_params_t params;
    stc8h_ota_boot_action_t action;

    if (stc8h_ota_params_store_load_active(&params_store, &params) != STC8H_OK) {
        return;
    }

    action = stc8h_ota_get_boot_action(&params);
    if (action == STC8H_OTA_BOOT_ACTION_JUMP_APP) {
        boot_jump_to_app();
    }
    if (action == STC8H_OTA_BOOT_ACTION_TRIAL_APP) {
        if (stc8h_ota_params_store_mark_boot_attempted(&params_store) == STC8H_OK) {
            boot_jump_to_app();
        }
    }
}

static void boot_reset_rx_frame(void)
{
    rx_len = 0u;
    rx_expected_len = 0u;
}

static void boot_uart1_write_bytes(const stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        stc8h_uart_putc(STC8H_UART1, (char)data[i]);
    }
}

static void boot_uart1_write_banner(void)
{
    stc8h_uart_putc(STC8H_UART1, 'B');
    stc8h_uart_putc(STC8H_UART1, 'O');
    stc8h_uart_putc(STC8H_UART1, 'O');
    stc8h_uart_putc(STC8H_UART1, 'T');
    stc8h_uart_putc(STC8H_UART1, '\r');
    stc8h_uart_putc(STC8H_UART1, '\n');
}

static stc8h_u8 boot_read_code_byte(stc8h_u16 addr)
{
    const STC8H_CODE stc8h_u8 *ptr;

    ptr = (const STC8H_CODE stc8h_u8 *)addr;
    return *ptr;
}

static stc8h_status_t boot_send_status(const proto_ota_frame_t *request, stc8h_u8 status)
{
    static STC8H_XDATA stc8h_u8 payload[H8K64U_OTA_STATUS_PAYLOAD_LEN];
    stc8h_u16 frame_len;

    if (request == 0) {
        return STC8H_ERROR;
    }

    payload[0] = request->cmd;
    payload[1] = status;
    payload[2] = (stc8h_u8)stc8h_ota_get_status(&ota_ctx);
    payload[3] = ota_ctx.fail_reason;
    if (request->cmd == PROTO_OTA_FRAME_CMD_COMMIT) {
        payload[4] = boot_read_code_byte(STC8H_BOOT_APP_BASE);
        payload[5] = boot_read_code_byte((stc8h_u16)(STC8H_BOOT_APP_BASE + 1u));
        payload[6] = boot_read_code_byte((stc8h_u16)(STC8H_BOOT_APP_BASE + 2u));
        payload[7] = boot_read_code_byte((stc8h_u16)(STC8H_BOOT_APP_BASE + 3u));
    } else {
        payload[4] = (stc8h_u8)(ota_ctx.manifest.app_size & 0xFFUL);
        payload[5] = (stc8h_u8)((ota_ctx.manifest.app_size >> 8) & 0xFFUL);
        payload[6] = (stc8h_u8)(request->len & 0xFFu);
        payload[7] = (stc8h_u8)((request->len >> 8) & 0xFFu);
    }

    if (proto_ota_frame_build(tx_frame,
                              sizeof(tx_frame),
                              request->src,
                              H8K64U_OTA_LOCAL_ADDR,
                              PROTO_OTA_FRAME_CMD_STATUS,
                              request->seq,
                              ota_ctx.write_offset,
                              payload,
                              sizeof(payload),
                              &frame_len) != STC8H_OK) {
        return STC8H_ERROR;
    }

    boot_uart1_write_bytes(tx_frame, frame_len);
    return STC8H_OK;
}

static stc8h_status_t boot_handle_begin(const proto_ota_frame_t *frame)
{
    static STC8H_XDATA stc8h_ota_manifest_t manifest;

    if ((frame == 0) || (frame->len != STC8H_OTA_MANIFEST_WIRE_SIZE)) {
        return STC8H_ERROR;
    }
    if (stc8h_ota_manifest_decode(frame->payload, frame->len, &manifest) != STC8H_OK) {
        return STC8H_ERROR;
    }
    return stc8h_ota_begin(&ota_ctx, &manifest);
}

static stc8h_status_t boot_handle_command(const proto_ota_frame_t *frame)
{
    if (frame == 0) {
        return STC8H_ERROR;
    }

    switch (frame->cmd) {
    case PROTO_OTA_FRAME_CMD_BEGIN:
        return boot_handle_begin(frame);
    case PROTO_OTA_FRAME_CMD_WRITE_BLOCK:
        if (frame->offset > 0xFFFFUL) {
            return STC8H_ERROR;
        }
        return stc8h_ota_write_chunk(&ota_ctx,
                                     (stc8h_u16)frame->offset,
                                     frame->payload,
                                     frame->len);
    case PROTO_OTA_FRAME_CMD_VERIFY:
        return (frame->len == 0u) ? stc8h_ota_verify(&ota_ctx) : STC8H_ERROR;
    case PROTO_OTA_FRAME_CMD_COMMIT:
        return (frame->len == 0u) ? stc8h_ota_commit(&ota_ctx) : STC8H_ERROR;
    case PROTO_OTA_FRAME_CMD_ABORT:
        return stc8h_ota_abort(&ota_ctx, 1u);
    default:
        return STC8H_ERROR;
    }
}

static void boot_process_complete_frame(void)
{
    proto_ota_frame_t frame;
    proto_ota_frame_parse_result_t parse_result;
    stc8h_status_t status;

    parse_result = proto_ota_frame_parse(rx_frame,
                                         rx_len,
                                         H8K64U_OTA_LOCAL_ADDR,
                                         last_seq,
                                         &frame);
    if (parse_result == PROTO_OTA_FRAME_PARSE_OK) {
        last_seq = frame.seq;
        status = boot_handle_command(&frame);
        (void)boot_send_status(&frame,
                               (status == STC8H_OK) ?
                               PROTO_OTA_FRAME_STATUS_OK :
                               PROTO_OTA_FRAME_STATUS_ERROR);
        if ((status == STC8H_OK) && (frame.cmd == PROTO_OTA_FRAME_CMD_COMMIT)) {
            boot_software_reset();
        }
    } else if (parse_result == PROTO_OTA_FRAME_PARSE_DUPLICATE) {
        (void)boot_send_status(&frame, PROTO_OTA_FRAME_STATUS_DUPLICATE);
    }

    boot_reset_rx_frame();
}

static void boot_feed_byte(stc8h_u8 value)
{
    stc8h_u16 payload_len;

    if ((rx_len == 0u) && (value != PROTO_OTA_FRAME_SOF0)) {
        return;
    }
    if ((rx_len == 1u) && (value != PROTO_OTA_FRAME_SOF1)) {
        boot_reset_rx_frame();
        return;
    }

    if (rx_len >= PROTO_OTA_FRAME_WIRE_MAX) {
        boot_reset_rx_frame();
        return;
    }
    rx_frame[rx_len] = value;
    ++rx_len;

    if (rx_len == PROTO_OTA_FRAME_HEADER_SIZE) {
        payload_len = boot_get_le16(&rx_frame[12]);
        if (payload_len > PROTO_OTA_FRAME_PAYLOAD_MAX) {
            boot_reset_rx_frame();
            return;
        }
        rx_expected_len = (stc8h_u16)(PROTO_OTA_FRAME_OVERHEAD + payload_len);
    }

    if ((rx_expected_len != 0u) && (rx_len == rx_expected_len)) {
        boot_process_complete_frame();
    }
}

void main(void)
{
    boot_safe_outputs_off();
    stc8h_ota_params_store_init(&params_store,
                                stc8h_iap_ota_params_erase,
                                stc8h_iap_ota_params_write,
                                stc8h_iap_ota_params_read);
    stc8h_ota_init(&ota_ctx, &app_backend, &params_store);
    (void)stc8h_uart_init(STC8H_UART1);
    boot_uart1_write_banner();
    boot_reset_rx_frame();
    boot_jump_existing_app_if_allowed();

    while (1) {
        if (stc8h_uart_readable(STC8H_UART1) != 0u) {
            boot_feed_byte((stc8h_u8)stc8h_uart_getc(STC8H_UART1));
        }
    }
}
