#include "stc8h_uart.h"
#include "util_filter.h"

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "filter ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "filter error\r\n");
    }
}

void main(void)
{
    stc8h_u8 ok;
    stc8h_u16 value;

    (void)stc8h_uart_init(STC8H_UART1);

    ok = 1u;
    if (util_filter_limit_u16(5u, 10u, 100u) != 10u) ok = 0u;
    if (util_filter_limit_u16(200u, 10u, 100u) != 100u) ok = 0u;
    if (util_filter_limit_u16(50u, 10u, 100u) != 50u) ok = 0u;

    if (util_filter_iir_u16(100u, 200u, 0u) != 200u) ok = 0u;
    if (util_filter_iir_u16(100u, 200u, 2u) != 125u) ok = 0u;
    if (util_filter_iir_u16(200u, 100u, 2u) != 175u) ok = 0u;
    if (util_filter_iir_u16(100u, 101u, 4u) != 101u) ok = 0u;
    if (util_filter_iir_u16(101u, 100u, 4u) != 100u) ok = 0u;
    if (util_filter_iir_u16(123u, 123u, 3u) != 123u) ok = 0u;

    value = 0u;
    value = util_filter_iir_u16(value, 1023u, 3u);
    value = util_filter_iir_u16(value, 1023u, 3u);
    value = util_filter_iir_u16(value, 1023u, 3u);
    if ((value == 0u) || (value >= 1023u)) ok = 0u;

    while (1) {
        print_result(ok);
    }
}
