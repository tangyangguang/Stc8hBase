#include "stc8h_gpio.h"
#include "stc8h_qei.h"

#if !STC8H_CHIP_STC8H8K64U
#error "h8k64u_qei_pwmb_validate requires STC8H8K64U."
#endif

static volatile stc8h_u16 qei_sample;

void main(void)
{
    stc8h_gpio_set_mode(2u, 0u, STC8H_GPIO_MODE_INPUT_ONLY);
    stc8h_gpio_set_mode(2u, 1u, STC8H_GPIO_MODE_INPUT_ONLY);
    stc8h_qei_pwmb_init();

    while (1) {
        qei_sample = stc8h_qei_pwmb_read();
    }
}
