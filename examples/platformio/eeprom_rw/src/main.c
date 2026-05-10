#include "stc8h_eeprom.h"
#include "stc8h_uart.h"

#ifndef BOARD_EEPROM_TEST_ADDR
#error "BOARD_EEPROM_TEST_ADDR must be defined explicitly."
#endif

#ifndef BOARD_EEPROM_TEST_SIZE
#error "BOARD_EEPROM_TEST_SIZE must be defined explicitly."
#endif

#ifndef BOARD_EEPROM_TEST_ENABLE
#error "BOARD_EEPROM_TEST_ENABLE must be defined explicitly for this destructive example."
#endif

#if (BOARD_EEPROM_TEST_ADDR & (STC8H_EEPROM_SECTOR_SIZE - 1u)) != 0
#error "BOARD_EEPROM_TEST_ADDR must be sector aligned."
#endif

#if BOARD_EEPROM_TEST_SIZE < STC8H_EEPROM_SECTOR_SIZE
#error "BOARD_EEPROM_TEST_SIZE must cover at least one erase sector."
#endif

#if (BOARD_EEPROM_TEST_ADDR + BOARD_EEPROM_TEST_SIZE) > STC8H_EEPROM_SIZE
#error "BOARD_EEPROM_TEST range exceeds STC8H1K08 EEPROM."
#endif

#if BOARD_EEPROM_TEST_ENABLE == 1
static STC8H_DATA stc8h_u8 write_data[4] = {0x12u, 0x34u, 0x56u, 0x78u};
static STC8H_DATA stc8h_u8 read_data[4];
#endif

#if BOARD_EEPROM_TEST_ENABLE == 1
static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "eeprom ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "eeprom error\r\n");
    }
}
#endif

#if BOARD_EEPROM_TEST_ENABLE != 1
void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);

    while (1) {
        stc8h_uart_write_code(STC8H_UART1, "eeprom write disabled\r\n");
    }
}
#else
void main(void)
{
    stc8h_u8 ok;
    stc8h_u8 i;

    (void)stc8h_uart_init(STC8H_UART1);

    ok = 1u;
    if (stc8h_eeprom_erase_sector(BOARD_EEPROM_TEST_ADDR) != STC8H_OK) {
        ok = 0u;
    }
    if (stc8h_eeprom_write(BOARD_EEPROM_TEST_ADDR, write_data, sizeof(write_data)) != STC8H_OK) {
        ok = 0u;
    }
    if (stc8h_eeprom_read(BOARD_EEPROM_TEST_ADDR, read_data, sizeof(read_data)) != STC8H_OK) {
        ok = 0u;
    }

    for (i = 0u; i < sizeof(write_data); ++i) {
        if (read_data[i] != write_data[i]) {
            ok = 0u;
        }
    }

    while (1) {
        print_result(ok);
    }
}
#endif
