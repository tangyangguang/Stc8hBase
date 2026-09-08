#include "drv_rs485_uart.h"
#include "proto_ota_frame.h"
#include "stc8h_boot_stub.h"
#include "stc8h_chip_identity.h"
#include "stc8h_iap_ota_params.h"
#include "stc8h_iap_program.h"
#include "stc8h_ota.h"
#include "stc8h_ota_params_store.h"
#include "stc8h_ota_receiver.h"
#include "stc8h_sfr.h"
#include "stc8h_wdt.h"
#include "util_crc32.h"

/* Keep the size-critical receiver and transport adapter in one SDCC unit. */
#include "../../../../protocols/stc8h_ota_receiver.c"

#ifndef H8K64U_OTA_LOCAL_ADDR
#define H8K64U_OTA_LOCAL_ADDR 0x22u
#endif

#define BOOT_IAP_CONTR_SWRST 0x20u
#define BOOT_READ_CHUNK_SIZE 16u

typedef void (*boot_app_entry_t)(void);

static STC8H_XDATA stc8h_ota_params_store_t params_store;
static STC8H_XDATA stc8h_ota_context_t ota_ctx;
static STC8H_XDATA proto_ota_frame_collector_t collector;
static STC8H_XDATA stc8h_u8 rx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static STC8H_XDATA stc8h_u8 tx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static STC8H_XDATA stc8h_u8 boot_uid[STC8H_DEVICE_UID_SIZE];
static stc8h_u32 last_session_id;
static stc8h_u16 last_seq;
static stc8h_u8 last_cmd;
static stc8h_u8 last_status;
static stc8h_u8 has_last;
static stc8h_u8 boot_uid_valid;

static void boot_keep_alive(void);

static const stc8h_ota_backend_t app_backend = {
    stc8h_iap_program_erase_sector,
    stc8h_iap_program_write,
    stc8h_iap_program_read,
    boot_keep_alive,
    STC8H_IAP_PROGRAM_SECTOR_SIZE
};

static void boot_keep_alive(void)
{
    stc8h_wdt_feed();
}

static void boot_put_le16(stc8h_u8 *bytes, stc8h_u16 value)
{
    bytes[0] = (stc8h_u8)value;
    bytes[1] = (stc8h_u8)(value >> 8);
}

static void boot_put_le32(stc8h_u8 *bytes, stc8h_u32 value)
{
    bytes[0] = (stc8h_u8)value;
    bytes[1] = (stc8h_u8)(value >> 8);
    bytes[2] = (stc8h_u8)(value >> 16);
    bytes[3] = (stc8h_u8)(value >> 24);
}

static void boot_jump_to_app(void)
{
    EA = 0u;
    SP = 0x07u;
    ((boot_app_entry_t)STC8H_BOOT_APP_BASE)();
}

static void boot_software_reset(void)
{
    EA = 0u;
    IAP_CONTR = BOOT_IAP_CONTR_SWRST;
    while (1) {
    }
}

static stc8h_u8 boot_app_crc_matches(const stc8h_ota_params_t *params)
{
    static STC8H_XDATA stc8h_u8 buffer[BOOT_READ_CHUNK_SIZE];
    stc8h_u16 offset;
    stc8h_u16 remaining;
    stc8h_u32 crc;
    stc8h_u16 read_len;

    if (params == 0) {
        return 0u;
    }
    offset = 0u;
    remaining = params->app_size;
    crc = 0UL;
    while (remaining != 0u) {
        read_len = remaining > BOOT_READ_CHUNK_SIZE ?
                   BOOT_READ_CHUNK_SIZE : remaining;
        if (app_backend.read((stc8h_u16)(params->app_base + offset),
                             buffer, read_len) != STC8H_OK) {
            return 0u;
        }
        boot_keep_alive();
        crc = util_crc32_ieee_update(crc, buffer, read_len);
        offset = (stc8h_u16)(offset + read_len);
        remaining = (stc8h_u16)(remaining - read_len);
    }
    return crc == params->app_crc32 ? 1u : 0u;
}

static void boot_jump_existing_app_if_allowed(void)
{
    static STC8H_XDATA stc8h_ota_params_t params;
    stc8h_ota_boot_action_t action;

    if (stc8h_ota_params_store_load_active(&params_store, &params) !=
        STC8H_OK) {
        return;
    }
    action = stc8h_ota_get_boot_action(&params);
    if ((action == STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER) ||
        (boot_app_crc_matches(&params) == 0u)) {
        return;
    }
    if (action == STC8H_OTA_BOOT_ACTION_JUMP_APP) {
        boot_jump_to_app();
    }
    if ((action == STC8H_OTA_BOOT_ACTION_TRIAL_APP) &&
        (stc8h_ota_params_store_mark_trial_started(&params_store) ==
         STC8H_OK)) {
        boot_jump_to_app();
    }
}

static void boot_send_status(const proto_ota_frame_t *request,
                             stc8h_u8 status)
{
    static STC8H_XDATA stc8h_u8 payload[STC8H_OTA_RECEIVER_INFO_SIZE];
    stc8h_u16 frame_len;
    stc8h_u16 payload_len;
    stc8h_u8 index;

    payload[0] = request->cmd;
    payload[1] = status;
    payload[2] = (stc8h_u8)ota_ctx.state;
    payload[3] = ota_ctx.fail_reason;
    payload[4] = STC8H_OTA_BOOTLOADER_VERSION;
    payload[5] = PROTO_OTA_FRAME_VERSION;
    payload[6] = H8K64U_OTA_LOCAL_ADDR;
    payload[7] = 0u;
    boot_put_le16(&payload[8], ota_ctx.write_offset);
    boot_put_le16(&payload[10], ota_ctx.manifest.app_size);
    boot_put_le16(&payload[12], ota_ctx.manifest.manifest_crc);
    boot_put_le16(&payload[14], ota_ctx.manifest.build_number);
    boot_put_le32(&payload[16], ota_ctx.session_id);
    payload_len = STC8H_OTA_RECEIVER_STATUS_SIZE;
    if (request->cmd == PROTO_OTA_FRAME_CMD_INFO) {
        for (index = 0u; index < STC8H_DEVICE_UID_SIZE; ++index) {
            payload[STC8H_OTA_RECEIVER_STATUS_SIZE + index] = boot_uid[index];
        }
        payload_len = STC8H_OTA_RECEIVER_INFO_SIZE;
    }
    if (proto_ota_frame_build(
            tx_frame, sizeof(tx_frame), request->src,
            H8K64U_OTA_LOCAL_ADDR, PROTO_OTA_FRAME_CMD_STATUS, 0u,
            request->session_id, request->seq, ota_ctx.write_offset,
            payload, payload_len, &frame_len) == STC8H_OK) {
        (void)drv_rs485_uart_write(BOARD_RS485_UART, tx_frame, frame_len);
    }
}

static void boot_process_frame(void)
{
    static STC8H_XDATA proto_ota_frame_t request;
    stc8h_u8 status;
    stc8h_u8 reset_requested;

    if (proto_ota_frame_parse(rx_frame, collector.length,
                              H8K64U_OTA_LOCAL_ADDR, &request) !=
        PROTO_OTA_FRAME_PARSE_OK) {
        proto_ota_frame_collector_reset(&collector);
        return;
    }
    if ((has_last != 0u) && (last_session_id == request.session_id) &&
        (last_seq == request.seq) && (last_cmd == request.cmd)) {
        status = last_status == PROTO_OTA_FRAME_STATUS_OK ?
                 PROTO_OTA_FRAME_STATUS_DUPLICATE : last_status;
        reset_requested = 0u;
    } else if ((boot_uid_valid == 0u) &&
               (request.cmd != PROTO_OTA_FRAME_CMD_INFO) &&
               (request.cmd != PROTO_OTA_FRAME_CMD_STATUS)) {
        status = PROTO_OTA_FRAME_STATUS_ERROR;
        reset_requested = 0u;
    } else {
        status = stc8h_ota_receiver_handle(&ota_ctx, boot_uid, &request,
                                            &reset_requested);
        last_session_id = request.session_id;
        last_seq = request.seq;
        last_cmd = request.cmd;
        last_status = status;
        has_last = 1u;
    }
    boot_send_status(&request, status);
    proto_ota_frame_collector_reset(&collector);
    if (reset_requested != 0u) {
        boot_software_reset();
    }
}

void main(void)
{
    proto_ota_collect_result_t collect_result;

    H8K64U_OTA_SAFE_OUTPUTS_OFF();
    stc8h_wdt_enable(STC8H_WDT_SCALE_256, 0u);
    boot_uid_valid = (stc8h_chip_identity_read_uid(boot_uid) == STC8H_OK) ?
                     1u : 0u;
    stc8h_ota_params_store_init(&params_store,
                                stc8h_iap_ota_params_erase,
                                stc8h_iap_ota_params_write,
                                stc8h_iap_ota_params_read);
    stc8h_ota_init(&ota_ctx, &app_backend, &params_store);
    (void)stc8h_ota_restore(&ota_ctx);
    boot_jump_existing_app_if_allowed();

    (void)drv_rs485_uart_init(BOARD_RS485_UART);
    proto_ota_frame_collector_init(&collector, rx_frame, sizeof(rx_frame));
    has_last = 0u;
    while (1) {
        if (drv_rs485_uart_readable(BOARD_RS485_UART) != 0u) {
            collect_result = proto_ota_frame_collector_feed(
                &collector, drv_rs485_uart_getc(BOARD_RS485_UART));
            if (collect_result == PROTO_OTA_COLLECT_FRAME) {
                boot_process_frame();
            }
        }
    }
}
