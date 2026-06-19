#include <stdio.h>
#include <string.h>

#include "stc8h_config.h"
#include "stc8h_uart.h"

static void event_append(char event);
static void board_rs485_tx_enable(void);
static void board_rs485_rx_enable(void);

#define BOARD_RS485_TX_ENABLE() board_rs485_tx_enable()
#define BOARD_RS485_RX_ENABLE() board_rs485_rx_enable()

#include "../../drivers/drv_rs485_uart.c"

static char events[64];
static unsigned int event_count;
static stc8h_u8 readable_value;
static char getc_value;

static void event_append(char event)
{
    if (event_count < sizeof(events)) {
        events[event_count] = event;
    }
    ++event_count;
}

static void board_rs485_tx_enable(void)
{
    event_append('T');
}

static void board_rs485_rx_enable(void)
{
    event_append('R');
}

stc8h_status_t stc8h_uart_init(stc8h_uart_id_t uart)
{
    event_append((uart == STC8H_UART2) ? '2' : 'i');
    return STC8H_OK;
}

void stc8h_uart_putc(stc8h_uart_id_t uart, char ch)
{
    (void)uart;
    event_append(ch);
}

void stc8h_uart_write(stc8h_uart_id_t uart, const char *data)
{
    (void)uart;
    (void)data;
}

void stc8h_uart_write_code(stc8h_uart_id_t uart, STC8H_CODE char *data)
{
    (void)uart;
    (void)data;
}

stc8h_u8 stc8h_uart_readable(stc8h_uart_id_t uart)
{
    event_append((uart == STC8H_UART2) ? 'r' : '?');
    return readable_value;
}

char stc8h_uart_getc(stc8h_uart_id_t uart)
{
    event_append((uart == STC8H_UART2) ? 'g' : '?');
    return getc_value;
}

static void reset_events(void)
{
    memset(events, 0, sizeof(events));
    event_count = 0u;
}

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static int require_events(const char *expected, const char *message)
{
    char actual[64];

    memset(actual, 0, sizeof(actual));
    memcpy(actual, events, sizeof(events));
    return require(strcmp(actual, expected) == 0, message);
}

static int test_init_sets_receive_mode(void)
{
    int failures;

    failures = 0;
    reset_events();

    failures += require(drv_rs485_uart_init(STC8H_UART2) == STC8H_OK,
                        "RS485 init must return UART init status");
    failures += require_events("2R", "RS485 init must initialize UART then enable RX");

    return failures;
}

static int test_write_switches_tx_then_rx(void)
{
    stc8h_u8 bytes[2];
    int failures;

    failures = 0;
    bytes[0] = 'A';
    bytes[1] = 'B';
    reset_events();

    failures += require(drv_rs485_uart_write(STC8H_UART2, bytes, sizeof(bytes)) == STC8H_OK,
                        "RS485 write must succeed for valid byte buffer");
    failures += require_events("TABR", "RS485 write must enable TX, send bytes, then enable RX");

    return failures;
}

static int test_empty_write_does_not_toggle_direction(void)
{
    int failures;

    failures = 0;
    reset_events();

    failures += require(drv_rs485_uart_write(STC8H_UART2, 0, 0u) == STC8H_OK,
                        "empty RS485 write must be a no-op success");
    failures += require_events("", "empty RS485 write must not toggle direction");

    return failures;
}

static int test_read_passthrough_uses_uart(void)
{
    int failures;

    failures = 0;
    readable_value = 1u;
    getc_value = 'Z';
    reset_events();

    failures += require(drv_rs485_uart_readable(STC8H_UART2) == 1u,
                        "RS485 readable must pass through UART readable");
    failures += require(drv_rs485_uart_getc(STC8H_UART2) == (stc8h_u8)'Z',
                        "RS485 getc must pass through UART getc");
    failures += require_events("rg", "RS485 read helpers must use selected UART");

    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_init_sets_receive_mode();
    failures += test_write_switches_tx_then_rx();
    failures += test_empty_write_does_not_toggle_direction();
    failures += test_read_passthrough_uses_uart();

    return failures == 0 ? 0 : 1;
}
