# STC 官方库函数参考审视

## 1. 来源

官方页面：

```text
https://www.stcmicro.com/slcx.html
```

本地归档：

```text
docs/vendor/stc/stc8g-stc8h-lib-demo-code.rar
```

SHA-256：

```text
95a759ea0cd5ec9e2e4d8183256da4ad7c7e863687c0bb1cfc0d36cfe926b703
```

解压分析目录只放在临时目录，不提交展开源码。

## 2. 使用原则

- 官方库用于核对寄存器、初始化顺序、触发保护序列和示例行为。
- 不直接复制官方库的整体架构。
- 不引入官方库的大型结构体配置、全局缓冲区和运行期多外设分派。
- 对本库已实现模块，如果官方库和 STC 手册冲突，优先以手册为硬件事实来源，再用实测确认。

## 3. 已核对模块

### GPIO

官方库 `STC8G_H_GPIO.c` 的模式映射：

| 模式 | M1 | M0 |
| --- | --- | --- |
| 准双向 | 0 | 0 |
| 高阻输入 | 1 | 0 |
| 开漏 | 1 | 1 |
| 推挽 | 0 | 1 |

本库 `stc8h_gpio_set_mode` 与该映射一致。

### UART

官方库 UART1 在可变波特率模式下使用：

```text
reload = 65536 - MAIN_Fosc / 4 / baud
```

并明确 Timer1/Timer2 可作为 UART1 波特率源。本库当前只实现 UART1 + Timer1，按 11.0592MHz 常用波特率做编译期重装值，避免默认路径拉入 32 位除法库。

### ADC

官方库 `Get_ADCResult(channel)` 做法：

- 选择通道后启动转换。
- 轮询 `ADC_FLAG`。
- 根据对齐方式合成结果。
- 超时返回错误值。

本库当前做法：

- STC8H1K08 按 10-bit ADC 实现。
- 默认右对齐，返回 `0..1023`。
- `ADCTIM=0x3f`，ADC 时钟 `SYSclk/2/16`。
- 无效通道或超时返回 `STC8H_ADC_INVALID_VALUE`。

### Timer

官方 Timer0 例程要点：

- Timer0 可选 12T、1T 或外部脉冲输入。
- `AUXR` bit7 选择 Timer0 1T/12T。
- `INTCLKO` bit0 控制 Timer0 时钟输出。
- `TR0` 控制 Timer0 启停。
- Timer0 中断向量为 1。
- 11.0592MHz、12T、1ms tick 的重装值为 `65536 - 11059200 / 12 / 1000 = 0xFC66`。

本库当前做法：

- 只实现 Timer0 1ms tick 的最小 HAL。
- 默认使用 12T，避免额外时钟配置。
- ISR 由示例或板级文件显式绑定，Timer HAL 不默认占用中断向量。
- UART1 使用 Timer1，`timer_tick` 示例同时验证 Timer0 与 Timer1 可并行使用。

### SPI

官方 SPI 库要点：

- SPI 状态寄存器为 `SPSTAT=0xCD`，控制寄存器为 `SPCTL=0xCE`，数据寄存器为 `SPDAT=0xCF`。
- `SPCTL` bit7 为 `SSIG`，bit6 为 SPI 使能，bit5 为 MSB/LSB 顺序，bit4 为主/从模式，bit3/bit2 为 CPOL/CPHA，bit1..0 为速度选择。
- `SPSTAT` bit7 为 `SPIF`，bit6 为 `WCOL`，写 1 清除。
- SPI 引脚组通过 `P_SW1[3:2]` 选择，官方库提供 P1、P2、P5/P4、P3 多组引脚。

本库当前做法：

- 第一版冻结为硬件 SPI 主机轮询实现。
- 默认 P1.3/P1.4/P1.5，`SSIG=1` 忽略硬件 SS，避免占用当前 P1.2 LED。
- 不启用 SPI 中断，不保存 RX 缓冲，不做主从自动切换。
- 片选由板级或应用代码自行控制，避免在 HAL 中保存运行期 CS 引脚。

### 软件 I2C

官方软件 I2C：

- Start/Stop/ACK/NAK 均用明确 GPIO 时序。
- ACK 通过释放 SDA 后读电平判断。
- I2C 延时随主频计算。

本库继续使用板级宏绑定 SDA/SCL，避免运行期 port/pin 分派。已通过 `i2c_lines` 验证释放状态能读高，避免全地址假 ACK。

### EEPROM/IAP

官方 EEPROM/IAP：

- 操作前启用 IAP。
- 触发前保存并关闭全局中断。
- 依次写 `IAP_TRIG=0x5A`、`IAP_TRIG=0xA5`。
- 操作后关闭 IAP 并清理寄存器。

后续实现 `stc8h_eeprom` 必须遵守这个触发保护方式，并继续要求示例显式定义测试地址范围。

## 4. 后续模块参考清单

| 后续模块 | 优先参考官方文件 |
| --- | --- |
| Timer | `STC8G_H_Timer.c`、`STC8G_H_Timer_Isr.c` |
| SPI | `STC8G_H_SPI.c`、`STC8G_H_SPI_Isr.c` |
| EEPROM/IAP | `STC8G_H_EEPROM.c` |
| PWM | `STC8H_PWM.c`、`STC8H_PWM_Isr.c` |
| 硬件 I2C | `STC8G_H_I2C.c`、`STC8G_H_I2C_Isr.c` |
| 软件 I2C | `STC8G_H_Soft_I2C.c` |
| ADC | `STC8G_H_ADC.c`、`STC8G_H_ADC_Isr.c` |
| DMA | `STC8H_DMA.c`、`STC8H_DMA_Isr.c` |

## 5. 当前结论

当前本库的 GPIO、UART1、软件 I2C、ADC、Timer0、SPI 实现方向与官方库和官方手册核对后没有发现需要推倒重来的问题。后续模块应在开工前先查本记录列出的官方文件，再结合 STC8H1K08 TSSOP20 的实际资源裁剪实现。
