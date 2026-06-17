#include "stc8h_eeprom.h"
#include "stc8h_delay.h"
#include "stc8h_uart.h"

#define H8K64U_EEPROM_TEST_ADDR 0x0000u

static STC8H_DATA stc8h_u8 write_data[4] = {0x86u, 0x4Au, 0xA5u, 0x5Au};
static STC8H_DATA stc8h_u8 read_data[4];

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "H8K64U EEPROM ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "H8K64U EEPROM error\r\n");
    }
}

void main(void)
{
    stc8h_u8 ok;
    stc8h_u8 i;

    (void)stc8h_uart_init(STC8H_UART1);

    ok = 1u;
    if (stc8h_eeprom_erase_sector(H8K64U_EEPROM_TEST_ADDR) != STC8H_OK) {
        ok = 0u;
    }
    if (stc8h_eeprom_write(H8K64U_EEPROM_TEST_ADDR, write_data, sizeof(write_data)) != STC8H_OK) {
        ok = 0u;
    }
    if (stc8h_eeprom_read(H8K64U_EEPROM_TEST_ADDR, read_data, sizeof(read_data)) != STC8H_OK) {
        ok = 0u;
    }

    for (i = 0u; i < sizeof(write_data); ++i) {
        if (read_data[i] != write_data[i]) {
            ok = 0u;
        }
    }

    while (1) {
        print_result(ok);
        stc8h_delay_ms(500u);
    }
}
