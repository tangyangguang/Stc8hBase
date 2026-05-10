#include "stc8h_uart.h"
#include "util_crc.h"

static STC8H_DATA stc8h_u8 test_data[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "crc ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "crc error\r\n");
    }
}

void main(void)
{
    stc8h_u8 ok;

    (void)stc8h_uart_init(STC8H_UART1);

    ok = 1u;
    if (util_checksum8(test_data, sizeof(test_data)) != 0xDDu) {
        ok = 0u;
    }
    if (util_crc16_modbus(test_data, sizeof(test_data)) != 0x4B37u) {
        ok = 0u;
    }
    if (util_checksum8(test_data, 0u) != 0u) {
        ok = 0u;
    }
    if (util_crc16_modbus(test_data, 0u) != 0xFFFFu) {
        ok = 0u;
    }

    while (1) {
        print_result(ok);
    }
}
