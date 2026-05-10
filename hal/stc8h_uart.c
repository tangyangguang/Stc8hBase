#include "stc8h_uart.h"
#include "stc8h_sfr.h"

#ifndef STC8H_AUXR_T1_1T
#define STC8H_AUXR_T1_1T 0x40u
#endif

#ifndef STC8H_UART1_RELOAD
#if (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 115200UL)
#define STC8H_UART1_RELOAD 0xFDu
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 57600UL)
#define STC8H_UART1_RELOAD 0xFAu
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 38400UL)
#define STC8H_UART1_RELOAD 0xF7u
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 19200UL)
#define STC8H_UART1_RELOAD 0xEEu
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 9600UL)
#define STC8H_UART1_RELOAD 0xDCu
#else
#define STC8H_UART1_RELOAD 0u
#endif
#endif

stc8h_status_t stc8h_uart_init(stc8h_uart_id_t uart)
{
    if (uart != STC8H_UART1) {
        return STC8H_ERROR;
    }

#if STC8H_UART1_RELOAD == 0
    return STC8H_ERROR;
#else
    SCON = 0x50u;
    AUXR |= STC8H_AUXR_T1_1T;
    TMOD &= 0x0Fu;
    TMOD |= 0x20u;
    TL1 = STC8H_UART1_RELOAD;
    TH1 = STC8H_UART1_RELOAD;
    TR1 = 1;
    TI = 0;
    RI = 0;

    return STC8H_OK;
#endif
}

void stc8h_uart_putc(stc8h_uart_id_t uart, char ch)
{
    if (uart != STC8H_UART1) {
        return;
    }

    SBUF = (stc8h_u8)ch;
    while (TI == 0) {
    }
    TI = 0;
}

void stc8h_uart_write(stc8h_uart_id_t uart, const char *data)
{
    if (data == 0) {
        return;
    }

    while (*data != '\0') {
        stc8h_uart_putc(uart, *data);
        ++data;
    }
}

void stc8h_uart_write_code(stc8h_uart_id_t uart, const STC8H_CODE char *data)
{
    if (data == 0) {
        return;
    }

    while (*data != '\0') {
        stc8h_uart_putc(uart, *data);
        ++data;
    }
}

stc8h_u8 stc8h_uart_readable(stc8h_uart_id_t uart)
{
    if (uart != STC8H_UART1) {
        return 0u;
    }
    return (RI != 0) ? 1u : 0u;
}

char stc8h_uart_getc(stc8h_uart_id_t uart)
{
    char ch;

    if (uart != STC8H_UART1) {
        return '\0';
    }

    while (RI == 0) {
    }
    RI = 0;
    ch = (char)SBUF;
    return ch;
}
