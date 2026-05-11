#ifndef DRV_IR_RX_H
#define DRV_IR_RX_H

#include "stc8h_config.h"

typedef enum {
    DRV_IR_RX_EVENT_NONE = 0,
    DRV_IR_RX_EVENT_FRAME,
    DRV_IR_RX_EVENT_REPEAT,
    DRV_IR_RX_EVENT_ERROR
} drv_ir_rx_event_t;

typedef struct {
    stc8h_u8 address;
    stc8h_u8 command;
    stc8h_u32 raw;
} drv_ir_rx_frame_t;

typedef struct {
    stc8h_u8 state;
    stc8h_u8 bit_index;
    stc8h_u32 raw;
    drv_ir_rx_event_t event;
    drv_ir_rx_frame_t frame;
} drv_ir_rx_t;

void drv_ir_rx_init(drv_ir_rx_t *rx);
void drv_ir_rx_reset(drv_ir_rx_t *rx);
void drv_ir_rx_feed_pulse(drv_ir_rx_t *rx, stc8h_u8 level, stc8h_u16 width_us);
void drv_ir_rx_feed_nec_falling_interval(drv_ir_rx_t *rx, stc8h_u16 width_us);
void drv_ir_rx_finish_nec_falling_interval(drv_ir_rx_t *rx);
drv_ir_rx_event_t drv_ir_rx_get_event(drv_ir_rx_t *rx, drv_ir_rx_frame_t *frame);

#endif
