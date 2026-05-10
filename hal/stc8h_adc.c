#include "stc8h_adc.h"
#include "stc8h_delay.h"
#include "stc8h_sfr.h"

#ifndef STC8H_ADC_POWER
#define STC8H_ADC_POWER 0x80u
#endif

#ifndef STC8H_ADC_START
#define STC8H_ADC_START 0x40u
#endif

#ifndef STC8H_ADC_FLAG
#define STC8H_ADC_FLAG 0x20u
#endif

#ifndef STC8H_ADC_RESFMT_RIGHT
#define STC8H_ADC_RESFMT_RIGHT 0x20u
#endif

#ifndef STC8H_ADC_SPEED_SLOWEST
#define STC8H_ADC_SPEED_SLOWEST 0x0Fu
#endif

#ifndef STC8H_ADC_TIMEOUT_LOOPS
#define STC8H_ADC_TIMEOUT_LOOPS 60000u
#endif

static stc8h_u8 stc8h_adc_channel_valid(stc8h_u8 channel)
{
    return ((channel <= 1u) || ((channel >= 8u) && (channel <= 14u)) || (channel == 15u)) ? 1u : 0u;
}

void stc8h_adc_init(void)
{
    P_SW2 |= 0x80u;
    ADCTIM = 0x3Fu;
    P_SW2 &= (stc8h_u8)~0x80u;

    ADCCFG = (stc8h_u8)(STC8H_ADC_RESFMT_RIGHT | STC8H_ADC_SPEED_SLOWEST);
    ADC_CONTR = STC8H_ADC_POWER;
    stc8h_delay_ms(1u);
}

stc8h_u16 stc8h_adc_read(stc8h_u8 channel)
{
    stc8h_u16 timeout;

    if (stc8h_adc_channel_valid(channel) == 0u) {
        return STC8H_ADC_INVALID_VALUE;
    }

    ADC_CONTR = (stc8h_u8)(STC8H_ADC_POWER | (channel & 0x0Fu));
    ADC_CONTR |= STC8H_ADC_START;
    STC8H_NOP();
    STC8H_NOP();

    timeout = STC8H_ADC_TIMEOUT_LOOPS;
    while ((ADC_CONTR & STC8H_ADC_FLAG) == 0u) {
        --timeout;
        if (timeout == 0u) {
            return STC8H_ADC_INVALID_VALUE;
        }
    }

    ADC_CONTR &= (stc8h_u8)~STC8H_ADC_FLAG;
    return (stc8h_u16)(((stc8h_u16)(ADC_RES & 0x03u) << 8) | ADC_RESL);
}
