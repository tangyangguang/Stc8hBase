#include "stc8h_spi.h"
#include "stc8h_sfr.h"

#ifndef STC8H_SPI_P_SW1_MASK
#define STC8H_SPI_P_SW1_MASK 0x0Cu
#endif

#ifndef STC8H_SPI_PIN_GROUP
#define STC8H_SPI_PIN_GROUP 0u
#endif

#if STC8H_SPI_PIN_GROUP > 3
#error "STC8H_SPI_PIN_GROUP must be 0, 1, 2, or 3"
#endif

#ifndef STC8H_SPI_SPCTL
#define STC8H_SPI_SPCTL 0xD0u
#endif

#ifndef STC8H_SPI_CONFIGURE_PORT_MODE
#define STC8H_SPI_CONFIGURE_PORT_MODE 1
#endif

#ifndef STC8H_SPI_ENABLE_MISO_INPUT
#define STC8H_SPI_ENABLE_MISO_INPUT 1
#endif

#ifndef STC8H_SPI_LATCH_MISO_HIGH
#define STC8H_SPI_LATCH_MISO_HIGH 1
#endif

#ifndef STC8H_SPI_EAXFR
#define STC8H_SPI_EAXFR 0x80u
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

#if STC8H_SPI_CONFIGURE_PORT_MODE
#if STC8H_SPI_PIN_GROUP == 0
    P1M0 &= (stc8h_u8)~0x38u;
    P1M1 &= (stc8h_u8)~0x38u;
#elif STC8H_SPI_PIN_GROUP == 1
    P2M0 &= (stc8h_u8)~0x38u;
    P2M1 &= (stc8h_u8)~0x38u;
#elif STC8H_SPI_PIN_GROUP == 2
    P4M0 &= (stc8h_u8)~0x0Bu;
    P4M1 &= (stc8h_u8)~0x0Bu;
#else
    P3M0 &= (stc8h_u8)~0x1Cu;
    P3M1 &= (stc8h_u8)~0x1Cu;
#endif
#endif

#if STC8H_SPI_LATCH_MISO_HIGH
#if STC8H_SPI_PIN_GROUP == 0
    P1 |= 0x10u;
#elif STC8H_SPI_PIN_GROUP == 1
    P2 |= 0x10u;
#elif STC8H_SPI_PIN_GROUP == 2
    P4 |= 0x02u;
#else
    P3 |= 0x08u;
#endif
#endif

#if STC8H_SPI_ENABLE_MISO_INPUT
    P_SW2 |= STC8H_SPI_EAXFR;
#if STC8H_SPI_PIN_GROUP == 0
    P1IE |= 0x10u;
#elif STC8H_SPI_PIN_GROUP == 1
    P2IE |= 0x10u;
#elif STC8H_SPI_PIN_GROUP == 2
    P4IE |= 0x02u;
#else
    P3IE |= 0x08u;
#endif
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
