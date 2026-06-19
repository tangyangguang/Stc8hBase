#include "drv_rs485_uart.h"

#if DRV_RS485_UART_TURNAROUND_DELAY_US != 0u
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

    drv_rs485_uart_enter_tx();
    for (i = 0u; i < len; ++i) {
        stc8h_uart_putc(uart, (char)data[i]);
    }
#if DRV_RS485_UART_TURNAROUND_DELAY_US != 0u
    stc8h_delay_us(DRV_RS485_UART_TURNAROUND_DELAY_US);
#endif
    drv_rs485_uart_enter_rx();

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
