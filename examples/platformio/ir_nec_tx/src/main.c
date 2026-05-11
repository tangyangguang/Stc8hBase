#include "drv_ir_tx.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_pwm.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#define IR_TX_PWM_CHANNEL STC8H_PWM_CHANNEL_1
#define IR_TX_PWM_PERIOD  290u
#define IR_TX_PWM_DUTY    97u
#define IR_TX_ADDRESS     0x00u
#define IR_TX_COMMAND     0x45u

static void ir_tx_carrier_on(void)
{
    (void)stc8h_pwm_set_duty(IR_TX_PWM_CHANNEL, IR_TX_PWM_DUTY);
    (void)stc8h_pwm_enable(IR_TX_PWM_CHANNEL);
}

static void ir_tx_carrier_off(void)
{
    (void)stc8h_pwm_disable(IR_TX_PWM_CHANNEL);
    P1 &= (stc8h_u8)~0x01u;
}

static stc8h_u8 ir_tx_send_nec(stc8h_u8 address, stc8h_u8 command)
{
    drv_ir_tx_nec_t tx;
    drv_ir_tx_level_t level;
    stc8h_u16 duration_us;

    drv_ir_tx_nec_begin(&tx, address, command);
    while (1) {
        level = drv_ir_tx_nec_next(&tx, &duration_us);
        if (level == DRV_IR_TX_DONE) {
            ir_tx_carrier_off();
            return 1u;
        }
        if (level == DRV_IR_TX_MARK) {
            ir_tx_carrier_on();
        } else {
            ir_tx_carrier_off();
        }
        stc8h_delay_timer0_1t_us(duration_us);
    }
}

static void ir_tx_board_init(void)
{
    stc8h_gpio_set_mode(1u, 0u, STC8H_GPIO_MODE_PUSH_PULL);
    P1 &= (stc8h_u8)~0x01u;
    (void)stc8h_delay_timer0_1t_init();
    (void)stc8h_pwm_init(IR_TX_PWM_CHANNEL, IR_TX_PWM_PERIOD);
    (void)stc8h_pwm_set_duty(IR_TX_PWM_CHANNEL, 0u);
    ir_tx_carrier_off();
}

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    ir_tx_board_init();
    stc8h_uart_write_code(STC8H_UART1, "ir nec tx P10\r\n");

    while (1) {
        if (ir_tx_send_nec(IR_TX_ADDRESS, IR_TX_COMMAND) != 0u) {
            stc8h_uart_write_code(STC8H_UART1, "ir tx ok\r\n");
        } else {
            stc8h_uart_write_code(STC8H_UART1, "ir tx error\r\n");
        }
        stc8h_delay_ms(1000u);
    }
}
