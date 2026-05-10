#ifndef STC8H_SPI_H
#define STC8H_SPI_H

#include "stc8h_config.h"

void stc8h_spi_init(void);
stc8h_u8 stc8h_spi_transfer(stc8h_u8 value);
void stc8h_spi_write(const STC8H_DATA stc8h_u8 *data, stc8h_u8 len);

#endif
