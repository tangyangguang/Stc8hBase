#ifndef DRV_IR_TX_H
#define DRV_IR_TX_H

#include "stc8h_config.h"

typedef enum {
    DRV_IR_TX_DONE = 0,
    DRV_IR_TX_MARK,
    DRV_IR_TX_SPACE
} drv_ir_tx_level_t;

typedef struct {
    stc8h_u32 raw;
    stc8h_u8 step;
} drv_ir_tx_nec_t;

void drv_ir_tx_nec_begin(drv_ir_tx_nec_t *tx, stc8h_u8 address, stc8h_u8 command);
drv_ir_tx_level_t drv_ir_tx_nec_next(drv_ir_tx_nec_t *tx, stc8h_u16 *duration_us);

#endif
