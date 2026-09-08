#include "proto_ota_frame.h"
#include "stc8h_uart.h"

#ifndef H8K64U_UART3_OTA_ADDR
#define H8K64U_UART3_OTA_ADDR 0x22u
#endif

static STC8H_XDATA stc8h_u8 rx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static STC8H_XDATA stc8h_u8 tx_frame[PROTO_OTA_FRAME_WIRE_MAX];
static STC8H_XDATA proto_ota_frame_collector_t collector;

static void uart3_write_bytes(const stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        stc8h_uart_putc(BOARD_RF433_UART, (char)data[i]);
    }
}

static void uart3_process_frame(void)
{
    proto_ota_frame_t request;
    stc8h_u8 payload[2];
    stc8h_u16 frame_len;

    if (proto_ota_frame_parse(rx_frame, collector.length,
                              H8K64U_UART3_OTA_ADDR, &request) !=
        PROTO_OTA_FRAME_PARSE_OK) {
        proto_ota_frame_collector_reset(&collector);
        return;
    }
    payload[0] = request.cmd;
    payload[1] = PROTO_OTA_FRAME_STATUS_OK;
    if (proto_ota_frame_build(
            tx_frame, sizeof(tx_frame), request.src, H8K64U_UART3_OTA_ADDR,
            PROTO_OTA_FRAME_CMD_STATUS, 0u, request.session_id, request.seq,
            request.offset, payload, sizeof(payload), &frame_len) == STC8H_OK) {
        uart3_write_bytes(tx_frame, frame_len);
    }
    proto_ota_frame_collector_reset(&collector);
}

void main(void)
{
    proto_ota_collect_result_t result;

    (void)stc8h_uart_init(BOARD_RF433_UART);
    proto_ota_frame_collector_init(&collector, rx_frame, sizeof(rx_frame));
    while (1) {
        if (stc8h_uart_readable(BOARD_RF433_UART) != 0u) {
            result = proto_ota_frame_collector_feed(
                &collector, (stc8h_u8)stc8h_uart_getc(BOARD_RF433_UART));
            if (result == PROTO_OTA_COLLECT_FRAME) {
                uart3_process_frame();
            }
        }
    }
}
