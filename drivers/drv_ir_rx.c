#include "drv_ir_rx.h"

#ifndef DRV_IR_RX_ENABLE_PULSE
#define DRV_IR_RX_ENABLE_PULSE 1
#endif

#ifndef DRV_IR_RX_ENABLE_FALLING
#define DRV_IR_RX_ENABLE_FALLING 1
#endif

#define DRV_IR_RX_STATE_IDLE       0u
#define DRV_IR_RX_STATE_LEAD_SPACE 1u
#define DRV_IR_RX_STATE_BIT_MARK   2u
#define DRV_IR_RX_STATE_BIT_SPACE  3u
#define DRV_IR_RX_STATE_STOP_MARK  4u
#define DRV_IR_RX_STATE_FALLING_BITS 5u
#define DRV_IR_RX_STATE_FALLING_REPEAT 6u

#define DRV_IR_LEVEL_MARK  0u
#define DRV_IR_LEVEL_SPACE 1u

static stc8h_u8 drv_ir_rx_between(stc8h_u16 value, stc8h_u16 low, stc8h_u16 high)
{
    return ((value >= low) && (value <= high)) ? STC8H_TRUE : STC8H_FALSE;
}

#if DRV_IR_RX_ENABLE_PULSE
static stc8h_u8 drv_ir_rx_mark_9000(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 7200u, 10800u);
}

static stc8h_u8 drv_ir_rx_mark_562(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 400u, 750u);
}

static stc8h_u8 drv_ir_rx_space_4500(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 3600u, 5400u);
}

static stc8h_u8 drv_ir_rx_space_2250(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 1800u, 2700u);
}

static stc8h_u8 drv_ir_rx_space_562(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 400u, 750u);
}

static stc8h_u8 drv_ir_rx_space_1687(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 1350u, 2050u);
}
#endif

#if DRV_IR_RX_ENABLE_FALLING
static stc8h_u8 drv_ir_rx_falling_lead(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 12500u, 16000u);
}

static stc8h_u8 drv_ir_rx_falling_repeat(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 10000u, 12500u);
}

static stc8h_u8 drv_ir_rx_falling_bit_0(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 900u, 1350u);
}

static stc8h_u8 drv_ir_rx_falling_bit_1(stc8h_u16 width_us)
{
    return drv_ir_rx_between(width_us, 1800u, 2700u);
}
#endif

static void drv_ir_rx_set_event(drv_ir_rx_t *rx, drv_ir_rx_event_t event);
static void drv_ir_rx_finish_frame(drv_ir_rx_t *rx);

static void drv_ir_rx_push_bit(drv_ir_rx_t *rx, stc8h_u8 bit)
{
    rx->raw >>= 1;
    if (bit != 0u) {
        rx->raw |= 0x80000000UL;
    }
    ++rx->bit_index;
}

#if DRV_IR_RX_ENABLE_FALLING
static stc8h_u8 drv_ir_rx_falling_bit(drv_ir_rx_t *rx, stc8h_u16 width_us)
{
    if (drv_ir_rx_falling_bit_0(width_us) != 0u) {
        drv_ir_rx_push_bit(rx, 0u);
    } else if (drv_ir_rx_falling_bit_1(width_us) != 0u) {
        drv_ir_rx_push_bit(rx, 1u);
    } else {
        return STC8H_FALSE;
    }

    if (rx->bit_index >= 32u) {
        drv_ir_rx_finish_frame(rx);
    }
    return STC8H_TRUE;
}
#endif

static void drv_ir_rx_set_event(drv_ir_rx_t *rx, drv_ir_rx_event_t event)
{
    if (rx->event == DRV_IR_RX_EVENT_NONE) {
        rx->event = event;
    }
}

static void drv_ir_rx_error(drv_ir_rx_t *rx)
{
    rx->state = DRV_IR_RX_STATE_IDLE;
    rx->bit_index = 0u;
    rx->raw = 0u;
    drv_ir_rx_set_event(rx, DRV_IR_RX_EVENT_ERROR);
}

static void drv_ir_rx_finish_frame(drv_ir_rx_t *rx)
{
    stc8h_u8 address;
    stc8h_u8 address_inv;
    stc8h_u8 command;
    stc8h_u8 command_inv;

    address = (stc8h_u8)rx->raw;
    address_inv = (stc8h_u8)(rx->raw >> 8);
    command = (stc8h_u8)(rx->raw >> 16);
    command_inv = (stc8h_u8)(rx->raw >> 24);

    rx->state = DRV_IR_RX_STATE_IDLE;
    rx->bit_index = 0u;

    if (((stc8h_u8)(address ^ address_inv) != 0xFFu) ||
        ((stc8h_u8)(command ^ command_inv) != 0xFFu)) {
        drv_ir_rx_set_event(rx, DRV_IR_RX_EVENT_ERROR);
        return;
    }

    if (rx->event == DRV_IR_RX_EVENT_NONE) {
        rx->frame.address = address;
        rx->frame.command = command;
        rx->frame.raw = rx->raw;
        rx->event = DRV_IR_RX_EVENT_FRAME;
    }
}

void drv_ir_rx_init(drv_ir_rx_t *rx)
{
    if (rx == 0) {
        return;
    }

    drv_ir_rx_reset(rx);
}

void drv_ir_rx_reset(drv_ir_rx_t *rx)
{
    if (rx == 0) {
        return;
    }

    rx->state = DRV_IR_RX_STATE_IDLE;
    rx->bit_index = 0u;
    rx->raw = 0u;
    rx->event = DRV_IR_RX_EVENT_NONE;
    rx->frame.address = 0u;
    rx->frame.command = 0u;
    rx->frame.raw = 0u;
}

#if DRV_IR_RX_ENABLE_PULSE
void drv_ir_rx_feed_pulse(drv_ir_rx_t *rx, stc8h_u8 level, stc8h_u16 width_us)
{
    if (rx == 0) {
        return;
    }

    if (rx->state == DRV_IR_RX_STATE_IDLE) {
        if ((level == DRV_IR_LEVEL_MARK) && (drv_ir_rx_mark_9000(width_us) != 0u)) {
            rx->raw = 0u;
            rx->bit_index = 0u;
            rx->state = DRV_IR_RX_STATE_LEAD_SPACE;
        }
        return;
    }

    if (rx->state == DRV_IR_RX_STATE_LEAD_SPACE) {
        if (level != DRV_IR_LEVEL_SPACE) {
            drv_ir_rx_error(rx);
        } else if (drv_ir_rx_space_4500(width_us) != 0u) {
            rx->state = DRV_IR_RX_STATE_BIT_MARK;
        } else if (drv_ir_rx_space_2250(width_us) != 0u) {
            rx->state = DRV_IR_RX_STATE_IDLE;
            drv_ir_rx_set_event(rx, DRV_IR_RX_EVENT_REPEAT);
        } else {
            drv_ir_rx_error(rx);
        }
        return;
    }

    if (rx->state == DRV_IR_RX_STATE_BIT_MARK) {
        if ((level == DRV_IR_LEVEL_MARK) && (drv_ir_rx_mark_562(width_us) != 0u)) {
            rx->state = DRV_IR_RX_STATE_BIT_SPACE;
        } else {
            drv_ir_rx_error(rx);
        }
        return;
    }

    if (rx->state == DRV_IR_RX_STATE_BIT_SPACE) {
        if (level != DRV_IR_LEVEL_SPACE) {
            drv_ir_rx_error(rx);
        } else if (drv_ir_rx_space_562(width_us) != 0u) {
            drv_ir_rx_push_bit(rx, 0u);
        } else if (drv_ir_rx_space_1687(width_us) != 0u) {
            drv_ir_rx_push_bit(rx, 1u);
        } else {
            drv_ir_rx_error(rx);
            return;
        }

        rx->state = (rx->bit_index >= 32u) ? DRV_IR_RX_STATE_STOP_MARK : DRV_IR_RX_STATE_BIT_MARK;
        return;
    }

    if ((level == DRV_IR_LEVEL_MARK) && (drv_ir_rx_mark_562(width_us) != 0u)) {
        drv_ir_rx_finish_frame(rx);
    } else {
        drv_ir_rx_error(rx);
    }
}
#endif

#if DRV_IR_RX_ENABLE_FALLING
void drv_ir_rx_feed_nec_falling_interval(drv_ir_rx_t *rx, stc8h_u16 width_us)
{
    if (rx == 0) {
        return;
    }

    if (rx->state == DRV_IR_RX_STATE_IDLE) {
        if (drv_ir_rx_falling_lead(width_us) != 0u) {
            rx->raw = 0u;
            rx->bit_index = 0u;
            rx->state = DRV_IR_RX_STATE_FALLING_BITS;
        } else if (drv_ir_rx_falling_repeat(width_us) != 0u) {
            rx->raw = 0u;
            rx->bit_index = 0u;
            rx->state = DRV_IR_RX_STATE_FALLING_REPEAT;
        }
        return;
    }

    if (rx->state == DRV_IR_RX_STATE_FALLING_REPEAT) {
        if (drv_ir_rx_falling_bit(rx, width_us) != 0u) {
            if (rx->state != DRV_IR_RX_STATE_IDLE) {
                rx->state = DRV_IR_RX_STATE_FALLING_BITS;
            }
        } else {
            rx->state = DRV_IR_RX_STATE_IDLE;
            drv_ir_rx_set_event(rx, DRV_IR_RX_EVENT_REPEAT);
        }
        return;
    }

    if (rx->state != DRV_IR_RX_STATE_FALLING_BITS) {
        drv_ir_rx_error(rx);
        return;
    }

    if (drv_ir_rx_falling_bit(rx, width_us) == 0u) {
        drv_ir_rx_error(rx);
    }
}
#endif

#if DRV_IR_RX_ENABLE_FALLING
void drv_ir_rx_finish_nec_falling_interval(drv_ir_rx_t *rx)
{
    if (rx == 0) {
        return;
    }

    if (rx->state == DRV_IR_RX_STATE_FALLING_REPEAT) {
        rx->state = DRV_IR_RX_STATE_IDLE;
        drv_ir_rx_set_event(rx, DRV_IR_RX_EVENT_REPEAT);
    } else if (rx->state == DRV_IR_RX_STATE_FALLING_BITS) {
        rx->state = DRV_IR_RX_STATE_IDLE;
        rx->bit_index = 0u;
        rx->raw = 0u;
    }
}
#endif

drv_ir_rx_event_t drv_ir_rx_get_event(drv_ir_rx_t *rx, drv_ir_rx_frame_t *frame)
{
    drv_ir_rx_event_t event;

    if (rx == 0) {
        return DRV_IR_RX_EVENT_NONE;
    }

    event = rx->event;
    if ((event == DRV_IR_RX_EVENT_FRAME) && (frame != 0)) {
        *frame = rx->frame;
    }
    rx->event = DRV_IR_RX_EVENT_NONE;

    return event;
}
