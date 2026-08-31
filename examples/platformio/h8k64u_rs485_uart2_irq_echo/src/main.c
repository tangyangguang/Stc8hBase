#include "drv_rs485_uart.h"
#include "stc8h_interrupt.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#define RX_BUFFER_SIZE 32u
#define RS485_DIR_MASK 0x10u

static STC8H_XDATA volatile stc8h_u8 rx_buffer[RX_BUFFER_SIZE];
static volatile stc8h_u8 rx_head;
static volatile stc8h_u8 rx_tail;
static volatile stc8h_u8 rx_overflow;

STC8H_INTERRUPT(uart2_isr, STC8H_VECTOR_UART2)
{
    stc8h_u8 value;
    stc8h_u8 next;

    if (stc8h_uart_try_getc(STC8H_UART2, &value) != 0u) {
        next = (stc8h_u8)((rx_head + 1u) & (RX_BUFFER_SIZE - 1u));
        if (next == rx_tail) {
            rx_overflow = 1u;
        } else {
            rx_buffer[rx_head] = value;
            rx_head = next;
        }
    }
    stc8h_uart_clear_tx_flag(STC8H_UART2);
}

static void board_rs485_direction_init(void)
{
    P4 &= (stc8h_u8)~RS485_DIR_MASK;
    P4M1 &= (stc8h_u8)~RS485_DIR_MASK;
    P4M0 |= RS485_DIR_MASK;
}

static stc8h_u8 app_try_pop(stc8h_u8 *value)
{
    if (value == 0 || rx_head == rx_tail) {
        return 0u;
    }
    *value = rx_buffer[rx_tail];
    rx_tail = (stc8h_u8)((rx_tail + 1u) & (RX_BUFFER_SIZE - 1u));
    return 1u;
}

void main(void)
{
    stc8h_u8 value;

    board_rs485_direction_init();
    if (drv_rs485_uart_init(STC8H_UART2) != STC8H_OK) {
        while (1) {
        }
    }
    stc8h_interrupt_enable_global();

    while (1) {
        if (app_try_pop(&value) != 0u) {
            (void)drv_rs485_uart_write(STC8H_UART2, &value, 1u);
        }
        if (rx_overflow != 0u) {
            rx_overflow = 0u;
        }
    }
}
