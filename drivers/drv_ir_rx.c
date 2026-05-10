#include "drv_ir_rx.h"

#define DRV_IR_RX_STATE_IDLE       0u
#define DRV_IR_RX_STATE_LEAD_SPACE 1u
#define DRV_IR_RX_STATE_BIT_MARK   2u
#define DRV_IR_RX_STATE_BIT_SPACE  3u
#define DRV_IR_RX_STATE_STOP_MARK  4u

#define DRV_IR_LEVEL_MARK  0u
#define DRV_IR_LEVEL_SPACE 1u

static stc8h_u8 drv_ir_rx_between(stc8h_u16 value, stc8h_u16 low, stc8h_u16 high)
{
    return ((value >= low) && (value <= high)) ? STC8H_TRUE : STC8H_FALSE;
}

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

static void drv_ir_rx_error(drv_ir_rx_t *rx)
{
    rx->state = DRV_IR_RX_STATE_IDLE;
    rx->bit_index = 0u;
    rx->raw = 0u;
    rx->event = DRV_IR_RX_EVENT_ERROR;
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
        rx->event = DRV_IR_RX_EVENT_ERROR;
        return;
    }

    rx->frame.address = address;
    rx->frame.command = command;
    rx->frame.raw = rx->raw;
    rx->event = DRV_IR_RX_EVENT_FRAME;
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
            rx->event = DRV_IR_RX_EVENT_REPEAT;
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
            ++rx->bit_index;
        } else if (drv_ir_rx_space_1687(width_us) != 0u) {
            rx->raw |= ((stc8h_u32)1u << rx->bit_index);
            ++rx->bit_index;
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
