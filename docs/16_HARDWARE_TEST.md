# 硬件测试清单

本文件记录硬件验证入口和必须注意的风险，不再保存逐次烧录日志。

## 通用原则

- 先构建，再烧录。
- EEPROM/IAP、OTA 参数区、看门狗复位、bootloader 跳转等高影响测试必须先确认地址、芯片型号和测试板可接受风险。
- 硬件现象只记录稳定结论；临时串口日志和调试流水不进入仓库。

## 日常无硬件验证

```sh
tools/check_host_tests.sh
tools/check_examples.sh
```

## 发布前完整验证

```sh
tools/check_examples_full.sh
tools/check_host_tests_full.sh
tools/prepare_h8k64u_validation.sh
```

## STC8H1K08 代表性硬件项

- `gpio_blink`：确认 GPIO 输出和延时。
- `uart_hello` / `uart_echo_buffered`：确认 UART1 波特率和轮询接收。
- `i2c_scan` / `lcd1602_text`：确认软件 I2C、LCD 地址和上拉。
- `spi_loopback`：短接 MOSI/MISO，确认 SPI 基础收发。
- `pwm_output`：确认 PWMA 输出和占空比变化。
- `adc_pot`：确认 ADC 输入范围和参考电压假设。
- `eeprom_rw`：只在确认 EEPROM 测试页可擦写后运行写擦环境。
- `wdt_reset_test`：确认复位标志和受控复位，不作为日常自动上传项。

## nRF24 专项硬件项

- `nrf24_fixed_ping`：基础固定 payload 发送。
- `nrf24_ack_payload`：ACK payload 和 dynamic payload。
- `nrf24_uart_diag`：单模块 SPI、寄存器和 FEATURE/DYNPD 验证。

## STC8H8K64U 专项硬件项

- `h8k64u_gpio_blink`：基础 GPIO。
- `h8k64u_uart2_hello` / `h8k64u_uart3_hello`：UART2/UART3 引脚组。
- `h8k64u_adc_read`：12-bit ADC。
- `h8k64u_eeprom_safe`：安全占位构建。
- `h8k64u_eeprom_rw`：破坏性 EEPROM/IAP 写擦读回，必须确认测试页。
- `h8k64u_ota_min_app`：应用链接基址和 mark-valid 路径。
- `h8k64u_uart1_ota_bootloader` / `h8k64u_rs485_ota_bootloader`：bootloader 链接布局、参数区边界、串口 OTA frame。

## 记录要求

硬件验证完成后，只补充：

- 日期。
- 板卡和芯片型号。
- 固件示例/环境名。
- 通过或失败结论。
- 对后续设计有影响的事实。

不记录完整串口输出、重复命令、临时失败步骤或与最终设计无关的过程。
