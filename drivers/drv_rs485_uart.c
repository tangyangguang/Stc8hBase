#include "drv_rs485_uart.h"

#if DRV_RS485_UART_ENABLE_WRITE
#include "stc8h_delay.h"
#endif

#ifndef BOARD_RS485_TX_ENABLE
#error "BOARD_RS485_TX_ENABLE() must be defined by board_pins.h."
#endif

#ifndef BOARD_RS485_RX_ENABLE
#error "BOARD_RS485_RX_ENABLE() must be defined by board_pins.h."
#endif

void drv_rs485_uart_enter_tx(void)
{
    BOARD_RS485_TX_ENABLE();
}

void drv_rs485_uart_enter_rx(void)
{
    BOARD_RS485_RX_ENABLE();
}

stc8h_status_t drv_rs485_uart_init(stc8h_uart_id_t uart)
{
    stc8h_status_t status;

    status = stc8h_uart_init(uart);
    if (status != STC8H_OK) {
        return status;
    }

    drv_rs485_uart_enter_rx();
#if DRV_RS485_UART_ENABLE_RX_INTERRUPT
    stc8h_uart_clear_tx_flag(uart);
    if (stc8h_uart_interrupt_enable(uart) != STC8H_OK) {
        return STC8H_ERROR;
    }
#endif
    return STC8H_OK;
}

#if DRV_RS485_UART_ENABLE_WRITE
stc8h_status_t drv_rs485_uart_write(stc8h_uart_id_t uart, const stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u16 i;

    if (len == 0u) {
        return STC8H_OK;
    }
    if (data == 0) {
        return STC8H_ERROR;
    }

#if DRV_RS485_UART_ENABLE_RX_INTERRUPT
    if (stc8h_uart_interrupt_disable(uart) != STC8H_OK) {
        return STC8H_ERROR;
    }
#endif

    drv_rs485_uart_enter_tx();
    for (i = 0u; i < len; ++i) {
        if (stc8h_uart_putc_bounded(uart,
                                    (char)data[i],
                                    (stc8h_u16)DRV_RS485_UART_TX_POLL_LIMIT) != STC8H_OK) {
            drv_rs485_uart_enter_rx();
#if DRV_RS485_UART_ENABLE_RX_INTERRUPT
            stc8h_uart_clear_tx_flag(uart);
            (void)stc8h_uart_interrupt_enable(uart);
#endif
            return STC8H_ERROR;
        }
    }
    stc8h_delay_us((stc8h_u16)DRV_RS485_UART_TX_COMPLETE_DELAY_US);
    drv_rs485_uart_enter_rx();
#if DRV_RS485_UART_ENABLE_RX_INTERRUPT
    stc8h_uart_clear_tx_flag(uart);
    if (stc8h_uart_interrupt_enable(uart) != STC8H_OK) {
        return STC8H_ERROR;
    }
#endif

    return STC8H_OK;
}
#endif

#if DRV_RS485_UART_ENABLE_READ
stc8h_u8 drv_rs485_uart_readable(stc8h_uart_id_t uart)
{
    return stc8h_uart_readable(uart);
}

stc8h_u8 drv_rs485_uart_getc(stc8h_uart_id_t uart)
{
    return (stc8h_u8)stc8h_uart_getc(uart);
}
#endif
