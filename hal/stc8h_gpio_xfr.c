#include "stc8h_gpio.h"
#include "stc8h_sfr.h"

static stc8h_u8 stc8h_gpio_xfr_apply_mask(stc8h_u8 value, stc8h_u8 mask, stc8h_u8 enable)
{
    if (enable != 0u) {
        return (stc8h_u8)(value | mask);
    }
    return (stc8h_u8)(value & (stc8h_u8)~mask);
}

stc8h_status_t stc8h_gpio_set_pullup_mask(stc8h_u8 port, stc8h_u8 mask, stc8h_u8 enable)
{
    P_SW2 |= 0x80u;

    if (port == 1u) {
        P1PU = stc8h_gpio_xfr_apply_mask(P1PU, mask, enable);
        return STC8H_OK;
    }
    if (port == 3u) {
        P3PU = stc8h_gpio_xfr_apply_mask(P3PU, mask, enable);
        return STC8H_OK;
    }
    if (port == 5u) {
        P5PU = stc8h_gpio_xfr_apply_mask(P5PU, mask, enable);
        return STC8H_OK;
    }

    return STC8H_ERROR;
}

stc8h_status_t stc8h_gpio_set_input_enable_mask(stc8h_u8 port, stc8h_u8 mask, stc8h_u8 enable)
{
    P_SW2 |= 0x80u;

    if (port == 1u) {
        P1IE = stc8h_gpio_xfr_apply_mask(P1IE, mask, enable);
        return STC8H_OK;
    }
    if (port == 3u) {
        P3IE = stc8h_gpio_xfr_apply_mask(P3IE, mask, enable);
        return STC8H_OK;
    }
    if (port == 5u) {
        P5IE = stc8h_gpio_xfr_apply_mask(P5IE, mask, enable);
        return STC8H_OK;
    }

    return STC8H_ERROR;
}
