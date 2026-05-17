#include "stc8h_spi.h"
#include "stc8h_sfr.h"

#ifndef STC8H_SPI_P_SW1_MASK
#define STC8H_SPI_P_SW1_MASK 0x0Cu
#endif

#ifndef STC8H_SPI_PIN_GROUP
#define STC8H_SPI_PIN_GROUP 0u
#endif

#ifndef STC8H_SPI_SPCTL
#define STC8H_SPI_SPCTL 0xD0u
#endif

#ifndef STC8H_SPI_FLAG_MASK
#define STC8H_SPI_FLAG_MASK 0xC0u
#endif

#ifndef STC8H_SPI_SPIF
#define STC8H_SPI_SPIF 0x80u
#endif

void stc8h_spi_init(void)
{
    P_SW1 = (stc8h_u8)((P_SW1 & (stc8h_u8)~STC8H_SPI_P_SW1_MASK) | (stc8h_u8)(STC8H_SPI_PIN_GROUP << 2));
#if STC8H_SPI_PIN_GROUP == 0
    P1M0 = (stc8h_u8)((P1M0 | 0x28u) & (stc8h_u8)~0x10u);
    P1M1 = (stc8h_u8)((P1M1 | 0x10u) & (stc8h_u8)~0x28u);
#endif
    SPCTL = STC8H_SPI_SPCTL;
    SPSTAT = STC8H_SPI_FLAG_MASK;
}

stc8h_u8 stc8h_spi_transfer(stc8h_u8 value)
{
    SPSTAT = STC8H_SPI_FLAG_MASK;
    SPDAT = value;
    while ((SPSTAT & STC8H_SPI_SPIF) == 0u) {
    }
    value = SPDAT;
    SPSTAT = STC8H_SPI_FLAG_MASK;
    return value;
}

#if STC8H_SPI_ENABLE_WRITE
void stc8h_spi_write(const STC8H_DATA stc8h_u8 *data, stc8h_u8 len)
{
    while (len != 0u) {
        (void)stc8h_spi_transfer(*data);
        ++data;
        --len;
    }
}
#endif
