#include <stdio.h>

#define STC8H_SYSCLK_HZ 35000000UL
#define DRV_NRF24L01_ENABLE_CHECK_PRESENT 0
#define DRV_NRF24L01_ENABLE_ARG_CHECK 0
#define DRV_NRF24L01_ENABLE_ADDRESS_API 0
#define DRV_NRF24L01_ENABLE_READ_FIFO_STATUS 0
#define DRV_NRF24L01_ENABLE_READ_OBSERVE_TX 0
#define DRV_NRF24L01_ENABLE_READ_STATUS 0
#define DRV_NRF24L01_ENABLE_RAW_API 0
#define DRV_NRF24L01_ENABLE_POWER_DOWN 0
#define DRV_NRF24L01_ENABLE_ENTER_STANDBY 0
#define DRV_NRF24L01_ENABLE_READ_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_ACK_PAYLOAD 0

static void test_event(char event);
static void test_ce_high(void);
static void test_ce_low(void);
static void test_csn_high(void);
static void test_csn_low(void);
static void test_configure_pins(void);
static void test_power_up_delay(void);
static void test_ce_pulse_delay(void);

#define DRV_NRF24L01_CE_HIGH() test_ce_high()
#define DRV_NRF24L01_CE_LOW() test_ce_low()
#define DRV_NRF24L01_CSN_HIGH() test_csn_high()
#define DRV_NRF24L01_CSN_LOW() test_csn_low()
#define DRV_NRF24L01_CONFIGURE_PINS() test_configure_pins()
#define DRV_NRF24L01_POWER_UP_DELAY() test_power_up_delay()
#define DRV_NRF24L01_CE_PULSE_DELAY() test_ce_pulse_delay()

#include "../../drivers/drv_nrf24l01.c"

#ifndef DRV_NRF24L01_CE_PULSE_DELAY_LOOPS
#define DRV_NRF24L01_CE_PULSE_DELAY_LOOPS 0u
#endif

static char events[32];
static unsigned char event_count;

static void test_event(char event)
{
    if (event_count < (unsigned char)sizeof(events)) {
        events[event_count] = event;
    }
    ++event_count;
}

static void test_ce_high(void)
{
    test_event('H');
}

static void test_ce_low(void)
{
    test_event('L');
}

static void test_csn_high(void)
{
    test_event('h');
}

static void test_csn_low(void)
{
    test_event('l');
}

static void test_configure_pins(void)
{
    test_event('C');
}

static void test_power_up_delay(void)
{
    test_event('P');
}

static void test_ce_pulse_delay(void)
{
    test_event('D');
}

stc8h_u8 stc8h_spi_transfer(stc8h_u8 value)
{
    (void)value;
    return 0u;
}

static void reset_events(void)
{
    event_count = 0u;
}

static int index_of(char event)
{
    unsigned char i;

    for (i = 0u; i < event_count; ++i) {
        if (events[i] == event) {
            return (int)i;
        }
    }
    return -1;
}

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failures;
    int delay_index;
    int ce_high_index;

    failures = 0;

    reset_events();
    drv_nrf24l01_init_pins();
    failures += require(index_of('C') == 0, "init_pins must configure board pins before driving CE/CSN");
    failures += require((index_of('L') > index_of('C')) && (index_of('h') > index_of('L')),
                        "init_pins must drive CE low then CSN high after board pin configuration");

    reset_events();
    drv_nrf24l01_enter_rx();
    delay_index = index_of('P');
    ce_high_index = index_of('H');
    failures += require(delay_index >= 0, "enter_rx must wait after PWR_UP");
    failures += require(ce_high_index >= 0, "enter_rx must raise CE");
    failures += require((delay_index >= 0) && (ce_high_index >= 0) && (delay_index < ce_high_index),
                        "enter_rx must wait before CE high");

    reset_events();
    drv_nrf24l01_enter_tx();
    failures += require(index_of('P') >= 0, "enter_tx must wait after PWR_UP");

    reset_events();
    drv_nrf24l01_pulse_ce();
    failures += require(index_of('D') >= 0, "pulse_ce must use CE pulse delay");
    failures += require((index_of('H') >= 0) && (index_of('D') > index_of('H')) && (index_of('L') > index_of('D')),
                        "pulse_ce must keep CE high while delaying");

    failures += require(DRV_NRF24L01_CE_PULSE_DELAY_LOOPS >= 140u,
                        "35MHz CE pulse loop count must cover at least 12us at 3 cycles/loop");

    (void)drv_nrf24l01_read_reg(0u);
    (void)drv_nrf24l01_read_buf(0u, (stc8h_u8 *)events, 0u);

    return failures == 0 ? 0 : 1;
}
