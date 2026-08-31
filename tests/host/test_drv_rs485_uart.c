#include <stdio.h>
#include <string.h>

#define STC8H_UART_ENABLE_ISR_API 1
#define STC8H_UART_ENABLE_BOUNDED_PUTC 1
#define DRV_RS485_UART_ENABLE_RX_INTERRUPT 1
#define DRV_RS485_UART_TX_POLL_LIMIT 100u
#define DRV_RS485_UART_TX_COMPLETE_DELAY_US 100u

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
static unsigned int bounded_putc_count;
static unsigned int bounded_putc_fail_at;

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

stc8h_status_t stc8h_uart_putc_bounded(stc8h_uart_id_t uart, char ch, stc8h_u16 poll_limit)
{
    (void)uart;
    if (poll_limit != DRV_RS485_UART_TX_POLL_LIMIT) {
        event_append('?');
        return STC8H_ERROR;
    }
    ++bounded_putc_count;
    if (bounded_putc_fail_at != 0u && bounded_putc_count == bounded_putc_fail_at) {
        event_append('!');
        return STC8H_ERROR;
    }
    event_append(ch);
    return STC8H_OK;
}

stc8h_status_t stc8h_uart_interrupt_enable(stc8h_uart_id_t uart)
{
    event_append((uart == STC8H_UART2) ? 'E' : '?');
    return STC8H_OK;
}

stc8h_status_t stc8h_uart_interrupt_disable(stc8h_uart_id_t uart)
{
    event_append((uart == STC8H_UART2) ? 'D' : '?');
    return STC8H_OK;
}

stc8h_u8 stc8h_uart_try_getc(stc8h_uart_id_t uart, stc8h_u8 *value)
{
    (void)uart;
    (void)value;
    return 0u;
}

void stc8h_uart_clear_tx_flag(stc8h_uart_id_t uart)
{
    event_append((uart == STC8H_UART2) ? 'C' : '?');
}

void stc8h_delay_us(stc8h_u16 us)
{
    event_append((us == DRV_RS485_UART_TX_COMPLETE_DELAY_US) ? 'U' : '?');
}

void stc8h_uart_write(stc8h_uart_id_t uart, const char *data)
{
    (void)uart;
    (void)data;
}

void stc8h_uart_write_code(stc8h_uart_id_t uart, const STC8H_CODE char *data)
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
    bounded_putc_count = 0u;
    bounded_putc_fail_at = 0u;
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
    failures += require_events("2RCE",
                               "RS485 init must enter RX, clear TX, then enable UART interrupt");

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
    failures += require_events("DTABURCE",
                               "RS485 write must mask IRQ, send bounded bytes, wait stop bit, then restore RX IRQ");

    return failures;
}

static int test_write_failure_releases_bus(void)
{
    stc8h_u8 bytes[2];
    int failures;

    failures = 0;
    bytes[0] = 'A';
    bytes[1] = 'B';
    reset_events();
    bounded_putc_fail_at = 2u;

    failures += require(drv_rs485_uart_write(STC8H_UART2, bytes, sizeof(bytes)) == STC8H_ERROR,
                        "RS485 write must report bounded UART failure");
    failures += require_events("DTA!RCE",
                               "RS485 write failure must release bus and restore RX interrupt");

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
    failures += test_write_failure_releases_bus();
    failures += test_empty_write_does_not_toggle_direction();
    failures += test_read_passthrough_uses_uart();

    return failures == 0 ? 0 : 1;
}
