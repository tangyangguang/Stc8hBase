#include "drv_tm1637.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

static void tm1637_board_init(void)
{
    P_SW2 |= 0x80u;
    P1IE |= BOARD_TM1637_CLK_MASK;
    P3IE |= BOARD_TM1637_DIO_MASK;
    P1PU |= BOARD_TM1637_CLK_MASK;
    P3PU |= BOARD_TM1637_DIO_MASK;

    stc8h_gpio_set_mode(BOARD_TM1637_CLK_PORT, BOARD_TM1637_CLK_PIN, STC8H_GPIO_MODE_OPEN_DRAIN);
    stc8h_gpio_set_mode(BOARD_TM1637_DIO_PORT, BOARD_TM1637_DIO_PIN, STC8H_GPIO_MODE_OPEN_DRAIN);
    BOARD_TM1637_CLK_HIGH();
    BOARD_TM1637_DIO_HIGH();
}

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "tm1637 ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "tm1637 error\r\n");
    }
}

static stc8h_u8 divmod10(stc8h_u16 *value)
{
    stc8h_u16 quotient;
    stc8h_u16 remainder;

    quotient = 0u;
    remainder = *value;
    while (remainder >= 10u) {
        remainder = (stc8h_u16)(remainder - 10u);
        ++quotient;
    }
    *value = quotient;

    return (stc8h_u8)remainder;
}

static stc8h_status_t display_u16_4(stc8h_u16 value)
{
    stc8h_u8 digits[4];
    stc8h_u8 pos;

    pos = 4u;
    do {
        --pos;
        digits[pos] = divmod10(&value);
    } while (pos != 0u);

    return drv_tm1637_display_digits(digits, 4u);
}

void main(void)
{
    stc8h_u16 value;
    stc8h_u8 ok;
    const stc8h_u8 all_on[4] = {8u, 8u, 8u, 8u};
    const stc8h_u8 order_a[4] = {0u, 1u, 2u, 3u};
    const stc8h_u8 order_b[4] = {4u, 5u, 6u, 7u};

    (void)stc8h_uart_init(STC8H_UART1);
    tm1637_board_init();
    drv_tm1637_init();
    drv_tm1637_set_brightness(7u);

    ok = (drv_tm1637_display_digits(all_on, 4u) == STC8H_OK) ? 1u : 0u;
    print_result(ok);
    stc8h_delay_ms(1500u);
    ok = (drv_tm1637_display_digits(order_a, 4u) == STC8H_OK) ? 1u : 0u;
    print_result(ok);
    stc8h_delay_ms(1500u);
    ok = (drv_tm1637_display_digits(order_b, 4u) == STC8H_OK) ? 1u : 0u;
    print_result(ok);
    stc8h_delay_ms(1500u);

    value = 0;
    while (1) {
        ok = (display_u16_4(value) == STC8H_OK) ? 1u : 0u;
        print_result(ok);
        ++value;
        if (value > 9999) {
            value = 0;
        }
        stc8h_delay_ms(500u);
    }
}
