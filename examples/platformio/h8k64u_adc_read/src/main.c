#include "stc8h_adc.h"
#include "stc8h_uart.h"

static void print_hex16(stc8h_u16 value)
{
    static const STC8H_CODE char hex[] = "0123456789ABCDEF";

    stc8h_uart_putc(STC8H_UART1, hex[(value >> 12) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[(value >> 8) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[(value >> 4) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[value & 0x0Fu]);
}

void main(void)
{
    stc8h_u16 value;

    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_adc_init();
    while (1) {
        value = stc8h_adc_read(0u);
        stc8h_uart_write_code(STC8H_UART1, "ADC0=0x");
        print_hex16(value);
        stc8h_uart_write_code(STC8H_UART1, "\r\n");
    }
}
