#include "stc8h_uart.h"
#include "stc8h_sfr.h"

#ifndef STC8H_AUXR_T1_1T
#define STC8H_AUXR_T1_1T 0x40u
#endif

#ifndef STC8H_AUXR_S1ST2
#define STC8H_AUXR_S1ST2 0x01u
#endif

#ifndef STC8H_INTCLKO_T1CLKO
#define STC8H_INTCLKO_T1CLKO 0x02u
#endif

#ifndef STC8H_P_SW1_UART1_MASK
#define STC8H_P_SW1_UART1_MASK 0xC0u
#endif

#ifndef STC8H_UART1_RELOAD
#if (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 115200UL)
#define STC8H_UART1_RELOAD 0xFFE8u
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 57600UL)
#define STC8H_UART1_RELOAD 0xFFD0u
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 38400UL)
#define STC8H_UART1_RELOAD 0xFFB8u
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 19200UL)
#define STC8H_UART1_RELOAD 0xFF70u
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_UART1_BAUD == 9600UL)
#define STC8H_UART1_RELOAD 0xFEE0u
#elif (STC8H_SYSCLK_HZ == 6000000UL) && (STC8H_UART1_BAUD == 115200UL)
#define STC8H_UART1_RELOAD 0xFFF3u
#elif (STC8H_SYSCLK_HZ == 6000000UL) && (STC8H_UART1_BAUD == 57600UL)
#define STC8H_UART1_RELOAD 0xFFE6u
#elif (STC8H_SYSCLK_HZ == 6000000UL) && (STC8H_UART1_BAUD == 38400UL)
#define STC8H_UART1_RELOAD 0xFFD9u
#elif (STC8H_SYSCLK_HZ == 6000000UL) && (STC8H_UART1_BAUD == 19200UL)
#define STC8H_UART1_RELOAD 0xFFB2u
#elif (STC8H_SYSCLK_HZ == 6000000UL) && (STC8H_UART1_BAUD == 9600UL)
#define STC8H_UART1_RELOAD 0xFF64u
#else
#define STC8H_UART1_RELOAD 0u
#endif
#endif

#ifndef STC8H_UART_ASSUME_UART1
#define STC8H_UART_ASSUME_UART1 0
#endif

#ifndef STC8H_UART_ENABLE_WRITE_RAM
#define STC8H_UART_ENABLE_WRITE_RAM 1
#endif

#ifndef STC8H_UART_ENABLE_RX
#define STC8H_UART_ENABLE_RX 1
#endif

#ifndef STC8H_UART_ENABLE_UART2
#define STC8H_UART_ENABLE_UART2 0
#endif

#ifndef STC8H_UART_ENABLE_UART3
#define STC8H_UART_ENABLE_UART3 0
#endif

#ifndef STC8H_UART2_PIN_GROUP
#define STC8H_UART2_PIN_GROUP 0u
#endif

#ifndef STC8H_UART3_PIN_GROUP
#define STC8H_UART3_PIN_GROUP 0u
#endif

#ifndef STC8H_UART_CONFIGURE_PORT_MODE
#define STC8H_UART_CONFIGURE_PORT_MODE 1
#endif

#if (STC8H_UART_ENABLE_UART2 || STC8H_UART_ENABLE_UART3) && STC8H_UART_ASSUME_UART1
#error "STC8H_UART_ASSUME_UART1 cannot be used when UART2 or UART3 is enabled."
#endif

#define STC8H_UART_RELOAD_VALUE(sysclk, baud) (65536UL - ((sysclk) / (baud) / 4UL))

#if STC8H_UART_ENABLE_UART2
#if STC8H_UART2_BAUD == 0UL
#error "STC8H_UART2_BAUD must be non-zero when UART2 is enabled."
#endif
#ifndef STC8H_UART2_RELOAD
#define STC8H_UART2_RELOAD STC8H_UART_RELOAD_VALUE(STC8H_SYSCLK_HZ, STC8H_UART2_BAUD)
#endif
#if (STC8H_UART2_RELOAD == 0UL) || (STC8H_UART2_RELOAD > 65535UL)
#error "STC8H_UART2_RELOAD is out of 16-bit range."
#endif
#endif

#if STC8H_UART_ENABLE_UART3
#if STC8H_UART3_BAUD == 0UL
#error "STC8H_UART3_BAUD must be non-zero when UART3 is enabled."
#endif
#ifndef STC8H_UART3_RELOAD
#define STC8H_UART3_RELOAD STC8H_UART_RELOAD_VALUE(STC8H_SYSCLK_HZ, STC8H_UART3_BAUD)
#endif
#if (STC8H_UART3_RELOAD == 0UL) || (STC8H_UART3_RELOAD > 65535UL)
#error "STC8H_UART3_RELOAD is out of 16-bit range."
#endif
#endif

#if STC8H_UART_ENABLE_UART2 || STC8H_UART_ENABLE_UART3
#define STC8H_AUXR_T2R   0x10u
#define STC8H_AUXR_T2_CT 0x08u
#define STC8H_AUXR_T2X12 0x04u
#define STC8H_UART2_REN  0x10u
#define STC8H_UART2_TI   0x02u
#define STC8H_UART2_RI   0x01u
#define STC8H_T4T3M_T3R    0x08u
#define STC8H_T4T3M_T3_CT  0x04u
#define STC8H_T4T3M_T3X12  0x02u
#define STC8H_T4T3M_T3CLKO 0x01u
#define STC8H_UART3_ST3    0x40u
#define STC8H_UART3_REN    0x10u
#define STC8H_UART3_TI     0x02u
#define STC8H_UART3_RI     0x01u

static void stc8h_uart_configure_quasi(stc8h_u8 port, stc8h_u8 mask)
{
#if STC8H_UART_CONFIGURE_PORT_MODE
    switch (port) {
    case 0u:
        P0M0 &= (stc8h_u8)~mask;
        P0M1 &= (stc8h_u8)~mask;
        P0 |= mask;
        break;
    case 1u:
        P1M0 &= (stc8h_u8)~mask;
        P1M1 &= (stc8h_u8)~mask;
        P1 |= mask;
        break;
    case 4u:
        P4M0 &= (stc8h_u8)~mask;
        P4M1 &= (stc8h_u8)~mask;
        P4 |= mask;
        break;
    case 5u:
        P5M0 &= (stc8h_u8)~mask;
        P5M1 &= (stc8h_u8)~mask;
        P5 |= mask;
        break;
    default:
        break;
    }
#else
    (void)port;
    (void)mask;
#endif
}
#endif

static stc8h_status_t stc8h_uart1_init(void)
{
#if STC8H_UART1_RELOAD == 0
    return STC8H_ERROR;
#else
    TR1 = 0;
    SCON = 0x50u;
    AUXR &= (stc8h_u8)~STC8H_AUXR_S1ST2;
    AUXR |= STC8H_AUXR_T1_1T;
    TMOD &= 0x0Fu;
    TL1 = (stc8h_u8)STC8H_UART1_RELOAD;
    TH1 = (stc8h_u8)(STC8H_UART1_RELOAD >> 8);
    ET1 = 0;
    INTCLKO &= (stc8h_u8)~STC8H_INTCLKO_T1CLKO;
    P_SW1 &= (stc8h_u8)~STC8H_P_SW1_UART1_MASK;
    P3M0 &= (stc8h_u8)~0x03u;
    P3M1 &= (stc8h_u8)~0x03u;
    TR1 = 1;
    TI = 0;
    RI = 0;

    return STC8H_OK;
#endif
}

#if STC8H_UART_ENABLE_UART2
static stc8h_status_t stc8h_uart2_init(void)
{
#if STC8H_UART2_PIN_GROUP == 0u
    P_SW2 &= (stc8h_u8)~0x01u;
    stc8h_uart_configure_quasi(1u, 0x03u);
#else
    P_SW2 |= 0x01u;
    stc8h_uart_configure_quasi(4u, 0xC0u);
#endif

    S2CON = STC8H_UART2_REN;
    AUXR &= (stc8h_u8)~(STC8H_AUXR_T2R | STC8H_AUXR_T2_CT | STC8H_AUXR_T2X12);
    T2L = (stc8h_u8)STC8H_UART2_RELOAD;
    T2H = (stc8h_u8)(STC8H_UART2_RELOAD >> 8);
    AUXR = (stc8h_u8)((AUXR & (stc8h_u8)~STC8H_AUXR_T2_CT) |
                      STC8H_AUXR_T2X12 | STC8H_AUXR_T2R);
    S2CON &= (stc8h_u8)~(STC8H_UART2_TI | STC8H_UART2_RI);

    return STC8H_OK;
}
#endif

#if STC8H_UART_ENABLE_UART3
static stc8h_status_t stc8h_uart3_init(void)
{
#if STC8H_UART3_PIN_GROUP == 0u
    P_SW2 &= (stc8h_u8)~0x02u;
    stc8h_uart_configure_quasi(0u, 0x03u);
#else
    P_SW2 |= 0x02u;
    stc8h_uart_configure_quasi(5u, 0x03u);
#endif

    S3CON = (stc8h_u8)(STC8H_UART3_ST3 | STC8H_UART3_REN);
    T4T3M &= (stc8h_u8)~(STC8H_T4T3M_T3R | STC8H_T4T3M_T3_CT |
                         STC8H_T4T3M_T3X12 | STC8H_T4T3M_T3CLKO);
    T3L = (stc8h_u8)STC8H_UART3_RELOAD;
    T3H = (stc8h_u8)(STC8H_UART3_RELOAD >> 8);
    T4T3M = (stc8h_u8)(T4T3M | STC8H_T4T3M_T3X12 | STC8H_T4T3M_T3R);
    S3CON &= (stc8h_u8)~(STC8H_UART3_TI | STC8H_UART3_RI);

    return STC8H_OK;
}
#endif

stc8h_status_t stc8h_uart_init(stc8h_uart_id_t uart)
{
#if STC8H_UART_ASSUME_UART1
    (void)uart;
    return stc8h_uart1_init();
#else
    switch (uart) {
    case STC8H_UART1:
        return stc8h_uart1_init();
#if STC8H_UART_ENABLE_UART2
    case STC8H_UART2:
        return stc8h_uart2_init();
#endif
#if STC8H_UART_ENABLE_UART3
    case STC8H_UART3:
        return stc8h_uart3_init();
#endif
    default:
        return STC8H_ERROR;
    }
#endif
}

void stc8h_uart_putc(stc8h_uart_id_t uart, char ch)
{
#if STC8H_UART_ASSUME_UART1
    (void)uart;
#else
    switch (uart) {
    case STC8H_UART1:
#endif
        SBUF = (stc8h_u8)ch;
        while (TI == 0) {
        }
        TI = 0;
#if !STC8H_UART_ASSUME_UART1
        break;
#if STC8H_UART_ENABLE_UART2
    case STC8H_UART2:
        S2CON &= (stc8h_u8)~STC8H_UART2_TI;
        S2BUF = (stc8h_u8)ch;
        while ((S2CON & STC8H_UART2_TI) == 0u) {
        }
        S2CON &= (stc8h_u8)~STC8H_UART2_TI;
        break;
#endif
#if STC8H_UART_ENABLE_UART3
    case STC8H_UART3:
        S3CON &= (stc8h_u8)~STC8H_UART3_TI;
        S3BUF = (stc8h_u8)ch;
        while ((S3CON & STC8H_UART3_TI) == 0u) {
        }
        S3CON &= (stc8h_u8)~STC8H_UART3_TI;
        break;
#endif
    default:
        break;
    }
#endif
}

#if STC8H_UART_ENABLE_WRITE_RAM
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
#endif

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

#if STC8H_UART_ENABLE_RX
stc8h_u8 stc8h_uart_readable(stc8h_uart_id_t uart)
{
#if STC8H_UART_ASSUME_UART1
    (void)uart;
    return (RI != 0) ? 1u : 0u;
#else
    switch (uart) {
    case STC8H_UART1:
        return (RI != 0) ? 1u : 0u;
#if STC8H_UART_ENABLE_UART2
    case STC8H_UART2:
        return ((S2CON & STC8H_UART2_RI) != 0u) ? 1u : 0u;
#endif
#if STC8H_UART_ENABLE_UART3
    case STC8H_UART3:
        return ((S3CON & STC8H_UART3_RI) != 0u) ? 1u : 0u;
#endif
    default:
        return 0u;
    }
#endif
}

char stc8h_uart_getc(stc8h_uart_id_t uart)
{
#if STC8H_UART_ASSUME_UART1
    (void)uart;

    while (RI == 0) {
    }
    RI = 0;
    return (char)SBUF;
#else
    switch (uart) {
    case STC8H_UART1:
        while (RI == 0) {
        }
        RI = 0;
        return (char)SBUF;
#if STC8H_UART_ENABLE_UART2
    case STC8H_UART2:
    {
        char ch;
        while ((S2CON & STC8H_UART2_RI) == 0u) {
        }
        ch = (char)S2BUF;
        S2CON &= (stc8h_u8)~STC8H_UART2_RI;
        return ch;
    }
#endif
#if STC8H_UART_ENABLE_UART3
    case STC8H_UART3:
    {
        char ch;
        while ((S3CON & STC8H_UART3_RI) == 0u) {
        }
        ch = (char)S3BUF;
        S3CON &= (stc8h_u8)~STC8H_UART3_RI;
        return ch;
    }
#endif
    default:
        return '\0';
    }
#endif
}
#endif
