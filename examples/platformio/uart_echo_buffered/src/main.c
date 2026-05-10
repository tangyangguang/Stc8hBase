#include "stc8h_uart.h"
#include "util_ring_buffer.h"

static STC8H_DATA stc8h_u8 rx_mem[16];

void main(void)
{
    util_ring_buffer_t rx;
    stc8h_u8 value;

    (void)stc8h_uart_init(STC8H_UART1);
    util_ring_buffer_init(&rx, rx_mem, sizeof(rx_mem));

    stc8h_uart_write_code(STC8H_UART1, "uart echo buffered\r\n");

    while (1) {
        if (stc8h_uart_readable(STC8H_UART1) != 0u) {
            value = (stc8h_u8)stc8h_uart_getc(STC8H_UART1);
            (void)util_ring_buffer_push(&rx, value);
        }

        if (util_ring_buffer_pop(&rx, &value) != 0u) {
            stc8h_uart_putc(STC8H_UART1, (char)value);
        }
    }
}
