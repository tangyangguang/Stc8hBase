#include "stc8h_iap_program.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#define H8K64U_EXEC_PROBE_APP_BASE 0x0200u
#define H8K64U_EXEC_PROBE_SECTOR_BASE 0x0200u

typedef void (*h8k64u_exec_probe_entry_t)(void);

static const STC8H_CODE stc8h_u8 exec_payload[] = {
    0xC2u, 0x99u,       /* CLR TI */
    0x75u, 0x99u, 'X',  /* MOV SBUF,#'X' */
    0x30u, 0x99u, 0xFDu,/* JNB TI,$ */
    0x80u, 0xFEu        /* SJMP $ */
};

static STC8H_XDATA stc8h_u8 iap_readback[sizeof(exec_payload)];

static void uart_puts(const char *text)
{
    while (*text != '\0') {
        stc8h_uart_putc(STC8H_UART1, *text);
        ++text;
    }
}

static void uart_put_hex_nibble(stc8h_u8 value)
{
    value &= 0x0Fu;
    stc8h_uart_putc(STC8H_UART1,
                    (char)((value < 10u) ? ('0' + value) : ('A' + value - 10u)));
}

static void uart_put_hex8(stc8h_u8 value)
{
    uart_put_hex_nibble((stc8h_u8)(value >> 4));
    uart_put_hex_nibble(value);
}

static stc8h_u8 read_code_byte(stc8h_u16 addr)
{
    const STC8H_CODE stc8h_u8 *ptr;

    ptr = (const STC8H_CODE stc8h_u8 *)addr;
    return *ptr;
}

static void print_bytes(const char *label, const stc8h_u8 *bytes, stc8h_u8 len)
{
    stc8h_u8 i;

    uart_puts(label);
    for (i = 0u; i < len; ++i) {
        uart_put_hex8(bytes[i]);
    }
    uart_puts("\r\n");
}

static void print_code_bytes(void)
{
    stc8h_u8 i;

    uart_puts("MOVC=");
    for (i = 0u; i < sizeof(exec_payload); ++i) {
        uart_put_hex8(read_code_byte((stc8h_u16)(H8K64U_EXEC_PROBE_APP_BASE + i)));
    }
    uart_puts("\r\n");
}

void main(void)
{
    stc8h_u8 i;
    stc8h_status_t status;

    (void)stc8h_uart_init(STC8H_UART1);
    uart_puts("IAP-EXEC-PROBE\r\n");

    status = stc8h_iap_program_erase_sector(H8K64U_EXEC_PROBE_SECTOR_BASE);
    uart_puts((status == STC8H_OK) ? "ERASE=OK\r\n" : "ERASE=ERR\r\n");
    if (status != STC8H_OK) {
        while (1) {
        }
    }

    for (i = 0u; i < sizeof(exec_payload); ++i) {
        iap_readback[i] = exec_payload[i];
    }
    status = stc8h_iap_program_write(H8K64U_EXEC_PROBE_APP_BASE,
                                     iap_readback,
                                     (stc8h_u16)sizeof(exec_payload));
    uart_puts((status == STC8H_OK) ? "WRITE=OK\r\n" : "WRITE=ERR\r\n");
    if (status != STC8H_OK) {
        while (1) {
        }
    }

    for (i = 0u; i < sizeof(exec_payload); ++i) {
        iap_readback[i] = 0u;
    }
    status = stc8h_iap_program_read(H8K64U_EXEC_PROBE_APP_BASE,
                                    iap_readback,
                                    (stc8h_u16)sizeof(exec_payload));
    uart_puts((status == STC8H_OK) ? "READ=OK\r\n" : "READ=ERR\r\n");
    print_bytes("IAP=", iap_readback, (stc8h_u8)sizeof(exec_payload));
    print_code_bytes();

    uart_puts("JUMP\r\n");
    ((h8k64u_exec_probe_entry_t)H8K64U_EXEC_PROBE_APP_BASE)();

    uart_puts("RETURNED\r\n");
    while (1) {
    }
}
