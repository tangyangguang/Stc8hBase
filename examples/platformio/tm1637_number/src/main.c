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

    for (pos = 0u; pos < 4u; ++pos) {
        digits[pos] = drv_tm1637_encode_digit(digits[pos]);
    }

    return drv_tm1637_display_raw4(digits);
}

void main(void)
{
    stc8h_u16 value;
    stc8h_u8 ok;
    const stc8h_u8 all_on[4] = {0x7Fu, 0x7Fu, 0x7Fu, 0x7Fu};
    const stc8h_u8 order_a[4] = {0x3Fu, 0x06u, 0x5Bu, 0x4Fu};
    const stc8h_u8 order_b[4] = {0x66u, 0x6Du, 0x7Du, 0x07u};

    (void)stc8h_uart_init(STC8H_UART1);
    tm1637_board_init();
    drv_tm1637_init();
    drv_tm1637_set_brightness(7u);

    ok = (drv_tm1637_display_raw4(all_on) == STC8H_OK) ? 1u : 0u;
    print_result(ok);
    stc8h_delay_ms(1500u);
    ok = (drv_tm1637_display_raw4(order_a) == STC8H_OK) ? 1u : 0u;
    print_result(ok);
    stc8h_delay_ms(1500u);
    ok = (drv_tm1637_display_raw4(order_b) == STC8H_OK) ? 1u : 0u;
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
