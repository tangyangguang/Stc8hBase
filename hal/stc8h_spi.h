#ifndef STC8H_SPI_H
#define STC8H_SPI_H

#include "stc8h_config.h"

#ifndef STC8H_SPI_ENABLE_WRITE
#define STC8H_SPI_ENABLE_WRITE 1
#endif

void stc8h_spi_init(void);
stc8h_u8 stc8h_spi_transfer(stc8h_u8 value);
#if STC8H_SPI_ENABLE_WRITE
void stc8h_spi_write(const STC8H_DATA stc8h_u8 *data, stc8h_u8 len);
#endif

#endif
