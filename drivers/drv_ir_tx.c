#include "drv_ir_tx.h"

#define DRV_IR_TX_TOTAL_STEPS 67u
#define DRV_IR_TX_REPEAT_FLAG 0x80u
#define DRV_IR_TX_REPEAT_STEPS 3u

void drv_ir_tx_nec_begin(drv_ir_tx_nec_t *tx, stc8h_u8 address, stc8h_u8 command)
{
    if (tx == 0) {
        return;
    }

    tx->raw = (stc8h_u32)address |
        ((stc8h_u32)(stc8h_u8)~address << 8) |
        ((stc8h_u32)command << 16) |
        ((stc8h_u32)(stc8h_u8)~command << 24);
    tx->step = 0u;
}

void drv_ir_tx_nec_repeat_begin(drv_ir_tx_nec_t *tx)
{
    if (tx == 0) {
        return;
    }

    tx->raw = 0u;
    tx->step = DRV_IR_TX_REPEAT_FLAG;
}

drv_ir_tx_level_t drv_ir_tx_nec_next(drv_ir_tx_nec_t *tx, stc8h_u16 *duration_us)
{
    stc8h_u8 bit_index;
    stc8h_u8 step;

    if ((tx == 0) || (duration_us == 0)) {
        return DRV_IR_TX_DONE;
    }

    if ((tx->step & DRV_IR_TX_REPEAT_FLAG) != 0u) {
        step = (stc8h_u8)(tx->step & (stc8h_u8)~DRV_IR_TX_REPEAT_FLAG);
        if (step == 0u) {
            *duration_us = 9000u;
            tx->step = (stc8h_u8)(DRV_IR_TX_REPEAT_FLAG | 1u);
            return DRV_IR_TX_MARK;
        }
        if (step == 1u) {
            *duration_us = 2250u;
            tx->step = (stc8h_u8)(DRV_IR_TX_REPEAT_FLAG | 2u);
            return DRV_IR_TX_SPACE;
        }
        if (step < DRV_IR_TX_REPEAT_STEPS) {
            *duration_us = 562u;
            tx->step = (stc8h_u8)(DRV_IR_TX_REPEAT_FLAG | DRV_IR_TX_REPEAT_STEPS);
            return DRV_IR_TX_MARK;
        }
        return DRV_IR_TX_DONE;
    }

    if (tx->step == 0u) {
        *duration_us = 9000u;
        ++tx->step;
        return DRV_IR_TX_MARK;
    }
    if (tx->step == 1u) {
        *duration_us = 4500u;
        ++tx->step;
        return DRV_IR_TX_SPACE;
    }
    if (tx->step >= DRV_IR_TX_TOTAL_STEPS) {
        return DRV_IR_TX_DONE;
    }
    if (tx->step == (DRV_IR_TX_TOTAL_STEPS - 1u)) {
        *duration_us = 562u;
        ++tx->step;
        return DRV_IR_TX_MARK;
    }

    bit_index = (stc8h_u8)((tx->step - 2u) >> 1);
    if ((tx->step & 1u) == 0u) {
        *duration_us = 562u;
        ++tx->step;
        return DRV_IR_TX_MARK;
    }

    *duration_us = ((tx->raw & ((stc8h_u32)1u << bit_index)) != 0u) ? 1687u : 562u;
    ++tx->step;
    return DRV_IR_TX_SPACE;
}
