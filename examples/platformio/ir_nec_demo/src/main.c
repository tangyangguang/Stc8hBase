#include "drv_ir_rx.h"
#include "drv_ir_tx.h"
#include "stc8h_uart.h"

static stc8h_u8 feed_tx_to_rx(drv_ir_rx_t *rx, stc8h_u8 address, stc8h_u8 command)
{
    drv_ir_tx_nec_t tx;
    drv_ir_tx_level_t level;
    stc8h_u16 duration;
    stc8h_u8 count;

    drv_ir_tx_nec_begin(&tx, address, command);
    count = 0u;
    while (1) {
        level = drv_ir_tx_nec_next(&tx, &duration);
        if (level == DRV_IR_TX_DONE) {
            break;
        }
        drv_ir_rx_feed_pulse(rx, (level == DRV_IR_TX_MARK) ? 0u : 1u, duration);
        ++count;
    }

    return count;
}

static stc8h_u8 test_nec(void)
{
    drv_ir_rx_t rx;
    drv_ir_rx_frame_t frame;
    drv_ir_rx_event_t event;
    stc8h_u8 count;

    drv_ir_rx_init(&rx);
    count = feed_tx_to_rx(&rx, 0x10u, 0xEFu);
    event = drv_ir_rx_get_event(&rx, &frame);
    if (count != 67u) {
        return 0u;
    }
    if (event != DRV_IR_RX_EVENT_FRAME) {
        return 0u;
    }
    if ((frame.address != 0x10u) || (frame.command != 0xEFu)) {
        return 0u;
    }

    drv_ir_rx_feed_pulse(&rx, 0u, 9000u);
    drv_ir_rx_feed_pulse(&rx, 1u, 2250u);
    event = drv_ir_rx_get_event(&rx, 0);
    if (event != DRV_IR_RX_EVENT_REPEAT) {
        return 0u;
    }

    drv_ir_rx_feed_pulse(&rx, 0u, 9000u);
    drv_ir_rx_feed_pulse(&rx, 1u, 4500u);
    drv_ir_rx_feed_pulse(&rx, 0u, 2000u);
    event = drv_ir_rx_get_event(&rx, 0);
    if (event != DRV_IR_RX_EVENT_ERROR) {
        return 0u;
    }

    return 1u;
}

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "ir nec ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "ir nec error\r\n");
    }
}

void main(void)
{
    stc8h_u8 ok;

    (void)stc8h_uart_init(STC8H_UART1);
    ok = test_nec();

    while (1) {
        print_result(ok);
    }
}
