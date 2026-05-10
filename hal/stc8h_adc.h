#ifndef STC8H_ADC_H
#define STC8H_ADC_H

#include "stc8h_config.h"

#ifndef STC8H_ADC_INVALID_VALUE
#define STC8H_ADC_INVALID_VALUE 0xFFFFu
#endif

void stc8h_adc_init(void);
stc8h_u16 stc8h_adc_read(stc8h_u8 channel);

#endif
