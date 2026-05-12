#include "board_init.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_uart.h"

static void uart_put_bit(stc8h_u8 value)
{
    stc8h_uart_putc(STC8H_UART1, value ? '1' : '0');
}

static void uart_put_hex4(stc8h_u8 value)
{
    value &= 0x0Fu;
    if (value < 10u) {
        stc8h_uart_putc(STC8H_UART1, (char)('0' + value));
    } else {
        stc8h_uart_putc(STC8H_UART1, (char)('A' + (value - 10u)));
    }
}

static void uart_put_hex8(stc8h_u8 value)
{
    stc8h_uart_write_code(STC8H_UART1, "0x");
    uart_put_hex4((stc8h_u8)(value >> 4));
    uart_put_hex4(value);
}

static void print_reg(const STC8H_CODE char *name, stc8h_u8 value)
{
    stc8h_uart_write_code(STC8H_UART1, name);
    stc8h_uart_putc(STC8H_UART1, '=');
    uart_put_hex8(value);
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

static void print_lines(const STC8H_CODE char *label)
{
    stc8h_uart_write_code(STC8H_UART1, label);
    stc8h_uart_write_code(STC8H_UART1, " SDA=");
    uart_put_bit(BOARD_I2C_SDA_READ());
    stc8h_uart_write_code(STC8H_UART1, " SCL=");
    uart_put_bit(BOARD_I2C_SCL_READ());
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

static void print_quasi_sda(void)
{
    stc8h_gpio_set_mode(BOARD_I2C_SDA_PORT, BOARD_I2C_SDA_PIN, STC8H_GPIO_MODE_QUASI);
    BOARD_I2C_SDA_HIGH();
    stc8h_delay_ms(10u);
    print_lines("sda_quasi");
    stc8h_gpio_set_mode(BOARD_I2C_SDA_PORT, BOARD_I2C_SDA_PIN, STC8H_GPIO_MODE_OPEN_DRAIN);
    BOARD_I2C_SDA_HIGH();
}

void main(void)
{
    board_i2c_init();
    (void)stc8h_gpio_set_input_enable_mask(BOARD_I2C_SCL_PORT, BOARD_I2C_SCL_MASK, 1u);
    (void)stc8h_gpio_set_input_enable_mask(BOARD_I2C_SDA_PORT, BOARD_I2C_SDA_MASK, 1u);
    (void)stc8h_gpio_set_pullup_mask(BOARD_I2C_SCL_PORT, BOARD_I2C_SCL_MASK, 1u);
    (void)stc8h_gpio_set_pullup_mask(BOARD_I2C_SDA_PORT, BOARD_I2C_SDA_MASK, 1u);
    (void)stc8h_uart_init(STC8H_UART1);

    stc8h_uart_write_code(STC8H_UART1, "I2C lines test\r\n");
    print_reg("P_SW2", P_SW2);
    print_reg("P1PU", P1PU);
    print_reg("P3PU", P3PU);
    print_reg("P1IE", P1IE);
    print_reg("P3IE", P3IE);
    print_reg("P1M0", P1M0);
    print_reg("P1M1", P1M1);
    print_reg("P3M0", P3M0);
    print_reg("P3M1", P3M1);
    print_reg("P1", P1);
    print_reg("P3", P3);

    while (1) {
        BOARD_I2C_SDA_HIGH();
        BOARD_I2C_SCL_HIGH();
        stc8h_delay_ms(10u);
        print_lines("release");
        print_quasi_sda();

        BOARD_I2C_SDA_LOW();
        BOARD_I2C_SCL_HIGH();
        stc8h_delay_ms(10u);
        print_lines("sda_low");

        BOARD_I2C_SDA_HIGH();
        BOARD_I2C_SCL_LOW();
        stc8h_delay_ms(10u);
        print_lines("scl_low");

        BOARD_I2C_SDA_LOW();
        BOARD_I2C_SCL_LOW();
        stc8h_delay_ms(10u);
        print_lines("both_low");

        BOARD_I2C_SDA_HIGH();
        BOARD_I2C_SCL_HIGH();
        stc8h_delay_ms(1000u);
    }
}
