#include "stc8h_adc.h"
#include "stc8h_delay.h"
#include "stc8h_uart.h"

#ifndef H8K64U_INTERNAL_REF_MV
#define H8K64U_INTERNAL_REF_MV 1191UL
#endif

static void print_hex16(stc8h_u16 value)
{
    static const STC8H_CODE char hex[] = "0123456789ABCDEF";

    stc8h_uart_putc(STC8H_UART1, hex[(value >> 12) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[(value >> 8) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[(value >> 4) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[value & 0x0Fu]);
}

static void print_u16_dec(stc8h_u16 value)
{
    char digits[5];
    stc8h_u8 i;
    stc8h_u8 started;

    for (i = 0u; i < 5u; ++i) {
        digits[i] = (char)('0' + (value % 10u));
        value = (stc8h_u16)(value / 10u);
    }

    started = 0u;
    for (i = 5u; i != 0u; --i) {
        if ((digits[i - 1u] != '0') || (started != 0u) || (i == 1u)) {
            stc8h_uart_putc(STC8H_UART1, digits[i - 1u]);
            started = 1u;
        }
    }
}

static stc8h_u16 calc_vref_mv(stc8h_u16 adc15)
{
    if ((adc15 == 0u) || (adc15 == STC8H_ADC_INVALID_VALUE)) {
        return STC8H_ADC_INVALID_VALUE;
    }
    return (stc8h_u16)((H8K64U_INTERNAL_REF_MV * 4095UL + (adc15 / 2u)) / adc15);
}

void main(void)
{
    stc8h_u16 adc0;
    stc8h_u16 adc15;
    stc8h_u16 vref_mv;

    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_adc_init();
    while (1) {
        adc0 = stc8h_adc_read(0u);
        adc15 = stc8h_adc_read(15u);
        vref_mv = calc_vref_mv(adc15);

        stc8h_uart_write_code(STC8H_UART1, "ADC0=0x");
        print_hex16(adc0);
        stc8h_uart_write_code(STC8H_UART1, " ADC15=0x");
        print_hex16(adc15);
        stc8h_uart_write_code(STC8H_UART1, " VREF+=");
        if (vref_mv == STC8H_ADC_INVALID_VALUE) {
            stc8h_uart_write_code(STC8H_UART1, "invalid");
        } else {
            print_u16_dec(vref_mv);
            stc8h_uart_write_code(STC8H_UART1, "mV");
        }
        stc8h_uart_write_code(STC8H_UART1, "\r\n");
        stc8h_delay_ms(500u);
    }
}
