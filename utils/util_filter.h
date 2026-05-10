#ifndef UTIL_FILTER_H
#define UTIL_FILTER_H

#include "stc8h_config.h"

stc8h_u16 util_filter_limit_u16(stc8h_u16 value, stc8h_u16 min_value, stc8h_u16 max_value);
stc8h_u16 util_filter_iir_u16(stc8h_u16 current, stc8h_u16 input, stc8h_u8 shift);

#endif
