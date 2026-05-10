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

## 13. PlatformIO `ring_buffer_demo` 示例

路径：

```text
examples/platformio/ring_buffer_demo
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 `util_ring_buffer` 空读、满写拒绝、回绕写入和顺序读出。
- UART1 周期输出 `ring buffer ok` 或 `ring buffer error`。

设计取舍：

- 默认使用 internal DATA RAM 缓冲，避免通用指针带来的代码和周期成本。
- 保留一个空位区分空/满，实际可存字节数为 `size - 1`。
- 不使用 `%` 取模，避免拉入除法/取模库。
- 不内置中断保护；ISR 与主循环并发访问时，由调用方处理临界区。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1352 bytes |
| Stack start | 0x23 |
| Internal RAM 边界 | 栈从 0x23 开始，当前静态/参数/overlay 占用到 0x22 |
| DATA buffer | 示例使用 4 字节数组，实际可存 3 字节 |
| XDATA/PDATA | 未使用 |
| Timer | 未使用 |
| 中断 | 未使用 |
| UART | UART1 |
| Ring Buffer | `util_ring_buffer` |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
.pio/build/STC8H1K08/src/util_ring_buffer_wrap.rel
```

关键符号检查：

```text
_util_ring_buffer_init
_util_ring_buffer_push
_util_ring_buffer_pop
_util_ring_buffer_available
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、soft timer、CRC、filter 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 已完成宿主机回绕测试。
- 等待烧录实测。

## 14. PlatformIO `uart_echo_buffered` 示例

路径：

```text
examples/platformio/uart_echo_buffered
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- UART1 轮询接收。
- 接收字节先进入 `util_ring_buffer`，再由主循环弹出并回显。
- 用于验证 ring buffer 在 UART RX 场景中的低速使用方式。

设计取舍：

- 当前示例不启用 UART 中断，避免 RI/TI 共用中断导致 TX/RX 状态机复杂化。
- 适合人工输入或低速输入回显。
- 高吞吐连续接收后续需要单独设计 UART 中断收发状态机。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 925 bytes |
| Stack start | 0x2f |
| Internal RAM 边界 | 栈从 0x2f 开始，当前静态/参数/overlay 占用到 0x2e |
| DATA buffer | 示例使用 16 字节数组，实际可存 15 字节 |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | 未使用 |
| UART | UART1，轮询接收 |
| Ring Buffer | `util_ring_buffer` |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
.pio/build/STC8H1K08/src/util_ring_buffer_wrap.rel
```

关键符号检查：

```text
_stc8h_uart_init
_stc8h_uart_readable
_stc8h_uart_getc
_stc8h_uart_putc
_util_ring_buffer_init
_util_ring_buffer_push
_util_ring_buffer_pop
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、soft timer、CRC、filter 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 等待烧录实测。

## 15. PlatformIO `crc_demo` 示例

路径：

```text
examples/platformio/crc_demo
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 `util_checksum8`。
- 验证 `util_crc16_modbus`。
- UART1 周期输出 `crc ok` 或 `crc error`。

设计取舍：

- `util_crc` 第一版只处理 RAM 数据指针。
- CRC16/MODBUS 使用逐位算法，不使用查表常量。
- 不做 CRC 参数对象、运行期多项式选择或反射配置。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 646 bytes |
| Stack start | 0x19 |
| Internal RAM 边界 | 栈从 0x19 开始，当前静态/参数/overlay 占用到 0x18 |
| DATA buffer | 示例使用 9 字节测试数据 |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | 未使用 |
| UART | UART1 |
| CRC | `checksum8`、`crc16_modbus` |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
.pio/build/STC8H1K08/src/util_crc_wrap.rel
```

关键符号检查：

```text
_util_checksum8
_util_crc16_modbus
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、ring buffer、soft timer、filter 或除法/取模库符号。

验证状态：

- 已完成宿主机标准向量测试：`"123456789"` 的 `checksum8=0xDD`，`crc16_modbus=0x4B37`。
- 已完成 SDCC 编译和资源检查。
- 等待烧录实测。

## 16. PlatformIO `filter_demo` 示例

路径：

```text
examples/platformio/filter_demo
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 `util_filter_limit_u16`。
- 验证 `util_filter_iir_u16`。
- UART1 周期输出 `filter ok` 或 `filter error`。

设计取舍：

- 不实现任意长度平均数组，避免除法库和额外样本缓存。
- `util_filter_iir_u16` 使用移位平滑，适合 ADC、电位器和慢速传感器输入。
- `shift=0` 表示立即跟随输入。
- 输入接近当前值时仍至少移动 1，避免小误差永久不收敛。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 946 bytes |
| Stack start | 0x11 |
| Internal RAM 边界 | 栈从 0x11 开始，当前静态/参数/overlay 占用到 0x10 |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | 未使用 |
| UART | UART1 |
| Filter | `limit_u16`、`iir_u16` |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
.pio/build/STC8H1K08/src/util_filter_wrap.rel
```

关键符号检查：

```text
_util_filter_limit_u16
_util_filter_iir_u16
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、ring buffer、soft timer、CRC 或除法/取模库符号。

验证状态：

- 已完成宿主机边界测试。
- 已完成 SDCC 编译和资源检查。
- 等待烧录实测。

## 17. PlatformIO `spi_loopback` 示例

路径：

```text
examples/platformio/spi_loopback
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证硬件 SPI 主机轮询收发。
- 短接 P1.3/MOSI 和 P1.4/MISO 后，发送字节应原样读回。
- UART1 周期输出 `spi loopback ok` 或 `spi loopback error`。

资料依据：

- STC 官方手册和官方 SPI 库：`SPSTAT=0xCD`、`SPCTL=0xCE`、`SPDAT=0xCF`。
- `SPCTL` bit7 为 `SSIG`，bit6 为 SPI 使能，bit4 为主机模式，bit3/bit2 为 CPOL/CPHA，bit1..0 为速度选择。
- `SPSTAT` bit7 为 `SPIF`，bit6 为 `WCOL`，写 1 清除。
- SPI 引脚组通过 `P_SW1[3:2]` 选择。

设计取舍：

- 第一版冻结为硬件 SPI 主机轮询实现。
- 默认 P1.3/P1.4/P1.5，硬件 SS 使用 `SSIG=1` 忽略，不占用当前 P1.2 LED。
- 默认 mode 0、MSB first、`SYSclk/4`。
- 不启用 SPI 中断，不使用 DMA，不保存 RX 缓冲。
- 片选由板级或应用代码自行控制，HAL 不保存 CS 引脚。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 563 bytes |
| Stack start | 0x0f |
| Internal RAM 边界 | 栈从 0x0f 开始，当前静态/参数/overlay 占用到 0x0e |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | 未使用 |
| UART | UART1 |
| SPI | 硬件 SPI 主机轮询，P1.3/P1.4/P1.5 |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| Utils | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_spi_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
```

关键符号检查：

```text
_stc8h_spi_init
_stc8h_spi_transfer
_stc8h_spi_write
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、EEPROM、TM1637、IR、utils 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 等待短接 P1.3/P1.4 后烧录实测。

## 18. PlatformIO `eeprom_rw` 示例

路径：

```text
examples/platformio/eeprom_rw
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 STC8H1K08 EEPROM/IAP 读、写、扇区擦除。
- 默认环境只验证编译和串口提示，不执行 EEPROM 写擦。
- 写擦环境 `STC8H1K08_write_test` 擦除 `0x0000..0x01FF`，写入 4 字节并读回校验。

资料依据：

- STC8H1K08 EEPROM/IAP 容量为 4KB，地址范围 `0x0000..0x0FFF`。
- 擦除粒度为 512 字节扇区。
- IAP 寄存器为 `IAP_DATA=0xC2`、`IAP_ADDRH=0xC3`、`IAP_ADDRL=0xC4`、`IAP_CMD=0xC5`、`IAP_TRIG=0xC6`、`IAP_CONTR=0xC7`、`IAP_TPS=0xF5`。
- IAP 触发序列为 `0x5A`、`0xA5`，触发窗口临时关闭全局中断。

设计取舍：

- HAL 只提供 `read`、`write`、`erase_sector`，不定义应用参数格式。
- 写入函数不隐式擦除，调用方必须先明确擦除扇区。
- 示例默认不执行写擦，避免误伤 EEPROM 中已有数据。
- 第一版只支持 `11.0592MHz` 和 `12MHz` 的 `IAP_TPS` 编译期配置。

默认安全环境资源占用：

| 项目 | 结果 |
| --- | --- |
| 环境 | `STC8H1K08` |
| ROM/EPROM/FLASH | 816 bytes |
| Stack start | 0x15 |
| Internal RAM 边界 | 栈从 0x15 开始，当前静态/参数/overlay 占用到 0x14 |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | 未启用；IAP 触发函数存在但默认环境不调用 |
| UART | UART1 |
| EEPROM/IAP | 编译入库，默认环境不执行写擦 |

写擦环境资源占用：

| 项目 | 结果 |
| --- | --- |
| 环境 | `STC8H1K08_write_test` |
| ROM/EPROM/FLASH | 952 bytes |
| Stack start | 0x1d |
| Internal RAM 边界 | 栈从 0x1d 开始，当前静态/参数/overlay 占用到 0x1c |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | IAP 触发窗口临时关闭全局中断，未启用 ISR |
| UART | UART1 |
| EEPROM/IAP | 擦除 `0x0000..0x01FF`，写入/读回 `0x0000..0x0003` |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/PWM/IR | 未使用 |
| Utils | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08_write_test/src/main.rel
.pio/build/STC8H1K08_write_test/src/stc8h_eeprom_wrap.rel
.pio/build/STC8H1K08_write_test/src/stc8h_uart_wrap.rel
```

关键符号检查：

```text
_stc8h_eeprom_read
_stc8h_eeprom_write
_stc8h_eeprom_erase_sector
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、TM1637、IR、utils 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 默认安全环境可直接烧录，预期输出 `eeprom write disabled`。
- 写擦环境等待用户确认 `0x0000..0x01FF` 可擦除后再烧录实测。

## 19. PlatformIO `output_levels` 示例

路径：

```text
examples/platformio/output_levels
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 `drv_led`、`drv_buzzer`、`drv_relay` 有效电平辅助。
- 验证 active-high 和 active-low 两类输出电平。
- UART1 周期输出 `output levels ok` 或 `output levels error`。

设计取舍：

- 驱动只保存 1 字节 `active_level`。
- 不直接操作 GPIO。
- 不实现闪烁、蜂鸣节奏、继电器保护时序等业务逻辑。
- 无源蜂鸣器不在本驱动中处理，后续通过 PWM 模块驱动。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 925 bytes |
| Stack start | 0x12 |
| Internal RAM 边界 | 栈从 0x12 开始，当前静态/参数/overlay 占用到 0x11 |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | 未使用 |
| UART | UART1 |
| LED/Buzzer/Relay | 有效电平辅助 |
| GPIO | 未使用 |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/EEPROM/PWM/IR | 未使用 |
| Utils | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/drv_buzzer_wrap.rel
.pio/build/STC8H1K08/src/drv_led_wrap.rel
.pio/build/STC8H1K08/src/drv_relay_wrap.rel
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
```

关键符号检查：

```text
_drv_led_init
_drv_led_level_on
_drv_led_level_off
_drv_buzzer_init
_drv_buzzer_level_on
_drv_buzzer_level_off
_drv_relay_init
_drv_relay_level_on
_drv_relay_level_off
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 GPIO、Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、utils 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 等待烧录实测串口输出。

## 20. PlatformIO `tm1637_number` 示例

路径：

```text
examples/platformio/tm1637_number
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 TM1637 二线 bit-bang 写显示数据。
- 默认 P1.6 为 CLK，P1.5 为 DIO。
- 周期显示递增数字，UART1 输出 `tm1637 ok` 或 `tm1637 error`。

资料依据：

- TM1637 datasheet：二线接口不是标准 I2C，无从地址。
- 显示寄存器地址为 `0xC0..0xC5`。
- 自动地址写数据命令为 `0x40`。
- 显示控制命令为 `0x88..0x8F`，低 3 位为亮度。
- 每字节传输后 TM1637 通过拉低 DIO 产生 ACK。

设计取舍：

- CLK/DIO 由板级宏编译期绑定，不走运行期 port/pin 分派。
- 不复用软件 I2C。
- `display_number` 不使用除法/取模库，使用小循环做十进制拆分。
- 第一版不实现按键扫描功能，只做显示。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1976 bytes |
| Stack start | 0x2b |
| Internal RAM 边界 | 栈从 0x2b 开始，当前静态/参数/overlay 占用到 0x2a |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用 |
| 中断 | 未使用 |
| UART | UART1 |
| GPIO | P1.6/CLK、P1.5/DIO，开漏输出 |
| TM1637 | 二线 bit-bang，ACK 检查 |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/EEPROM/PWM/IR | 未使用 |
| Utils | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/drv_tm1637_wrap.rel
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_delay_wrap.rel
.pio/build/STC8H1K08/src/stc8h_gpio_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
```

关键符号检查：

```text
_drv_tm1637_init
_drv_tm1637_set_brightness
_drv_tm1637_display_number
_drv_tm1637_display_raw
_stc8h_delay_us
_stc8h_gpio_set_mode
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、IR、utils 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 等待连接 TM1637 模块后烧录实测。

## 21. PlatformIO `pwm_output` 示例

路径：

```text
examples/platformio/pwm_output
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 `PWMA` channel 2 基础 PWM 输出。
- 默认 P1.2/PWM2P 输出，直接驱动当前核心板 LED。
- UART1 启动时输出 `pwm output ok` 或 `pwm output error`。

资料依据：

- STC8H 官方 PWM 资料和官方 `STC8H_PWM.c/.h`。
- `PWMA` 1..4 默认 P1 引脚组。
- `PWMA_ARRH/L` 设置周期，`PWMA_CCR2H/L` 设置 channel 2 占空比。
- `PWMA_ENO`、`PWMA_CCER1` 和 `PWMA_BKR.MOE` 共同打开 PWM2P 输出。

设计取舍：

- 第一版只实现 `PWMA` 1..4 基础 P 输出。
- 不实现互补输出、死区、刹车、中断、PWMB 和运行期引脚自动适配。
- `PWMA` 1..4 共用一个周期。
- 示例使用 P1.2 LED 做明暗变化验证。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 1987 bytes |
| Stack start | 0x21 |
| Internal RAM 边界 | 栈从 0x21 开始，当前静态/参数/overlay 占用到 0x20 |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用；PWMA 自身计数器用于 PWM |
| 中断 | 未使用 |
| UART | UART1 |
| GPIO | P1.2 设置为推挽输出 |
| PWM | PWMA channel 2 / P1.2 PWM2P |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC | 未使用 |
| SPI/EEPROM/IR | 未使用 |
| Utils | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_delay_wrap.rel
.pio/build/STC8H1K08/src/stc8h_gpio_wrap.rel
.pio/build/STC8H1K08/src/stc8h_pwm_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
```

关键符号检查：

```text
_stc8h_pwm_init
_stc8h_pwm_set_duty
_stc8h_pwm_enable
_stc8h_pwm_disable
_stc8h_gpio_set_mode
_stc8h_delay_ms
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、IR、utils 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 已烧录实测通过：串口输出 `pwm output ok`，P1.2 指示灯呈现逐渐变亮、逐渐变暗的呼吸效果。

## 22. PlatformIO `ir_nec_demo` 示例

路径：

```text
examples/platformio/ir_nec_demo
```

工具链：

```text
PlatformIO intel_mcs51 / SDCC 4.4.0
```

功能：

- 验证 `drv_ir_tx` NEC `mark/space` 编码器。
- 验证 `drv_ir_rx` NEC 解码状态机。
- 不接外部红外元件，内部把 TX 编码器输出的脉宽送入 RX 解码器。
- 串口每约 1 秒输出 `ir nec ok` 或 `ir nec error`。

资料依据：

- Infineon 官方红外遥控应用笔记用于核对 NEC 引导码、重复码、数据位脉宽和载波事实。
- 第一版只实现 NEC 普通 8-bit 地址格式：`address`、`~address`、`command`、`~command`。
- VS1838B/HS0038 这类解调接收头通常输出低有效 mark，因此 RX feed 中 `level=0` 表示 mark，`level=1` 表示 space。

设计取舍：

- `drv_ir_tx` 只生成 `mark/space + duration_us` 序列，不直接控制 GPIO、PWM 或 Timer。
- `drv_ir_rx` 只消费 `level + width_us`，不直接读取 GPIO、Timer 或中断标志。
- 真实硬件接收层后续由板级代码绑定外部中断/Timer 或固定周期采样。
- 真实硬件发射层后续由板级代码绑定 PWM/Timer 载波和红外 LED 驱动管。

资源占用：

| 项目 | 结果 |
| --- | --- |
| ROM/EPROM/FLASH | 2997 bytes |
| Stack start | 0x4d |
| Internal RAM 边界 | 栈从 0x4d 开始，当前静态/参数/overlay 占用到 0x4c |
| XDATA/PDATA | 未使用 |
| Timer | Timer1 由 UART1 初始化使用；IR 协议层未使用 Timer |
| 中断 | 未使用 |
| UART | UART1 |
| Delay | 阻塞延时，用于降低示例串口输出频率 |
| GPIO | 未使用 |
| PWM | 未使用 |
| IR | NEC TX 编码器、NEC RX 解码状态机 |
| I2C/LCD1602 | 未使用 |
| Button/EC11 | 未使用 |
| ADC/SPI/EEPROM/TM1637 | 未使用 |
| Utils | 未使用 |

链接文件检查：

```text
.pio/build/STC8H1K08/src/drv_ir_rx_wrap.rel
.pio/build/STC8H1K08/src/drv_ir_tx_wrap.rel
.pio/build/STC8H1K08/src/main.rel
.pio/build/STC8H1K08/src/stc8h_delay_wrap.rel
.pio/build/STC8H1K08/src/stc8h_uart_wrap.rel
```

关键符号检查：

```text
_drv_ir_rx_init
_drv_ir_rx_reset
_drv_ir_rx_feed_pulse
_drv_ir_rx_get_event
_drv_ir_tx_nec_begin
_drv_ir_tx_nec_next
_stc8h_delay_us
_stc8h_delay_ms
_stc8h_uart_init
_stc8h_uart_write_code
```

未使用模块检查：

- 已检查 PlatformIO 构建产物，未发现 Timer0、`stc8h_interrupt`、GPIO、I2C、LCD1602、`drv_button`、`drv_ec11`、`stc8h_adc`、SPI、EEPROM、TM1637、PWM、utils 或除法/取模库符号。

验证状态：

- 已完成 SDCC 编译和资源检查。
- 协议自检已烧录实测通过：串口 115200 可读到连续 `ir nec ok`。
- 降频版本已复烧确认：串口 115200 在约 4 秒内读到 2 行 `ir nec ok`，不再刷屏。
