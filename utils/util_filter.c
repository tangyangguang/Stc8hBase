#include "util_filter.h"

stc8h_u16 util_filter_limit_u16(stc8h_u16 value, stc8h_u16 min_value, stc8h_u16 max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

stc8h_u16 util_filter_iir_u16(stc8h_u16 current, stc8h_u16 input, stc8h_u8 shift)
{
    stc8h_u16 delta;
    stc8h_u16 step;

    if (shift == 0u) {
        return input;
    }

    if (input > current) {
        delta = (stc8h_u16)(input - current);
        step = (stc8h_u16)(delta >> shift);
        if (step == 0u) {
            step = 1u;
        }
        return (stc8h_u16)(current + step);
    }

    if (input < current) {
        delta = (stc8h_u16)(current - input);
        step = (stc8h_u16)(delta >> shift);
        if (step == 0u) {
            step = 1u;
        }
        return (stc8h_u16)(current - step);
    }

    return current;
}
