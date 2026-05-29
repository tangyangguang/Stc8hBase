#include <stdio.h>
#include <string.h>

#include "stc8h_config.h"

#define DRV_TM1637_ENABLE_DISPLAY_RAW 0
#define DRV_TM1637_ENABLE_DISPLAY_RAW4 1
#define DRV_TM1637_ENABLE_DISPLAY_RAW4_DATA 1
#define DRV_TM1637_ENABLE_SET_DISPLAY 0
#define DRV_TM1637_ENABLE_BRIGHTNESS_STATE 0
#define DRV_TM1637_ENABLE_RAW_LEN_CHECK 0
#define DRV_TM1637_ENABLE_DISPLAY_DIGITS 0
#define DRV_TM1637_ENABLE_DISPLAY_NUMBER 0
#define DRV_TM1637_ENABLE_ENCODE_DIGIT 0
#define DRV_TM1637_ENABLE_CLEAR 0

static void event_append(char event);
static void board_clk_high(void);
static void board_clk_low(void);
static void board_dio_high(void);
static void board_dio_low(void);
static stc8h_u8 board_dio_read(void);

#define BOARD_TM1637_CLK_HIGH() board_clk_high()
#define BOARD_TM1637_CLK_LOW() board_clk_low()
#define BOARD_TM1637_DIO_HIGH() board_dio_high()
#define BOARD_TM1637_DIO_LOW() board_dio_low()
#define BOARD_TM1637_DIO_READ() board_dio_read()

#include "../../drivers/drv_tm1637.c"

static char events[2048];
static unsigned int event_count;

static void event_append(char event)
{
    if (event_count < sizeof(events)) {
        events[event_count] = event;
    }
    ++event_count;
}

static void board_clk_high(void)
{
    event_append('H');
}

static void board_clk_low(void)
{
    event_append('L');
}

static void board_dio_high(void)
{
    event_append('1');
}

static void board_dio_low(void)
{
    event_append('0');
}

static stc8h_u8 board_dio_read(void)
{
    event_append('R');
    return 0u;
}

void stc8h_delay_us(stc8h_u16 us)
{
    (void)us;
    event_append('d');
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

static int test_data_raw4_matches_generic_raw4_event_sequence(void)
{
    int failures;
    static STC8H_DATA stc8h_u8 segments[4] = { 0x3Fu, 0x06u, 0x5Bu, 0x4Fu };
    char generic_events[2048];
    unsigned int generic_count;
    stc8h_status_t generic_status;
    stc8h_status_t data_status;

    failures = 0;

    reset_events();
    generic_status = drv_tm1637_display_raw4(segments);
    generic_count = event_count;
    memcpy(generic_events, events, sizeof(generic_events));

    reset_events();
    data_status = drv_tm1637_display_raw4_data(segments);

    failures += require(generic_status == STC8H_OK,
                        "generic raw4 must succeed with ACK low");
    failures += require(data_status == STC8H_OK,
                        "DATA raw4 must succeed with ACK low");
    failures += require(generic_count == event_count,
                        "DATA raw4 must emit the same number of GPIO events as generic raw4");
    failures += require(memcmp(generic_events, events, sizeof(generic_events)) == 0,
                        "DATA raw4 must emit the same GPIO event sequence as generic raw4");

    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_data_raw4_matches_generic_raw4_event_sequence();

    return failures == 0 ? 0 : 1;
}
