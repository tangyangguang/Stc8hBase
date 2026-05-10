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

#define BOARD_I2C_SDA_MASK 0x04u
#define BOARD_I2C_SCL_MASK 0x80u
#define BOARD_EC11_A_MASK 0x01u
#define BOARD_EC11_B_MASK 0x02u
#define BOARD_EC11_SW_MASK 0x10u

#define BOARD_I2C_SDA_HIGH() do { P3 |= BOARD_I2C_SDA_MASK; } while (0)
#define BOARD_I2C_SDA_LOW() do { P3 &= (stc8h_u8)~BOARD_I2C_SDA_MASK; } while (0)
#define BOARD_I2C_SCL_HIGH() do { P1 |= BOARD_I2C_SCL_MASK; } while (0)
#define BOARD_I2C_SCL_LOW() do { P1 &= (stc8h_u8)~BOARD_I2C_SCL_MASK; } while (0)
#define BOARD_I2C_SDA_READ() ((P3 & BOARD_I2C_SDA_MASK) ? 1u : 0u)
#define BOARD_I2C_SCL_READ() ((P1 & BOARD_I2C_SCL_MASK) ? 1u : 0u)
#define BOARD_EC11_A_READ() ((P1 & BOARD_EC11_A_MASK) ? 1u : 0u)
#define BOARD_EC11_B_READ() ((P1 & BOARD_EC11_B_MASK) ? 1u : 0u)
#define BOARD_EC11_SW_READ() ((P5 & BOARD_EC11_SW_MASK) ? 1u : 0u)

#endif
