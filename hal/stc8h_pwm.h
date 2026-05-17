#ifndef STC8H_PWM_H
#define STC8H_PWM_H

#include "stc8h_config.h"

#define STC8H_PWM_GROUP_A 0u
#define STC8H_PWM_GROUP_B 1u

#define STC8H_PWM_CHANNEL_1 1u
#define STC8H_PWM_CHANNEL_2 2u
#define STC8H_PWM_CHANNEL_3 3u
#define STC8H_PWM_CHANNEL_4 4u
#define STC8H_PWM_CHANNEL_5 5u
#define STC8H_PWM_CHANNEL_6 6u
#define STC8H_PWM_CHANNEL_7 7u
#define STC8H_PWM_CHANNEL_8 8u

#define STC8H_PWM_PIN_PWM1_P10 0u
#define STC8H_PWM_PIN_PWM1_P20 1u
#define STC8H_PWM_PIN_PWM1_P60 2u

#define STC8H_PWM_PIN_PWM2_P12 0u
#define STC8H_PWM_PIN_PWM2_P22 1u
#define STC8H_PWM_PIN_PWM2_P62 2u

#define STC8H_PWM_PIN_PWM3_P14 0u
#define STC8H_PWM_PIN_PWM3_P24 1u
#define STC8H_PWM_PIN_PWM3_P64 2u

#define STC8H_PWM_PIN_PWM4_P16 0u
#define STC8H_PWM_PIN_PWM4_P26 1u
#define STC8H_PWM_PIN_PWM4_P66 2u
#define STC8H_PWM_PIN_PWM4_P34 3u

#define STC8H_PWM_PIN_PWM5_P20 0u
#define STC8H_PWM_PIN_PWM5_P17 1u
#define STC8H_PWM_PIN_PWM5_P00 2u
#define STC8H_PWM_PIN_PWM5_P74 3u

#define STC8H_PWM_PIN_PWM6_P21 0u
#define STC8H_PWM_PIN_PWM6_P54 1u
#define STC8H_PWM_PIN_PWM6_P01 2u
#define STC8H_PWM_PIN_PWM6_P75 3u

#define STC8H_PWM_PIN_PWM7_P22 0u
#define STC8H_PWM_PIN_PWM7_P33 1u
#define STC8H_PWM_PIN_PWM7_P02 2u
#define STC8H_PWM_PIN_PWM7_P76 3u

#define STC8H_PWM_PIN_PWM8_P23 0u
#define STC8H_PWM_PIN_PWM8_P34 1u
#define STC8H_PWM_PIN_PWM8_P03 2u
#define STC8H_PWM_PIN_PWM8_P77 3u

#ifndef STC8H_PWM_ENABLE_DISABLE
#define STC8H_PWM_ENABLE_DISABLE 1
#endif

/* Hot-path trimming switches. Default keeps the existing safe behaviour;
 * fixed-channel apps can set these to 0 once they have validated that
 * the duty value is already bounded and the (group, channel) pair is
 * guaranteed valid by board configuration. */
#ifndef STC8H_PWM_ENABLE_SET_DUTY_CHANNEL_CHECK
#define STC8H_PWM_ENABLE_SET_DUTY_CHANNEL_CHECK 1
#endif
#ifndef STC8H_PWM_ENABLE_SET_DUTY_CLAMP
#define STC8H_PWM_ENABLE_SET_DUTY_CLAMP 1
#endif

/* When 0, the per-group period and prescaler shadow variables are
 * not stored in RAM. Apps that set the prescaler / period exactly
 * once at init and rely on STC8H_PWM_ENABLE_SET_DUTY_CLAMP=0 can
 * shave 4-8 bytes of XDATA (one shadow pair per enabled group).
 * When 0, stc8h_pwm_set_period and stc8h_pwm_set_prescaler still
 * write the SFRs but skip the "running guard" that requires
 * remembering the previously stored value. Default 1 preserves the
 * existing run-time guard. */
#ifndef STC8H_PWM_TRACK_PERIOD_PRESCALER
#define STC8H_PWM_TRACK_PERIOD_PRESCALER 1
#endif

#if !STC8H_PWM_TRACK_PERIOD_PRESCALER && STC8H_PWM_ENABLE_SET_DUTY_CLAMP
#error "STC8H_PWM_ENABLE_SET_DUTY_CLAMP requires STC8H_PWM_TRACK_PERIOD_PRESCALER."
#endif

stc8h_status_t stc8h_pwm_set_prescaler(stc8h_u8 group, stc8h_u16 prescaler);
stc8h_status_t stc8h_pwm_set_period(stc8h_u8 group, stc8h_u16 period);
stc8h_status_t stc8h_pwm_init_channel(stc8h_u8 group, stc8h_u8 channel, stc8h_u8 pin_select);
stc8h_status_t stc8h_pwm_set_duty(stc8h_u8 group, stc8h_u8 channel, stc8h_u16 duty);
stc8h_status_t stc8h_pwm_enable(stc8h_u8 group, stc8h_u8 channel);
#if STC8H_PWM_ENABLE_DISABLE
stc8h_status_t stc8h_pwm_disable(stc8h_u8 group, stc8h_u8 channel);
#endif

#endif
