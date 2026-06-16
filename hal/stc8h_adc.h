#ifndef STC8H_ADC_H
#define STC8H_ADC_H

#include "stc8h_config.h"

#ifndef STC8H_ADC_INVALID_VALUE
#define STC8H_ADC_INVALID_VALUE 0xFFFFu
#endif

#ifndef STC8H_ADC_ENABLE_CHANNEL_CHECK
#define STC8H_ADC_ENABLE_CHANNEL_CHECK 1
#endif

#ifndef STC8H_ADC_BITS
#define STC8H_ADC_BITS 10u
#endif

#if (STC8H_ADC_BITS != 10u) && (STC8H_ADC_BITS != 12u)
#error "STC8H_ADC_BITS must be 10 or 12."
#endif

#ifndef STC8H_ADC_CHIP_CHANNEL_MASK
#define STC8H_ADC_CHIP_CHANNEL_MASK 0xFF03u
#endif

#ifndef STC8H_ADC_CHANNEL_MASK
#define STC8H_ADC_CHANNEL_MASK STC8H_ADC_CHIP_CHANNEL_MASK
#endif

void stc8h_adc_init(void);
stc8h_u16 stc8h_adc_read(stc8h_u8 channel);

#endif
