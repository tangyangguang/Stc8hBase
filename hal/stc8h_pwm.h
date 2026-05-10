#ifndef STC8H_PWM_H
#define STC8H_PWM_H

#include "stc8h_config.h"

#define STC8H_PWM_CHANNEL_1 1u
#define STC8H_PWM_CHANNEL_2 2u
#define STC8H_PWM_CHANNEL_3 3u
#define STC8H_PWM_CHANNEL_4 4u

stc8h_status_t stc8h_pwm_init(stc8h_u8 channel, stc8h_u16 period);
stc8h_status_t stc8h_pwm_set_duty(stc8h_u8 channel, stc8h_u16 duty);
stc8h_status_t stc8h_pwm_enable(stc8h_u8 channel);
stc8h_status_t stc8h_pwm_disable(stc8h_u8 channel);

#endif
