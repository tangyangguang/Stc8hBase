#include "stc8h_pwm.h"
#include "stc8h_sfr.h"

#define PWMA_ENO   STC8H_SFRX(0xFEB1u)
#define PWMA_PS    STC8H_SFRX(0xFEB2u)
#define PWMA_CR1   STC8H_SFRX(0xFEC0u)
#define PWMA_CCMR1 STC8H_SFRX(0xFEC8u)
#define PWMA_CCMR2 STC8H_SFRX(0xFEC9u)
#define PWMA_CCMR3 STC8H_SFRX(0xFECAu)
#define PWMA_CCMR4 STC8H_SFRX(0xFECBu)
#define PWMA_CCER1 STC8H_SFRX(0xFECCu)
#define PWMA_CCER2 STC8H_SFRX(0xFECDu)
#define PWMA_PSCRH STC8H_SFRX(0xFED0u)
#define PWMA_PSCRL STC8H_SFRX(0xFED1u)
#define PWMA_ARRH  STC8H_SFRX(0xFED2u)
#define PWMA_ARRL  STC8H_SFRX(0xFED3u)
#define PWMA_RCR   STC8H_SFRX(0xFED4u)
#define PWMA_CCR1H STC8H_SFRX(0xFED5u)
#define PWMA_CCR1L STC8H_SFRX(0xFED6u)
#define PWMA_CCR2H STC8H_SFRX(0xFED7u)
#define PWMA_CCR2L STC8H_SFRX(0xFED8u)
#define PWMA_CCR3H STC8H_SFRX(0xFED9u)
#define PWMA_CCR3L STC8H_SFRX(0xFEDAu)
#define PWMA_CCR4H STC8H_SFRX(0xFEDBu)
#define PWMA_CCR4L STC8H_SFRX(0xFEDCu)
#define PWMA_BKR   STC8H_SFRX(0xFEDDu)
#define PWMA_DTR   STC8H_SFRX(0xFEDEu)

#define STC8H_PWM_MODE1_PRELOAD 0x68u
#define STC8H_PWM_CR1_ARPE      0x80u
#define STC8H_PWM_CR1_CEN       0x01u
#define STC8H_PWM_BKR_MOE       0x80u

#ifndef STC8H_PWM_CHANNEL_MASK
#define STC8H_PWM_CHANNEL_MASK 0x0Fu
#endif

#define STC8H_PWM_CHANNEL_ENABLED(channel) ((STC8H_PWM_CHANNEL_MASK & (1u << ((channel) - 1u))) != 0u)

static stc8h_u16 stc8h_pwm_period;

static stc8h_u8 stc8h_pwm_output_mask(stc8h_u8 channel)
{
#if STC8H_PWM_CHANNEL_ENABLED(1)
    if (channel == STC8H_PWM_CHANNEL_1) {
        return 0x01u;
    }
#endif
#if STC8H_PWM_CHANNEL_ENABLED(2)
    if (channel == STC8H_PWM_CHANNEL_2) {
        return 0x04u;
    }
#endif
#if STC8H_PWM_CHANNEL_ENABLED(3)
    if (channel == STC8H_PWM_CHANNEL_3) {
        return 0x10u;
    }
#endif
#if STC8H_PWM_CHANNEL_ENABLED(4)
    if (channel == STC8H_PWM_CHANNEL_4) {
        return 0x40u;
    }
#endif
    return 0u;
}

static stc8h_u8 stc8h_pwm_channel_ps_shift(stc8h_u8 channel)
{
    if ((channel < STC8H_PWM_CHANNEL_1) || (channel > STC8H_PWM_CHANNEL_4)) {
        return 0u;
    }

    return (stc8h_u8)((channel - 1u) << 1);
}

static void stc8h_pwm_write16(volatile stc8h_u8 *high, volatile stc8h_u8 *low, stc8h_u16 value)
{
    *high = (stc8h_u8)(value >> 8);
    *low = (stc8h_u8)value;
}

static stc8h_status_t stc8h_pwm_set_channel_mode(stc8h_u8 channel)
{
#if STC8H_PWM_CHANNEL_ENABLED(1)
    if (channel == STC8H_PWM_CHANNEL_1) {
        PWMA_CCMR1 = (stc8h_u8)((PWMA_CCMR1 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
        PWMA_CCER1 &= (stc8h_u8)~0x03u;
        return STC8H_OK;
    }
#endif
#if STC8H_PWM_CHANNEL_ENABLED(2)
    if (channel == STC8H_PWM_CHANNEL_2) {
        PWMA_CCMR2 = (stc8h_u8)((PWMA_CCMR2 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
        PWMA_CCER1 &= (stc8h_u8)~0x30u;
        return STC8H_OK;
    }
#endif
#if STC8H_PWM_CHANNEL_ENABLED(3)
    if (channel == STC8H_PWM_CHANNEL_3) {
        PWMA_CCMR3 = (stc8h_u8)((PWMA_CCMR3 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
        PWMA_CCER2 &= (stc8h_u8)~0x03u;
        return STC8H_OK;
    }
#endif
#if STC8H_PWM_CHANNEL_ENABLED(4)
    if (channel == STC8H_PWM_CHANNEL_4) {
        PWMA_CCMR4 = (stc8h_u8)((PWMA_CCMR4 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
        PWMA_CCER2 &= (stc8h_u8)~0x30u;
        return STC8H_OK;
    }
#endif

    return STC8H_ERROR;
}

stc8h_status_t stc8h_pwm_init_period(stc8h_u16 period)
{
    if (period == 0u) {
        return STC8H_ERROR;
    }

    P_SW2 |= 0x80u;
    PWMA_CR1 &= (stc8h_u8)~STC8H_PWM_CR1_CEN;
    stc8h_pwm_period = period;

    PWMA_PSCRH = 0u;
    PWMA_PSCRL = 0u;
    stc8h_pwm_write16(&PWMA_ARRH, &PWMA_ARRL, period);
    PWMA_RCR = 0u;
    PWMA_DTR = 0u;
    PWMA_BKR |= STC8H_PWM_BKR_MOE;
    PWMA_CR1 = (stc8h_u8)(PWMA_CR1 | STC8H_PWM_CR1_ARPE);

    return STC8H_OK;
}

stc8h_status_t stc8h_pwm_channel_init(stc8h_u8 channel)
{
    stc8h_u8 ps_shift;
    stc8h_u8 ps_mask;

    if (stc8h_pwm_output_mask(channel) == 0u) {
        return STC8H_ERROR;
    }

    P_SW2 |= 0x80u;
    ps_shift = stc8h_pwm_channel_ps_shift(channel);
    ps_mask = (stc8h_u8)(0x03u << ps_shift);
    PWMA_PS &= (stc8h_u8)~ps_mask;

    if (stc8h_pwm_set_channel_mode(channel) != STC8H_OK) {
        return STC8H_ERROR;
    }

    return stc8h_pwm_set_duty(channel, 0u);
}

stc8h_status_t stc8h_pwm_init(stc8h_u8 channel, stc8h_u16 period)
{
    if (stc8h_pwm_init_period(period) != STC8H_OK) {
        return STC8H_ERROR;
    }
    return stc8h_pwm_channel_init(channel);
}

stc8h_status_t stc8h_pwm_set_duty(stc8h_u8 channel, stc8h_u16 duty)
{
    if (stc8h_pwm_output_mask(channel) == 0u) {
        return STC8H_ERROR;
    }

    if (duty > stc8h_pwm_period) {
        duty = stc8h_pwm_period;
    }

    if (channel == STC8H_PWM_CHANNEL_1) {
#if STC8H_PWM_CHANNEL_ENABLED(1)
        stc8h_pwm_write16(&PWMA_CCR1H, &PWMA_CCR1L, duty);
        return STC8H_OK;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_2) {
#if STC8H_PWM_CHANNEL_ENABLED(2)
        stc8h_pwm_write16(&PWMA_CCR2H, &PWMA_CCR2L, duty);
        return STC8H_OK;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_3) {
#if STC8H_PWM_CHANNEL_ENABLED(3)
        stc8h_pwm_write16(&PWMA_CCR3H, &PWMA_CCR3L, duty);
        return STC8H_OK;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_4) {
#if STC8H_PWM_CHANNEL_ENABLED(4)
        stc8h_pwm_write16(&PWMA_CCR4H, &PWMA_CCR4L, duty);
        return STC8H_OK;
#endif
    }

    return STC8H_ERROR;
}

stc8h_status_t stc8h_pwm_enable(stc8h_u8 channel)
{
    stc8h_u8 output_mask;

    output_mask = stc8h_pwm_output_mask(channel);
    if (output_mask == 0u) {
        return STC8H_ERROR;
    }

    PWMA_ENO |= output_mask;
    if (channel == STC8H_PWM_CHANNEL_1) {
#if STC8H_PWM_CHANNEL_ENABLED(1)
        PWMA_CCER1 |= 0x01u;
        PWMA_CR1 |= STC8H_PWM_CR1_CEN;
        return STC8H_OK;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_2) {
#if STC8H_PWM_CHANNEL_ENABLED(2)
        PWMA_CCER1 |= 0x10u;
        PWMA_CR1 |= STC8H_PWM_CR1_CEN;
        return STC8H_OK;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_3) {
#if STC8H_PWM_CHANNEL_ENABLED(3)
        PWMA_CCER2 |= 0x01u;
        PWMA_CR1 |= STC8H_PWM_CR1_CEN;
        return STC8H_OK;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_4) {
#if STC8H_PWM_CHANNEL_ENABLED(4)
        PWMA_CCER2 |= 0x10u;
        PWMA_CR1 |= STC8H_PWM_CR1_CEN;
        return STC8H_OK;
#endif
    }

    return STC8H_ERROR;
}

stc8h_status_t stc8h_pwm_disable(stc8h_u8 channel)
{
    stc8h_u8 output_mask;

    output_mask = stc8h_pwm_output_mask(channel);
    if (output_mask == 0u) {
        return STC8H_ERROR;
    }

    PWMA_ENO &= (stc8h_u8)~output_mask;
    if (channel == STC8H_PWM_CHANNEL_1) {
#if STC8H_PWM_CHANNEL_ENABLED(1)
        PWMA_CCER1 &= (stc8h_u8)~0x01u;
        goto check_stop;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_2) {
#if STC8H_PWM_CHANNEL_ENABLED(2)
        PWMA_CCER1 &= (stc8h_u8)~0x10u;
        goto check_stop;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_3) {
#if STC8H_PWM_CHANNEL_ENABLED(3)
        PWMA_CCER2 &= (stc8h_u8)~0x01u;
        goto check_stop;
#endif
    }
    if (channel == STC8H_PWM_CHANNEL_4) {
#if STC8H_PWM_CHANNEL_ENABLED(4)
        PWMA_CCER2 &= (stc8h_u8)~0x10u;
        goto check_stop;
#endif
    }

    return STC8H_ERROR;

check_stop:
    if ((PWMA_ENO & 0x55u) == 0u) {
        PWMA_CR1 &= (stc8h_u8)~STC8H_PWM_CR1_CEN;
    }

    return STC8H_OK;
}
