#include "proto_ota_frame.h"
#include "stc8h_uart.h"

#ifndef H8K64U_UART3_OTA_ADDR
#define H8K64U_UART3_OTA_ADDR 0x22u
#endif

static STC8H_XDATA stc8h_u8 rx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static STC8H_XDATA stc8h_u8 tx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static stc8h_u16 rx_len;
static stc8h_u16 rx_expected_len;
static stc8h_u16 last_seq = 0xFFFFu;

static stc8h_u16 uart3_ota_get_le16(const stc8h_u8 *bytes)
{
    return (stc8h_u16)(((stc8h_u16)bytes[1] << 8) | bytes[0]);
}

static void uart3_write_bytes(const stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        stc8h_uart_putc(BOARD_RF433_UART, (char)data[i]);
    }
}

static void uart3_ota_reset_rx(void)
{
    rx_len = 0u;
    rx_expected_len = 0u;
}

static void uart3_ota_send_status(const proto_ota_frame_t *request, stc8h_u8 status)
{
    stc8h_u8 payload[2];
    stc8h_u16 frame_len;

    if (request == 0) {
        return;
    }

    payload[0] = request->cmd;
    payload[1] = status;
    if (proto_ota_frame_build(tx_frame,
                              sizeof(tx_frame),
                              request->src,
                              H8K64U_UART3_OTA_ADDR,
                              PROTO_OTA_FRAME_CMD_STATUS,
                              request->seq,
                              request->offset,
                              payload,
                              sizeof(payload),
                              &frame_len) == STC8H_OK) {
        uart3_write_bytes(tx_frame, frame_len);
    }
}

static void uart3_ota_process_frame(void)
{
    proto_ota_frame_t frame;
    proto_ota_frame_parse_result_t result;

    result = proto_ota_frame_parse(rx_frame,
                                   rx_len,
                                   H8K64U_UART3_OTA_ADDR,
                                   last_seq,
                                   &frame);
    if (result == PROTO_OTA_FRAME_PARSE_OK) {
        last_seq = frame.seq;
        uart3_ota_send_status(&frame, PROTO_OTA_FRAME_STATUS_OK);
    } else if (result == PROTO_OTA_FRAME_PARSE_DUPLICATE) {
        uart3_ota_send_status(&frame, PROTO_OTA_FRAME_STATUS_DUPLICATE);
    }

    uart3_ota_reset_rx();
}

static void uart3_ota_feed_byte(stc8h_u8 value)
{
    stc8h_u16 payload_len;

    if ((rx_len == 0u) && (value != PROTO_OTA_FRAME_SOF0)) {
        return;
    }
    if ((rx_len == 1u) && (value != PROTO_OTA_FRAME_SOF1)) {
        uart3_ota_reset_rx();
        return;
    }
    if (rx_len >= PROTO_OTA_FRAME_WIRE_MAX) {
        uart3_ota_reset_rx();
        return;
    }

    rx_frame[rx_len] = value;
    ++rx_len;

    if (rx_len == PROTO_OTA_FRAME_HEADER_SIZE) {
        payload_len = uart3_ota_get_le16(&rx_frame[12]);
        if (payload_len > PROTO_OTA_FRAME_PAYLOAD_MAX) {
            uart3_ota_reset_rx();
            return;
        }
        rx_expected_len = (stc8h_u16)(PROTO_OTA_FRAME_OVERHEAD + payload_len);
    }

    if ((rx_expected_len != 0u) && (rx_len == rx_expected_len)) {
        uart3_ota_process_frame();
    }
}

void main(void)
{
    (void)stc8h_uart_init(BOARD_RF433_UART);
    uart3_ota_reset_rx();

    while (1) {
        if (stc8h_uart_readable(BOARD_RF433_UART) != 0u) {
            uart3_ota_feed_byte((stc8h_u8)stc8h_uart_getc(BOARD_RF433_UART));
        }
    }
}
