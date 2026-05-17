#include "drv_ec11.h"
#include "stc8h_gpio.h"
#include "stc8h_pwm.h"

#define SERVO_PERIOD_TICKS 18431u
#define SERVO_PRESCALER 11u
#define SERVO_MIN_US_TICKS 461u
#define SERVO_CENTER_US_TICKS 1382u
#define MOTOR_PERIOD_TICKS 1105u

static drv_ec11_small_t encoder;

static void pwm_pins_init(void)
{
    stc8h_gpio_set_mode(1u, 0u, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_set_mode(3u, 3u, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_set_mode(3u, 4u, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_set_mode(5u, 4u, STC8H_GPIO_MODE_PUSH_PULL);
}

void main(void)
{
    pwm_pins_init();
    drv_ec11_small_init(&encoder);

    (void)stc8h_pwm_set_prescaler(STC8H_PWM_GROUP_A, SERVO_PRESCALER);
    (void)stc8h_pwm_set_period(STC8H_PWM_GROUP_A, SERVO_PERIOD_TICKS);
    (void)stc8h_pwm_init_channel(STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_1, STC8H_PWM_PIN_PWM1_P10);
    (void)stc8h_pwm_set_duty(STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_1, SERVO_CENTER_US_TICKS);
    (void)stc8h_pwm_enable(STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_1);

    (void)stc8h_pwm_set_period(STC8H_PWM_GROUP_B, MOTOR_PERIOD_TICKS);
    (void)stc8h_pwm_init_channel(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_6, STC8H_PWM_PIN_PWM6_P54);
    (void)stc8h_pwm_init_channel(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_7, STC8H_PWM_PIN_PWM7_P33);
    (void)stc8h_pwm_init_channel(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_8, STC8H_PWM_PIN_PWM8_P34);
    (void)stc8h_pwm_set_duty(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_6, 0u);
    (void)stc8h_pwm_set_duty(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_7, MOTOR_PERIOD_TICKS / 2u);
    (void)stc8h_pwm_set_duty(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_8, 0u);
    (void)stc8h_pwm_enable(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_6);
    (void)stc8h_pwm_enable(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_7);
    (void)stc8h_pwm_enable(STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_8);

    while (1) {
        (void)drv_ec11_scan_delta_small(&encoder, 1u, 1u);
        (void)stc8h_pwm_set_duty(STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_1, SERVO_MIN_US_TICKS);
    }
}
