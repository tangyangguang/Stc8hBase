#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_pwm.h"

#if !STC8H_CHIP_STC8H8K64U
#error "h8k64u_pwm_8ch_validate requires STC8H8K64U."
#endif

#define PWMA_PERIOD_TICKS 1023u
#define PWMB_PERIOD_TICKS 2047u
#define PWM_PRESCALER     10u

typedef struct {
    stc8h_u8 group;
    stc8h_u8 channel;
    stc8h_u8 pin_select;
    stc8h_u8 port;
    stc8h_u8 pin;
    stc8h_u16 duty_a;
    stc8h_u16 duty_b;
} pwm_test_channel_t;

static const pwm_test_channel_t pwm_channels[] = {
    { STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_1, STC8H_PWM_PIN_PWM1_P10, 1u, 0u, 128u, 768u },
    { STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_2, STC8H_PWM_PIN_PWM2_P12, 1u, 2u, 256u, 640u },
    { STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_3, STC8H_PWM_PIN_PWM3_P14, 1u, 4u, 384u, 512u },
    { STC8H_PWM_GROUP_A, STC8H_PWM_CHANNEL_4, STC8H_PWM_PIN_PWM4_P16, 1u, 6u, 512u, 384u },
    { STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_5, STC8H_PWM_PIN_PWM5_P20, 2u, 0u, 256u, 1536u },
    { STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_6, STC8H_PWM_PIN_PWM6_P21, 2u, 1u, 512u, 1280u },
    { STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_7, STC8H_PWM_PIN_PWM7_P22, 2u, 2u, 768u, 1024u },
    { STC8H_PWM_GROUP_B, STC8H_PWM_CHANNEL_8, STC8H_PWM_PIN_PWM8_P23, 2u, 3u, 1024u, 768u },
};

#define PWM_CHANNEL_COUNT ((stc8h_u8)(sizeof(pwm_channels) / sizeof(pwm_channels[0])))

static void require_ok(stc8h_status_t status)
{
    if (status != STC8H_OK) {
        while (1) {
        }
    }
}

static void pwm_pins_init(void)
{
    stc8h_u8 i;

    for (i = 0u; i < PWM_CHANNEL_COUNT; ++i) {
        stc8h_gpio_set_mode(pwm_channels[i].port, pwm_channels[i].pin, STC8H_GPIO_MODE_PUSH_PULL);
    }
}

static void pwm_groups_init(void)
{
    require_ok(stc8h_pwm_set_prescaler(STC8H_PWM_GROUP_A, PWM_PRESCALER));
    require_ok(stc8h_pwm_set_period(STC8H_PWM_GROUP_A, PWMA_PERIOD_TICKS));
    require_ok(stc8h_pwm_set_prescaler(STC8H_PWM_GROUP_B, PWM_PRESCALER));
    require_ok(stc8h_pwm_set_period(STC8H_PWM_GROUP_B, PWMB_PERIOD_TICKS));
}

static void pwm_channels_init(void)
{
    stc8h_u8 i;

    for (i = 0u; i < PWM_CHANNEL_COUNT; ++i) {
        require_ok(stc8h_pwm_init_channel(pwm_channels[i].group,
                                          pwm_channels[i].channel,
                                          pwm_channels[i].pin_select));
    }
}

static void pwm_set_pattern(stc8h_u8 pattern)
{
    stc8h_u8 i;
    stc8h_u16 duty;

    for (i = 0u; i < PWM_CHANNEL_COUNT; ++i) {
        duty = (pattern == 0u) ? pwm_channels[i].duty_a : pwm_channels[i].duty_b;
        require_ok(stc8h_pwm_set_duty(pwm_channels[i].group, pwm_channels[i].channel, duty));
    }
}

static void pwm_enable_all(void)
{
    stc8h_u8 i;

    for (i = 0u; i < PWM_CHANNEL_COUNT; ++i) {
        require_ok(stc8h_pwm_enable(pwm_channels[i].group, pwm_channels[i].channel));
    }
}

static void pwm_disable_alternate(void)
{
    stc8h_u8 i;

    for (i = 0u; i < PWM_CHANNEL_COUNT; i = (stc8h_u8)(i + 2u)) {
        require_ok(stc8h_pwm_disable(pwm_channels[i].group, pwm_channels[i].channel));
    }
}

static void pwm_disable_all(void)
{
    stc8h_u8 i;

    for (i = 0u; i < PWM_CHANNEL_COUNT; ++i) {
        require_ok(stc8h_pwm_disable(pwm_channels[i].group, pwm_channels[i].channel));
    }
}

void main(void)
{
    pwm_pins_init();
    pwm_groups_init();
    pwm_channels_init();

    while (1) {
        pwm_set_pattern(0u);
        pwm_enable_all();
        stc8h_delay_ms(1000u);

        pwm_set_pattern(1u);
        stc8h_delay_ms(1000u);

        pwm_disable_alternate();
        stc8h_delay_ms(1000u);

        pwm_enable_all();
        stc8h_delay_ms(1000u);

        pwm_disable_all();
        stc8h_delay_ms(1000u);
    }
}
