#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_pwm.h"
#include "stc8h_uart.h"

#define PWM_TEST_GROUP STC8H_PWM_GROUP_A
#define PWM_TEST_CHANNEL STC8H_PWM_CHANNEL_2
#define PWM_TEST_PIN STC8H_PWM_PIN_PWM2_P12
#define PWM_TEST_PERIOD 1023u
#define PWM_TEST_STEP 31u

static void pwm_pin_init(void)
{
    stc8h_gpio_set_mode(BOARD_LED_PORT, BOARD_LED_PIN, STC8H_GPIO_MODE_PUSH_PULL);
}

void main(void)
{
    stc8h_u16 duty;
    stc8h_u8 rising;
    stc8h_u8 ok;

    (void)stc8h_uart_init(STC8H_UART1);
    pwm_pin_init();

    ok = 1u;
    if (stc8h_pwm_set_period(PWM_TEST_GROUP, PWM_TEST_PERIOD) != STC8H_OK) {
        ok = 0u;
    }
    if (stc8h_pwm_init_channel(PWM_TEST_GROUP, PWM_TEST_CHANNEL, PWM_TEST_PIN) != STC8H_OK) {
        ok = 0u;
    }
    if (stc8h_pwm_set_duty(PWM_TEST_GROUP, PWM_TEST_CHANNEL, 0u) != STC8H_OK) {
        ok = 0u;
    }
    if (stc8h_pwm_enable(PWM_TEST_GROUP, PWM_TEST_CHANNEL) != STC8H_OK) {
        ok = 0u;
    }

    stc8h_uart_write_code(STC8H_UART1, ok ? "pwm output ok\r\n" : "pwm output error\r\n");

    duty = 0u;
    rising = 1u;
    while (1) {
        (void)stc8h_pwm_set_duty(PWM_TEST_GROUP, PWM_TEST_CHANNEL, duty);
        if (rising != 0u) {
            duty = (stc8h_u16)(duty + PWM_TEST_STEP);
            if (duty >= PWM_TEST_PERIOD) {
                duty = PWM_TEST_PERIOD;
                rising = 0u;
            }
        } else {
            if (duty <= PWM_TEST_STEP) {
                duty = 0u;
                rising = 1u;
            } else {
                duty = (stc8h_u16)(duty - PWM_TEST_STEP);
            }
        }
        stc8h_delay_ms(20u);
    }
}
