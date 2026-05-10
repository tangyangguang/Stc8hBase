#include "drv_tm1637.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

static void tm1637_board_init(void)
{
    P_SW2 |= 0x80u;
    P1IE |= (BOARD_TM1637_CLK_MASK | BOARD_TM1637_DIO_MASK);
    P1PU |= (BOARD_TM1637_CLK_MASK | BOARD_TM1637_DIO_MASK);

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

void main(void)
{
    stc8h_s16 value;
    stc8h_u8 ok;

    (void)stc8h_uart_init(STC8H_UART1);
    tm1637_board_init();
    drv_tm1637_init();
    drv_tm1637_set_brightness(3u);

    value = 0;
    while (1) {
        ok = (drv_tm1637_display_number(value) == STC8H_OK) ? 1u : 0u;
        print_result(ok);
        ++value;
        if (value > 9999) {
            value = 0;
        }
        stc8h_delay_ms(500u);
    }
}
