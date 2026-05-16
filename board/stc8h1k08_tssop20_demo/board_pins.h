#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include "stc8h_sfr.h"

#define BOARD_LED_PORT 1u
#define BOARD_LED_PIN 2u

#define BOARD_I2C_SDA_PORT 3u
#define BOARD_I2C_SDA_PIN 2u
#define BOARD_I2C_SCL_PORT 1u
#define BOARD_I2C_SCL_PIN 7u

#define BOARD_EC11_A_PORT 1u
#define BOARD_EC11_A_PIN 0u
#define BOARD_EC11_B_PORT 1u
#define BOARD_EC11_B_PIN 1u
#define BOARD_EC11_SW_PORT 5u
#define BOARD_EC11_SW_PIN 4u

#define BOARD_POT_ADC_PORT 3u
#define BOARD_POT_ADC_PIN 3u

/* Current hardware-test wiring, not a recommended default pinout. */
#define BOARD_TM1637_CLK_PORT 1u
#define BOARD_TM1637_CLK_PIN 7u
#define BOARD_TM1637_DIO_PORT 3u
#define BOARD_TM1637_DIO_PIN 2u

#define BOARD_I2C_SDA_MASK 0x04u
#define BOARD_I2C_SCL_MASK 0x80u
#define BOARD_EC11_A_MASK 0x01u
#define BOARD_EC11_B_MASK 0x02u
#define BOARD_EC11_SW_MASK 0x10u
#define BOARD_POT_ADC_MASK 0x08u
#define BOARD_TM1637_CLK_MASK 0x80u
#define BOARD_TM1637_DIO_MASK 0x04u

#define BOARD_I2C_SDA_HIGH() do { P3 |= BOARD_I2C_SDA_MASK; } while (0)
#define BOARD_I2C_SDA_LOW() do { P3 &= (stc8h_u8)~BOARD_I2C_SDA_MASK; } while (0)
#define BOARD_I2C_SCL_HIGH() do { P1 |= BOARD_I2C_SCL_MASK; } while (0)
#define BOARD_I2C_SCL_LOW() do { P1 &= (stc8h_u8)~BOARD_I2C_SCL_MASK; } while (0)
#define BOARD_I2C_SDA_READ() ((P3 & BOARD_I2C_SDA_MASK) ? 1u : 0u)
#define BOARD_I2C_SCL_READ() ((P1 & BOARD_I2C_SCL_MASK) ? 1u : 0u)
#define BOARD_EC11_A_READ() ((P1 & BOARD_EC11_A_MASK) ? 1u : 0u)
#define BOARD_EC11_B_READ() ((P1 & BOARD_EC11_B_MASK) ? 1u : 0u)
#define BOARD_EC11_SW_READ() ((P5 & BOARD_EC11_SW_MASK) ? 1u : 0u)

#define BOARD_TM1637_CLK_HIGH() do { P1 |= BOARD_TM1637_CLK_MASK; } while (0)
#define BOARD_TM1637_CLK_LOW() do { P1 &= (stc8h_u8)~BOARD_TM1637_CLK_MASK; } while (0)
#define BOARD_TM1637_DIO_HIGH() do { P3 |= BOARD_TM1637_DIO_MASK; } while (0)
#define BOARD_TM1637_DIO_LOW() do { P3 &= (stc8h_u8)~BOARD_TM1637_DIO_MASK; } while (0)
#define BOARD_TM1637_DIO_READ() ((P3 & BOARD_TM1637_DIO_MASK) ? 1u : 0u)

/* Current nRF24L01 hardware-test wiring. SPI uses P1.3/P1.4/P1.5. */
#define BOARD_NRF24L01_CSN_MASK 0x04u
#define BOARD_NRF24L01_CE_MASK 0x40u
#define BOARD_NRF24L01_IRQ_MASK 0x04u

#define DRV_NRF24L01_CSN_HIGH() do { P1 |= BOARD_NRF24L01_CSN_MASK; } while (0)
#define DRV_NRF24L01_CSN_LOW() do { P1 &= (stc8h_u8)~BOARD_NRF24L01_CSN_MASK; } while (0)
#define DRV_NRF24L01_CE_HIGH() do { P1 |= BOARD_NRF24L01_CE_MASK; } while (0)
#define DRV_NRF24L01_CE_LOW() do { P1 &= (stc8h_u8)~BOARD_NRF24L01_CE_MASK; } while (0)
#define DRV_NRF24L01_IRQ_READ() ((P3 & BOARD_NRF24L01_IRQ_MASK) ? 1u : 0u)

#endif
