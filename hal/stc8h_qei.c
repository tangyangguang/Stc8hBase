#include "stc8h_qei.h"
#include "stc8h_sfr.h"

#define PWMB_ENO   STC8H_SFRX(0xFEB5u)
#define PWMB_PS    STC8H_SFRX(0xFEB6u)
#define PWMB_CR1   STC8H_SFRX(0xFEE0u)
#define PWMB_SMCR  STC8H_SFRX(0xFEE2u)
#define PWMB_IER   STC8H_SFRX(0xFEE4u)
#define PWMB_SR1   STC8H_SFRX(0xFEE5u)
#define PWMB_SR2   STC8H_SFRX(0xFEE6u)
#define PWMB_EGR   STC8H_SFRX(0xFEE7u)
#define PWMB_CCMR1 STC8H_SFRX(0xFEE8u)
#define PWMB_CCMR2 STC8H_SFRX(0xFEE9u)
#define PWMB_CCER1 STC8H_SFRX(0xFEECu)
#define PWMB_CCER2 STC8H_SFRX(0xFEEDu)
#define PWMB_CNTRH STC8H_SFRX(0xFEEEu)
#define PWMB_CNTRL STC8H_SFRX(0xFEEFu)
#define PWMB_PSCRH STC8H_SFRX(0xFEF0u)
#define PWMB_PSCRL STC8H_SFRX(0xFEF1u)
#define PWMB_ARRH  STC8H_SFRX(0xFEF2u)
#define PWMB_ARRL  STC8H_SFRX(0xFEF3u)
#define PWMB_RCR   STC8H_SFRX(0xFEF4u)

#define STC8H_QEI_EAXFR             0x80u
#define STC8H_QEI_CR1_CEN           0x01u
#define STC8H_QEI_SMCR_MODE_MASK    0x07u
#define STC8H_QEI_CCER1_INPUT_MASK  0x33u
#define STC8H_QEI_CCER1_ENABLE_BOTH 0x11u
#define STC8H_QEI_CCMR_DIRECT_INPUT 0x01u
#define STC8H_QEI_EGR_UPDATE        0x01u
#define STC8H_QEI_PS_INPUT_MASK     0x0Fu

void stc8h_qei_pwmb_init(void)
{
    P_SW2 |= STC8H_QEI_EAXFR;

    PWMB_CR1 &= (stc8h_u8)~STC8H_QEI_CR1_CEN;
    PWMB_ENO = 0u;
    PWMB_IER = 0u;
    PWMB_SMCR &= (stc8h_u8)~STC8H_QEI_SMCR_MODE_MASK;
    PWMB_CCER1 &= (stc8h_u8)~STC8H_QEI_CCER1_INPUT_MASK;
    PWMB_CCER2 = 0u;

    PWMB_PS = (stc8h_u8)((PWMB_PS & (stc8h_u8)~STC8H_QEI_PS_INPUT_MASK) |
                         STC8H_QEI_PWMB_TI5_PIN_SELECT |
                         (STC8H_QEI_PWMB_TI6_PIN_SELECT << 2));

    PWMB_CCMR1 = (stc8h_u8)((STC8H_QEI_PWMB_FILTER << 4) |
                            STC8H_QEI_CCMR_DIRECT_INPUT);
    PWMB_CCMR2 = (stc8h_u8)((STC8H_QEI_PWMB_FILTER << 4) |
                            STC8H_QEI_CCMR_DIRECT_INPUT);
    PWMB_CCER1 = STC8H_QEI_CCER1_ENABLE_BOTH;

    PWMB_CR1 = 0u;
    PWMB_PSCRH = 0u;
    PWMB_PSCRL = 0u;
    PWMB_ARRH = 0xFFu;
    PWMB_ARRL = 0xFFu;
    PWMB_RCR = 0u;
    PWMB_CNTRH = 0u;
    PWMB_CNTRL = 0u;
    PWMB_EGR = STC8H_QEI_EGR_UPDATE;
    PWMB_SR1 = 0u;
    PWMB_SR2 = 0u;

    PWMB_SMCR = STC8H_QEI_PWMB_MODE;
    PWMB_CR1 = STC8H_QEI_CR1_CEN;
}

stc8h_u16 stc8h_qei_pwmb_read(void)
{
    stc8h_u8 high_before;
    stc8h_u8 high_after;
    stc8h_u8 low;

    P_SW2 |= STC8H_QEI_EAXFR;
    do {
        high_before = PWMB_CNTRH;
        low = PWMB_CNTRL;
        high_after = PWMB_CNTRH;
    } while (high_before != high_after);

    return (stc8h_u16)(((stc8h_u16)high_after << 8) | low);
}

void stc8h_qei_pwmb_stop(void)
{
    P_SW2 |= STC8H_QEI_EAXFR;

    PWMB_CR1 &= (stc8h_u8)~STC8H_QEI_CR1_CEN;
    PWMB_SMCR &= (stc8h_u8)~STC8H_QEI_SMCR_MODE_MASK;
    PWMB_CCER1 &= (stc8h_u8)~STC8H_QEI_CCER1_INPUT_MASK;
    PWMB_CCMR1 = 0u;
    PWMB_CCMR2 = 0u;
}
