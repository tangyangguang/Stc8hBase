#include "stc8h_pwm.h"
#include "stc8h_sfr.h"

#define PWMA_ENO   STC8H_SFRX(0xFEB1u)
#define PWMA_PS    STC8H_SFRX(0xFEB2u)
#define PWMA_CR1   STC8H_SFRX(0xFEC0u)
#define PWMA_EGR   STC8H_SFRX(0xFEC7u)
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

#define PWMB_ENO   STC8H_SFRX(0xFEB5u)
#define PWMB_PS    STC8H_SFRX(0xFEB6u)
#define PWMB_CR1   STC8H_SFRX(0xFEE0u)
#define PWMB_EGR   STC8H_SFRX(0xFEE7u)
#define PWMB_CCMR1 STC8H_SFRX(0xFEE8u)
#define PWMB_CCMR2 STC8H_SFRX(0xFEE9u)
#define PWMB_CCMR3 STC8H_SFRX(0xFEEAu)
#define PWMB_CCMR4 STC8H_SFRX(0xFEEBu)
#define PWMB_CCER1 STC8H_SFRX(0xFEECu)
#define PWMB_CCER2 STC8H_SFRX(0xFEEDu)
#define PWMB_PSCRH STC8H_SFRX(0xFEF0u)
#define PWMB_PSCRL STC8H_SFRX(0xFEF1u)
#define PWMB_ARRH  STC8H_SFRX(0xFEF2u)
#define PWMB_ARRL  STC8H_SFRX(0xFEF3u)
#define PWMB_RCR   STC8H_SFRX(0xFEF4u)
#define PWMB_CCR5H STC8H_SFRX(0xFEF5u)
#define PWMB_CCR5L STC8H_SFRX(0xFEF6u)
#define PWMB_CCR6H STC8H_SFRX(0xFEF7u)
#define PWMB_CCR6L STC8H_SFRX(0xFEF8u)
#define PWMB_CCR7H STC8H_SFRX(0xFEF9u)
#define PWMB_CCR7L STC8H_SFRX(0xFEFAu)
#define PWMB_CCR8H STC8H_SFRX(0xFEFBu)
#define PWMB_CCR8L STC8H_SFRX(0xFEFCu)
#define PWMB_BKR   STC8H_SFRX(0xFEFDu)
#define PWMB_DTR   STC8H_SFRX(0xFEFEu)

#define STC8H_PWM_MODE1_PRELOAD 0x68u
#define STC8H_PWM_CR1_ARPE      0x80u
#define STC8H_PWM_CR1_CEN       0x01u
#define STC8H_PWM_BKR_MOE       0x80u
#define STC8H_PWM_EGR_UG        0x01u
#define STC8H_PWM_OUTPUT_MASK   0x55u

#ifndef STC8H_PWM_GROUP_MASK
#define STC8H_PWM_GROUP_MASK 0x03u
#endif

#ifndef STC8H_PWM_A_CHANNEL_MASK
#define STC8H_PWM_A_CHANNEL_MASK 0x0Fu
#endif

#ifndef STC8H_PWM_B_CHANNEL_MASK
#define STC8H_PWM_B_CHANNEL_MASK 0x0Fu
#endif

#define STC8H_PWM_GROUP_ENABLED(group) ((STC8H_PWM_GROUP_MASK & (1u << (group))) != 0u)
#define STC8H_PWM_A_CHANNEL_ENABLED(channel) ((STC8H_PWM_A_CHANNEL_MASK & (1u << ((channel) - 1u))) != 0u)
#define STC8H_PWM_B_CHANNEL_ENABLED(channel) ((STC8H_PWM_B_CHANNEL_MASK & (1u << ((channel) - 5u))) != 0u)

#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
static stc8h_u16 stc8h_pwm_period_a;
static stc8h_u16 stc8h_pwm_prescaler_a;
#endif
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
static stc8h_u16 stc8h_pwm_period_b;
static stc8h_u16 stc8h_pwm_prescaler_b;
#endif

static stc8h_u8 stc8h_pwm_group_valid(stc8h_u8 group)
{
    if (group == STC8H_PWM_GROUP_A) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
        return 1u;
#endif
    }
    if (group == STC8H_PWM_GROUP_B) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
        return 1u;
#endif
    }
    return 0u;
}

static void stc8h_pwm_write16(volatile stc8h_u8 *high, volatile stc8h_u8 *low, stc8h_u16 value)
{
    *high = (stc8h_u8)(value >> 8);
    *low = (stc8h_u8)value;
}

static stc8h_u8 stc8h_pwm_channel_output_mask(stc8h_u8 group, stc8h_u8 channel)
{
    if (group == STC8H_PWM_GROUP_A) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
#if STC8H_PWM_A_CHANNEL_ENABLED(1)
        if (channel == STC8H_PWM_CHANNEL_1) { return 0x01u; }
#endif
#if STC8H_PWM_A_CHANNEL_ENABLED(2)
        if (channel == STC8H_PWM_CHANNEL_2) { return 0x04u; }
#endif
#if STC8H_PWM_A_CHANNEL_ENABLED(3)
        if (channel == STC8H_PWM_CHANNEL_3) { return 0x10u; }
#endif
#if STC8H_PWM_A_CHANNEL_ENABLED(4)
        if (channel == STC8H_PWM_CHANNEL_4) { return 0x40u; }
#endif
#endif
    } else if (group == STC8H_PWM_GROUP_B) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
#if STC8H_PWM_B_CHANNEL_ENABLED(5)
        if (channel == STC8H_PWM_CHANNEL_5) { return 0x01u; }
#endif
#if STC8H_PWM_B_CHANNEL_ENABLED(6)
        if (channel == STC8H_PWM_CHANNEL_6) { return 0x04u; }
#endif
#if STC8H_PWM_B_CHANNEL_ENABLED(7)
        if (channel == STC8H_PWM_CHANNEL_7) { return 0x10u; }
#endif
#if STC8H_PWM_B_CHANNEL_ENABLED(8)
        if (channel == STC8H_PWM_CHANNEL_8) { return 0x40u; }
#endif
#endif
    }

    return 0u;
}

static stc8h_u8 stc8h_pwm_ps_shift(stc8h_u8 group, stc8h_u8 channel)
{
    if (group == STC8H_PWM_GROUP_A) {
        return (stc8h_u8)((channel - STC8H_PWM_CHANNEL_1) << 1);
    }
    return (stc8h_u8)((channel - STC8H_PWM_CHANNEL_5) << 1);
}

static stc8h_u16 stc8h_pwm_get_period(stc8h_u8 group)
{
    if (group == STC8H_PWM_GROUP_A) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
        return stc8h_pwm_period_a;
#else
        return 0u;
#endif
    }
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
    return stc8h_pwm_period_b;
#else
    return 0u;
#endif
}

static stc8h_u16 stc8h_pwm_get_prescaler(stc8h_u8 group)
{
    if (group == STC8H_PWM_GROUP_A) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
        return stc8h_pwm_prescaler_a;
#else
        return 0u;
#endif
    }
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
    return stc8h_pwm_prescaler_b;
#else
    return 0u;
#endif
}

static stc8h_u8 stc8h_pwm_group_running(stc8h_u8 group)
{
    return (group == STC8H_PWM_GROUP_A) ?
        (stc8h_u8)(PWMA_ENO & STC8H_PWM_OUTPUT_MASK) :
        (stc8h_u8)(PWMB_ENO & STC8H_PWM_OUTPUT_MASK);
}

static void stc8h_pwm_apply_prescaler(stc8h_u8 group)
{
    if (group == STC8H_PWM_GROUP_A) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
        stc8h_pwm_write16(&PWMA_PSCRH, &PWMA_PSCRL, stc8h_pwm_prescaler_a);
        PWMA_EGR = STC8H_PWM_EGR_UG;
#endif
    } else {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
        stc8h_pwm_write16(&PWMB_PSCRH, &PWMB_PSCRL, stc8h_pwm_prescaler_b);
        PWMB_EGR = STC8H_PWM_EGR_UG;
#endif
    }
}

static stc8h_status_t stc8h_pwm_set_mode(stc8h_u8 group, stc8h_u8 channel)
{
    if (group == STC8H_PWM_GROUP_A) {
        if (channel == STC8H_PWM_CHANNEL_1) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(1)
            PWMA_CCMR1 = (stc8h_u8)((PWMA_CCMR1 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMA_CCER1 &= (stc8h_u8)~0x03u;
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_2) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(2)
            PWMA_CCMR2 = (stc8h_u8)((PWMA_CCMR2 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMA_CCER1 &= (stc8h_u8)~0x30u;
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_3) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(3)
            PWMA_CCMR3 = (stc8h_u8)((PWMA_CCMR3 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMA_CCER2 &= (stc8h_u8)~0x03u;
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_4) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(4)
            PWMA_CCMR4 = (stc8h_u8)((PWMA_CCMR4 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMA_CCER2 &= (stc8h_u8)~0x30u;
            return STC8H_OK;
#endif
        }
    } else if (group == STC8H_PWM_GROUP_B) {
        if (channel == STC8H_PWM_CHANNEL_5) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(5)
            PWMB_CCMR1 = (stc8h_u8)((PWMB_CCMR1 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMB_CCER1 &= (stc8h_u8)~0x03u;
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_6) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(6)
            PWMB_CCMR2 = (stc8h_u8)((PWMB_CCMR2 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMB_CCER1 &= (stc8h_u8)~0x30u;
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_7) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(7)
            PWMB_CCMR3 = (stc8h_u8)((PWMB_CCMR3 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMB_CCER2 &= (stc8h_u8)~0x03u;
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_8) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(8)
            PWMB_CCMR4 = (stc8h_u8)((PWMB_CCMR4 & (stc8h_u8)~0x7Bu) | STC8H_PWM_MODE1_PRELOAD);
            PWMB_CCER2 &= (stc8h_u8)~0x30u;
            return STC8H_OK;
#endif
        }
    }

    return STC8H_ERROR;
}

stc8h_status_t stc8h_pwm_set_prescaler(stc8h_u8 group, stc8h_u16 prescaler)
{
    if (stc8h_pwm_group_valid(group) == 0u) {
        return STC8H_ERROR;
    }
    if (stc8h_pwm_group_running(group) != 0u) {
        return (prescaler == stc8h_pwm_get_prescaler(group)) ? STC8H_OK : STC8H_ERROR;
    }

    P_SW2 |= 0x80u;
    if (group == STC8H_PWM_GROUP_A) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
        stc8h_pwm_prescaler_a = prescaler;
#else
        return STC8H_ERROR;
#endif
    } else if (group == STC8H_PWM_GROUP_B) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
        stc8h_pwm_prescaler_b = prescaler;
#else
        return STC8H_ERROR;
#endif
    } else {
        return STC8H_ERROR;
    }
    stc8h_pwm_apply_prescaler(group);

    return STC8H_OK;
}

stc8h_status_t stc8h_pwm_set_period(stc8h_u8 group, stc8h_u16 period)
{
    if (period == 0u) {
        return STC8H_ERROR;
    }
    if (stc8h_pwm_group_valid(group) == 0u) {
        return STC8H_ERROR;
    }
    if (stc8h_pwm_group_running(group) != 0u) {
        return (period == stc8h_pwm_get_period(group)) ? STC8H_OK : STC8H_ERROR;
    }

    P_SW2 |= 0x80u;
    if (group == STC8H_PWM_GROUP_A) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A)
        PWMA_CR1 &= (stc8h_u8)~STC8H_PWM_CR1_CEN;
        stc8h_pwm_period_a = period;
        stc8h_pwm_write16(&PWMA_ARRH, &PWMA_ARRL, period);
        PWMA_RCR = 0u;
        PWMA_DTR = 0u;
        PWMA_BKR |= STC8H_PWM_BKR_MOE;
        PWMA_CR1 = (stc8h_u8)(PWMA_CR1 | STC8H_PWM_CR1_ARPE);
        PWMA_EGR = STC8H_PWM_EGR_UG;
#else
        return STC8H_ERROR;
#endif
    } else {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B)
        PWMB_CR1 &= (stc8h_u8)~STC8H_PWM_CR1_CEN;
        stc8h_pwm_period_b = period;
        stc8h_pwm_write16(&PWMB_ARRH, &PWMB_ARRL, period);
        PWMB_RCR = 0u;
        PWMB_DTR = 0u;
        PWMB_BKR |= STC8H_PWM_BKR_MOE;
        PWMB_CR1 = (stc8h_u8)(PWMB_CR1 | STC8H_PWM_CR1_ARPE);
        PWMB_EGR = STC8H_PWM_EGR_UG;
#else
        return STC8H_ERROR;
#endif
    }

    return STC8H_OK;
}

stc8h_status_t stc8h_pwm_init_channel(stc8h_u8 group, stc8h_u8 channel, stc8h_u8 pin_select)
{
    stc8h_u8 shift;
    stc8h_u8 mask;

    if ((pin_select > 3u) || (stc8h_pwm_channel_output_mask(group, channel) == 0u)) {
        return STC8H_ERROR;
    }

    P_SW2 |= 0x80u;
    shift = stc8h_pwm_ps_shift(group, channel);
    mask = (stc8h_u8)(0x03u << shift);

    if (group == STC8H_PWM_GROUP_A) {
        PWMA_PS = (stc8h_u8)((PWMA_PS & (stc8h_u8)~mask) | (stc8h_u8)(pin_select << shift));
    } else {
        PWMB_PS = (stc8h_u8)((PWMB_PS & (stc8h_u8)~mask) | (stc8h_u8)(pin_select << shift));
    }

    if (stc8h_pwm_set_mode(group, channel) != STC8H_OK) {
        return STC8H_ERROR;
    }

    return stc8h_pwm_set_duty(group, channel, 0u);
}

stc8h_status_t stc8h_pwm_set_duty(stc8h_u8 group, stc8h_u8 channel, stc8h_u16 duty)
{
#if STC8H_PWM_ENABLE_SET_DUTY_CLAMP
    stc8h_u16 period;
#endif

#if STC8H_PWM_ENABLE_SET_DUTY_CHANNEL_CHECK
    if (stc8h_pwm_channel_output_mask(group, channel) == 0u) {
        return STC8H_ERROR;
    }
#endif

#if STC8H_PWM_ENABLE_SET_DUTY_CLAMP
    period = stc8h_pwm_get_period(group);
    if (duty > period) {
        duty = period;
    }
#endif

    if (group == STC8H_PWM_GROUP_A) {
        if (channel == STC8H_PWM_CHANNEL_1) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(1)
            stc8h_pwm_write16(&PWMA_CCR1H, &PWMA_CCR1L, duty);
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_2) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(2)
            stc8h_pwm_write16(&PWMA_CCR2H, &PWMA_CCR2L, duty);
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_3) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(3)
            stc8h_pwm_write16(&PWMA_CCR3H, &PWMA_CCR3L, duty);
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_4) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_A) && STC8H_PWM_A_CHANNEL_ENABLED(4)
            stc8h_pwm_write16(&PWMA_CCR4H, &PWMA_CCR4L, duty);
            return STC8H_OK;
#endif
        }
    } else if (group == STC8H_PWM_GROUP_B) {
        if (channel == STC8H_PWM_CHANNEL_5) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(5)
            stc8h_pwm_write16(&PWMB_CCR5H, &PWMB_CCR5L, duty);
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_6) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(6)
            stc8h_pwm_write16(&PWMB_CCR6H, &PWMB_CCR6L, duty);
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_7) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(7)
            stc8h_pwm_write16(&PWMB_CCR7H, &PWMB_CCR7L, duty);
            return STC8H_OK;
#endif
        }
        if (channel == STC8H_PWM_CHANNEL_8) {
#if STC8H_PWM_GROUP_ENABLED(STC8H_PWM_GROUP_B) && STC8H_PWM_B_CHANNEL_ENABLED(8)
            stc8h_pwm_write16(&PWMB_CCR8H, &PWMB_CCR8L, duty);
            return STC8H_OK;
#endif
        }
    }

    return STC8H_ERROR;
}

stc8h_status_t stc8h_pwm_enable(stc8h_u8 group, stc8h_u8 channel)
{
    stc8h_u8 output_mask;

    output_mask = stc8h_pwm_channel_output_mask(group, channel);
    if (output_mask == 0u) {
        return STC8H_ERROR;
    }

    if (group == STC8H_PWM_GROUP_A) {
        PWMA_ENO |= output_mask;
        if ((channel == STC8H_PWM_CHANNEL_1) || (channel == STC8H_PWM_CHANNEL_3)) {
            if (channel == STC8H_PWM_CHANNEL_1) { PWMA_CCER1 |= 0x01u; }
            else { PWMA_CCER2 |= 0x01u; }
        } else {
            if (channel == STC8H_PWM_CHANNEL_2) { PWMA_CCER1 |= 0x10u; }
            else { PWMA_CCER2 |= 0x10u; }
        }
        PWMA_CR1 |= STC8H_PWM_CR1_CEN;
    } else {
        PWMB_ENO |= output_mask;
        if ((channel == STC8H_PWM_CHANNEL_5) || (channel == STC8H_PWM_CHANNEL_7)) {
            if (channel == STC8H_PWM_CHANNEL_5) { PWMB_CCER1 |= 0x01u; }
            else { PWMB_CCER2 |= 0x01u; }
        } else {
            if (channel == STC8H_PWM_CHANNEL_6) { PWMB_CCER1 |= 0x10u; }
            else { PWMB_CCER2 |= 0x10u; }
        }
        PWMB_CR1 |= STC8H_PWM_CR1_CEN;
    }

    return STC8H_OK;
}

#if STC8H_PWM_ENABLE_DISABLE
stc8h_status_t stc8h_pwm_disable(stc8h_u8 group, stc8h_u8 channel)
{
    stc8h_u8 output_mask;

    output_mask = stc8h_pwm_channel_output_mask(group, channel);
    if (output_mask == 0u) {
        return STC8H_ERROR;
    }

    if (group == STC8H_PWM_GROUP_A) {
        PWMA_ENO &= (stc8h_u8)~output_mask;
        if (channel == STC8H_PWM_CHANNEL_1) { PWMA_CCER1 &= (stc8h_u8)~0x01u; }
        else if (channel == STC8H_PWM_CHANNEL_2) { PWMA_CCER1 &= (stc8h_u8)~0x10u; }
        else if (channel == STC8H_PWM_CHANNEL_3) { PWMA_CCER2 &= (stc8h_u8)~0x01u; }
        else { PWMA_CCER2 &= (stc8h_u8)~0x10u; }
        if ((PWMA_ENO & STC8H_PWM_OUTPUT_MASK) == 0u) {
            PWMA_CR1 &= (stc8h_u8)~STC8H_PWM_CR1_CEN;
        }
    } else {
        PWMB_ENO &= (stc8h_u8)~output_mask;
        if (channel == STC8H_PWM_CHANNEL_5) { PWMB_CCER1 &= (stc8h_u8)~0x01u; }
        else if (channel == STC8H_PWM_CHANNEL_6) { PWMB_CCER1 &= (stc8h_u8)~0x10u; }
        else if (channel == STC8H_PWM_CHANNEL_7) { PWMB_CCER2 &= (stc8h_u8)~0x01u; }
        else { PWMB_CCER2 &= (stc8h_u8)~0x10u; }
        if ((PWMB_ENO & STC8H_PWM_OUTPUT_MASK) == 0u) {
            PWMB_CR1 &= (stc8h_u8)~STC8H_PWM_CR1_CEN;
        }
    }

    return STC8H_OK;
}
#endif
