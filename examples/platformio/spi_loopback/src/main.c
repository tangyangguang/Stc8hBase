#include "stc8h_spi.h"
#include "stc8h_uart.h"

static const stc8h_u8 test_values[4] = {0x00u, 0x55u, 0xAAu, 0xFFu};

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "spi loopback ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "spi loopback error\r\n");
    }
}

void main(void)
{
    stc8h_u8 i;
    stc8h_u8 ok;

    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_spi_init();

    ok = 1u;
    for (i = 0u; i < sizeof(test_values); ++i) {
        if (stc8h_spi_transfer(test_values[i]) != test_values[i]) {
            ok = 0u;
        }
    }

    while (1) {
        print_result(ok);
    }
}
