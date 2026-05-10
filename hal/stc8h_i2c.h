#ifndef STC8H_I2C_H
#define STC8H_I2C_H

#include "stc8h_config.h"

void stc8h_i2c_init(void);
stc8h_status_t stc8h_i2c_write(stc8h_u8 addr7, const stc8h_u8 *data, stc8h_u8 len);
stc8h_status_t stc8h_i2c_read(stc8h_u8 addr7, stc8h_u8 *data, stc8h_u8 len);
stc8h_u8 stc8h_i2c_probe(stc8h_u8 addr7);

#endif
