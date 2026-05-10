# 资源报告

## 1. `gpio_blink` 示例

路径：

```text
examples/make/gpio_blink
```

工具链：

```text
SDCC mcs51
```

系统时钟：

```text
11.0592MHz
```

编译命令：

```text
make clean && make
```

编译文件：

```text
main.c
core/stc8h_delay.c
hal/stc8h_gpio.c
```

功能：

- GPIO LED 快闪/慢闪组合测试。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 997 bytes |
| Stack start | 0x11 |
| Internal RAM 边界 | 栈从 0x11 开始，当前静态/参数/overlay 占用到 0x10 |
| XDATA/PDATA | 未分配应用数据 |
| Timer | 未使用 |
| UART | 未使用 |
| I2C | 未使用 |
| PWM | 未使用 |
| ADC | 未使用 |
| SPI | 未使用 |
| 外部中断 | 未使用 |

链接文件检查：

```text
build/main.rel
build/stc8h_delay.rel
build/stc8h_gpio.rel
```

未使用模块检查：

- 未编译 `stc8h_interrupt`、UART、I2C、LCD1602、SPI、PWM、ADC、EEPROM、button、EC11、TM1637、IR、ring buffer、soft timer、CRC、filter 模块。
- 已检查 map/sym 文件，未发现 `stc8h_uart`、`stc8h_i2c`、`drv_lcd1602`、`stc8h_interrupt`、`drv_button`、`drv_ec11`、`drv_ir`、`drv_tm1637`、`stc8h_spi`、`stc8h_adc`、`stc8h_eeprom`、`util_*` 等未使用模块符号前缀。

## 2. PlatformIO `gpio_blink` 示例

路径：

```text
examples/platformio/gpio_blink
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

系统时钟：

```text
11.0592MHz
```

编译命令：

```text
pio run
```

编译文件：

```text
src/main.c
src/stc8h_delay_wrap.c
src/stc8h_gpio_wrap.c
```

说明：

- PlatformIO 的 MCS-51 构建链路要求源文件生成 `.rel`，示例用极薄的 wrapper 源文件包含库源码，避免 `extra_scripts` 生成 `.o` 后被链接器忽略。
- 这是 PlatformIO 示例工程组织方式，不改变基础库源码结构。

功能：

- GPIO LED 快闪/慢闪组合测试。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 927 bytes |
| Stack start | 0x11 |
| Internal RAM 边界 | 栈从 0x11 开始，当前静态/参数/overlay 占用到 0x10 |
| XDATA/PDATA | 未分配应用数据 |
| Timer | 未使用 |
| UART | 未使用 |
| I2C | 未使用 |
| PWM | 未使用 |
| ADC | 未使用 |
| SPI | 未使用 |
| 外部中断 | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_delay_wrap.rel
.pio/build/STC8H1K08/src/stc8h_gpio_wrap.rel
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 UART、I2C、LCD1602、`stc8h_interrupt` 或 `__divulong` 等无关符号。

## 3. `i2c_scan` 示例

路径：

```text
examples/make/i2c_scan
```

工具链：

```text
SDCC mcs51
```

系统时钟：

```text
11.0592MHz
```

编译命令：

```text
make clean && make
```

编译文件：

```text
main.c
core/stc8h_delay.c
hal/stc8h_gpio.c
hal/stc8h_uart.c
hal/stc8h_i2c.c
board/stc8h1k08_tssop20_demo/board_init.c
```

功能：

- UART1 输出 I2C 地址扫描结果。
- 软件 I2C 扫描 `0x08` 到 `0x77` 的 7-bit 地址。

硬件实测：

- LCD1602 背包芯片为 8574T / PCF8574T 兼容。
- 修正接线后扫描到 `0x27`。
- LED 翻转表示程序仍在运行。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1891 bytes |
| Stack start | 0x1c |
| Internal RAM 边界 | 栈从 0x1c 开始，当前静态/参数/overlay 占用到 0x1b |
| XDATA/PDATA | XFR 扩展寄存器访问，用于 `P1PU/P3PU/P1IE/P3IE` |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| I2C | 软件 I2C，SDA/SCL 由 `board_pins.h` 宏绑定 |
| PWM | 未使用 |
| ADC | 未使用 |
| SPI | 未使用 |
| 外部中断 | 未使用 |

链接文件检查：

```text
build/main.rel
build/stc8h_delay.rel
build/stc8h_gpio.rel
build/stc8h_uart.rel
build/stc8h_i2c.rel
build/board_init.rel
```

未使用模块检查：

- 未编译 LCD1602、`stc8h_interrupt`、SPI、PWM、ADC、EEPROM、button、EC11、TM1637、IR、ring buffer、soft timer、CRC、filter 模块。
- 已检查 map/sym 文件，未发现 `drv_lcd1602`、`stc8h_interrupt`、`drv_button`、`drv_ec11`、`drv_ir`、`drv_tm1637`、`stc8h_spi`、`stc8h_adc`、`stc8h_eeprom`、`util_*`、`__divulong` 等未使用模块或除法库符号前缀。

## 4. PlatformIO `i2c_scan` 示例

路径：

```text
examples/platformio/i2c_scan
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- UART1 输出 I2C 地址扫描结果。
- 软件 I2C 扫描 `0x08` 到 `0x77` 的 7-bit 地址。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1827 bytes |
| Stack start | 0x1c |
| Internal RAM 边界 | 栈从 0x1c 开始，当前静态/参数/overlay 占用到 0x1b |
| XDATA/PDATA | 未分配应用数据 |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| I2C | 软件 I2C，SDA/SCL 由 `board_pins.h` 宏绑定 |
| LCD1602 | 未使用 |

## 5. PlatformIO `uart_hello` 示例

路径：

```text
examples/platformio/uart_hello
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- UART1 每约 1 秒输出固定文本。
- 独立验证 UART1，不混入 I2C/LCD。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 463 bytes |
| Stack start | 0x0e |
| Internal RAM 边界 | 栈从 0x0e 开始，当前静态/参数/overlay 占用到 0x0d |
| XDATA/PDATA | 未分配应用数据 |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| I2C | 未使用 |
| LCD1602 | 未使用 |

## 6. PlatformIO `i2c_lines` 示例

路径：

```text
examples/platformio/i2c_lines
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- UART1 输出 SDA/SCL 释放和拉低状态。
- 独立验证 I2C 引脚、开漏模式、数字输入使能和内部上拉。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1727 bytes |
| Stack start | 0x1c |
| Internal RAM 边界 | 栈从 0x1c 开始，当前静态/参数/overlay 占用到 0x1b |
| XDATA/PDATA | XFR 扩展寄存器访问，用于 `P1PU/P3PU/P1IE/P3IE` |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| I2C | 不调用 I2C 状态机，只直接操作 SDA/SCL 板级宏 |
| LCD1602 | 未使用 |

## 7. PlatformIO `lcd1602_text` 示例

路径：

```text
examples/platformio/lcd1602_text
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- UART1 输出测试提示。
- I2C LCD1602 显示固定文本。
- LED 翻转表示程序仍在运行。

硬件实测：

- 串口输出 `LCD1602 test`。
- LCD 显示 `Stc8hBase` / `LCD1602 OK`。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 2233 bytes |
| Stack start | 0x23 |
| Internal RAM 边界 | 栈从 0x23 开始，当前静态/参数/overlay 占用到 0x22 |
| XDATA/PDATA | XFR 扩展寄存器访问，用于 `P1PU/P3PU/P1IE/P3IE` |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| I2C | 软件 I2C，SDA/SCL 由 `board_pins.h` 宏绑定 |
| LCD1602 | I2C 地址使用 `BOARD_LCD1602_ADDR7` |

## 8. `milestone1_demo` 示例

路径：

```text
examples/make/milestone1_demo
```

工具链：

```text
SDCC mcs51
```

系统时钟：

```text
11.0592MHz
```

编译命令：

```text
make clean && make
```

编译文件：

```text
main.c
core/stc8h_delay.c
hal/stc8h_gpio.c
hal/stc8h_uart.c
hal/stc8h_i2c.c
drivers/drv_lcd1602.c
board/stc8h1k08_tssop20_demo/board_init.c
```

功能：

- GPIO LED 翻转。
- UART1 阻塞发送。
- 软件 I2C。
- I2C LCD1602 文本显示。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 2297 bytes |
| Stack start | 0x23 |
| Internal RAM 边界 | 栈从 0x23 开始，当前静态/参数/overlay 占用到 0x22 |
| XDATA/PDATA | XFR 扩展寄存器访问，用于 `P1PU/P3PU/P1IE/P3IE` |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| I2C | 软件 I2C，SDA/SCL 由 `board_pins.h` 宏绑定 |
| PWM | 未使用 |
| ADC | 未使用 |
| SPI | 未使用 |
| 外部中断 | 未使用 |

链接文件检查：

```text
build/main.rel
build/stc8h_delay.rel
build/stc8h_gpio.rel
build/stc8h_uart.rel
build/stc8h_i2c.rel
build/drv_lcd1602.rel
build/board_init.rel
```

未使用模块检查：

- 未编译 `stc8h_interrupt`、SPI、PWM、ADC、EEPROM、button、EC11、TM1637、IR、ring buffer、soft timer、CRC、filter 模块。
- 已检查 map/sym 文件，未发现 `stc8h_interrupt`、`drv_button`、`drv_ec11`、`drv_ir`、`drv_tm1637`、`stc8h_spi`、`stc8h_adc`、`stc8h_eeprom`、`util_*` 等未使用模块符号前缀。

GPIO 模式寄存器访问检查：

- `P0M0/P0M1` 到 `P5M0/P5M1` 使用普通 SFR 声明，不使用 `STC8H_SFRX`。
- 已检查 `build/stc8h_gpio.asm`，端口模式寄存器读写生成为 direct `mov _P?M?`，未发现端口模式寄存器使用 `movx` 访问。
- 已用 STC 官方 `STC8H Series Manual` 核对端口模式寄存器为 direct DATA/SFR 地址：`P0M1=0x93`、`P0M0=0x94`、`P1M1=0x91`、`P1M0=0x92`、`P2M1=0x95`、`P2M0=0x96`、`P3M1=0xB1`、`P3M0=0xB2`、`P4M1=0xB3`、`P4M0=0xB4`、`P5M1=0xC9`、`P5M0=0xCA`。

Keil C51 验证状态：

- 本机未发现 `c51`、`C51`、`uv4`、`UV4` 或 `wine`，暂不能自动完成 Keil C51 编译验证。
- 当前仅完成 SDCC 编译验证；Keil C51 状态为待人工验证。

备注：

- SDCC `.mem` 报告中的最大 ROM 按通用 64K 地址空间显示，实际 STC8H1K08 约束按 8K Flash 判断。
- SDCC `.mem` 中 `No spare internal RAM space left` 是栈区显示方式；本报告以 `Stack starts at` 作为当前内部 RAM 使用边界。
- `milestone1_demo` 当前 ROM 约 2.2K，低于 8K。
- UART 初始化已改为编译期 reload，map/sym 文件未发现 `__divulong`。
- UART1 已按官方 STC8H 串口示例核对：Timer1 作为波特率发生器，Timer1 1T，Timer1 mode0 16 位自动重装，11.0592MHz / 115200 reload 为 `0xFFE8`。
- I2C 板级初始化已按官方 STC8H GPIO/XFR 资料核对：开漏模式需要上拉；P1.7/P3.2 已启用数字输入和内部 4.1K 上拉。

## 9. PlatformIO `ec11_counter` 示例

路径：

```text
examples/platformio/ec11_counter
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- UART1 输出 EC11 旋转方向和累计计数。
- EC11 默认支持快速旋转加速：两次有效定位格间隔小于等于 30ms 时，每格输出 `+10/-10`。
- EC11 A/B 使用 P1.0/P1.1，按键 SW 使用 P5.4。
- 按键事件输出 `press`、`short`、`long`、`release`。
- LED 翻转表示主循环仍在运行。

资料依据：

- STC 官方手册：P1/P5 数字输入使能、内部上拉、GPIO 模式寄存器。
- Alps Alpine EC11E 官方资料：EC11 系列旋转编码器为两相 A/B 增量输出；具体器件以实物和实际型号为准。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 4465 bytes |
| Stack start | 0x59 |
| Internal RAM 边界 | 栈从 0x59 开始，当前静态/参数/overlay 占用到 0x58 |
| XDATA/PDATA | XFR 扩展寄存器访问，用于 `P1PU/P5PU/P1IE/P5IE` 等 |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| Button | SW=P5.4，轮询扫描 |
| EC11 | A=P1.0，B=P1.1，轮询扫描 |
| EC11 运行时配置 | 快速阈值、快速步进值、方向、每定位格跳变数可通过 API 修改 |
| I2C | 未使用 |
| LCD1602 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/board_init_wrap.rel
.pio/build/STC8H1K08/src/stc8h_delay_wrap.rel
.pio/build/STC8H1K08/src/stc8h_gpio_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
.pio/build/STC8H1K08/src/drv_button_wrap.rel
.pio/build/STC8H1K08/src/drv_ec11_wrap.rel
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 I2C、LCD1602、`stc8h_interrupt`、SPI、PWM、ADC、EEPROM、TM1637、IR、ring buffer、soft timer、CRC、filter 或除法库符号。

硬件验证状态：

- 已完成 SDCC 编译和资源检查。
- 已完成烧录实测。
- 慢速顺时针输出 `+1`，慢速逆时针输出 `-1`。
- 快速顺时针已触发 `+10` 步进。
- EC11 SW 短按已输出 `SW press` / `SW short`。
- EC11 SW 长按复测通过，输出 `SW long ms=800`，松开输出 `SW release ms=1067`。
- EC11 每定位格有效跳变数默认保持 `4`；测试例程已改为 1ms 扫描、100ms 汇总打印，避免串口逐格打印拖慢 EC11 扫描。
- 慢速旋转复测通过：顺时针主要输出 `+1`，逆时针主要输出 `-1`。
- 快速旋转复测通过：顺时针出现 `delta=30`，逆时针出现 `delta=-30`。

## 10. PlatformIO `adc_pot` 示例

路径：

```text
examples/platformio/adc_pot
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- UART1 周期输出 P3.3/ADC11 的 10-bit ADC 原始值。
- LED 翻转表示主循环仍在运行。

资料依据：

- STC 官方手册：STC8H1K08 为 10-bit ADC，P3.3 对应 ADC11；ADC 引脚需配置为高阻输入；ADC 上电后等待约 1ms；`ADCTIM=0x3f`；10-bit ADC 速度不高于 500kHz。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1582 bytes |
| Stack start | 0x21 |
| Internal RAM 边界 | 栈从 0x21 开始，当前静态/参数/overlay 占用到 0x20 |
| XDATA/PDATA | XFR 扩展寄存器访问，用于 `ADCTIM` 和 P3.3 数字输入控制 |
| Timer | Timer1 由 UART1 初始化使用 |
| UART | UART1 |
| ADC | P3.3/ADC11，轮询转换 |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/board_init_wrap.rel
.pio/build/STC8H1K08/src/stc8h_adc_wrap.rel
.pio/build/STC8H1K08/src/stc8h_delay_wrap.rel
.pio/build/STC8H1K08/src/stc8h_gpio_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_interrupt`、SPI、EEPROM、TM1637、IR、ring buffer、soft timer、CRC、filter 或除法库符号。

硬件验证状态：

- 已完成 SDCC 编译和资源检查。
- 已完成烧录实测。
- P3.3/ADC11 输出覆盖低端、中间和高端，观察到 `0`、`47`、`235`、`334`、`503`、`511`、`657`、`826`、`1023` 等读数。
- 旋转 10K 电位器时 ADC 原始值随位置变化。

## 11. PlatformIO `timer_tick` 示例

路径：

```text
examples/platformio/timer_tick
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- Timer0 产生 1ms tick。
- Timer0 ISR 每约 500ms 翻转 P1.2 LED。
- UART1 每约 1000ms 输出一次 `tick`。
- 验证 Timer0 与 UART1 使用的 Timer1 可以同时工作。

资料依据：

- STC 官方手册：Timer0 中断向量为 1；Timer0 12T/1T 由 `AUXR` bit7 选择；Timer0 时钟输出由 `INTCLKO` bit0 控制。
- STC 官方 Timer0 示例：11.0592MHz、12T、1ms tick 重装值为 `0xFC66`。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1473 bytes |
| Stack start | 0x21 |
| Internal RAM 边界 | 栈从 0x21 开始，当前静态/参数/overlay 占用到 0x20 |
| XDATA/PDATA | 未使用 |
| Timer | Timer0 1ms tick；Timer1 由 UART1 初始化使用 |
| 中断 | Timer0 向量 1；全局中断显式开启 |
| UART | UART1 |
| GPIO | P1.2 LED |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/board_init_wrap.rel
.pio/build/STC8H1K08/src/stc8h_gpio_wrap.rel
.pio/build/STC8H1K08/src/stc8h_interrupt_wrap.rel
.pio/build/STC8H1K08/src/stc8h_timer_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
```

关键符号检查：

```text
_timer0_isr
_stc8h_interrupt_enable_global
_stc8h_timer_init_1ms
_stc8h_timer_start
_stc8h_timer_enable_interrupt
_stc8h_timer_clear_flag
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、ring buffer、soft timer、CRC、filter 或除法库符号。

硬件验证状态：

- 已完成 SDCC 编译和资源检查。
- 已完成烧录实测。
- 串口输出已验证：启动后输出 `Timer0 1ms tick`，随后每约 1 秒输出 `tick`。
- Timer0 中断、全局中断、UART1/Timer1 并行工作已验证。
- P1.2 LED 每约 500ms 翻转仍需人工目视确认。

## 12. PlatformIO `soft_timer_tick` 示例

路径：

```text
examples/platformio/soft_timer_tick
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- Timer0 只提供 1ms tick。
- 主循环使用两个 `util_soft_timer_t` 对象分别控制 250ms LED 翻转和 1000ms UART 输出。
- 验证软件定时器不占用额外硬件资源。

设计取舍：

- `util_soft_timer_t` 使用 16-bit tick，单个对象占 4 字节 RAM。
- 1ms tick 下支持 65 秒以内的非阻塞间隔。
- 不做任务调度器，不保存函数指针，不自动注册 timer。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1694 bytes |
| Stack start | 0x25 |
| Internal RAM 边界 | 栈从 0x25 开始，当前静态/参数/overlay 占用到 0x24 |
| XDATA/PDATA | 未使用 |
| Timer | Timer0 1ms tick；Timer1 由 UART1 初始化使用 |
| 中断 | Timer0 向量 1；全局中断显式开启 |
| UART | UART1 |
| GPIO | P1.2 LED |
| Soft Timer | 两个 4 字节对象，分别用于 250ms 和 1000ms 周期 |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/board_init_wrap.rel
.pio/build/STC8H1K08/src/stc8h_gpio_wrap.rel
.pio/build/STC8H1K08/src/stc8h_interrupt_wrap.rel
.pio/build/STC8H1K08/src/stc8h_timer_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
.pio/build/STC8H1K08/src/util_soft_timer_wrap.rel
```

关键符号检查：

```text
_timer0_isr
_util_soft_timer_start
_util_soft_timer_expired
_stc8h_timer_init_1ms
_stc8h_timer_start
_stc8h_timer_enable_interrupt
_stc8h_timer_clear_flag
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、ring buffer、CRC、filter 或除法库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 已完成宿主机 16-bit 回绕测试。
- 等待烧录实测。
