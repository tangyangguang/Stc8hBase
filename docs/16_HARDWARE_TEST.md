# 硬件测试清单

本文档用于 Milestone 1 硬件实测。

## 1. 当前可烧录产物

已通过 SDCC + Makefile 编译：

```text
examples/make/gpio_blink/build/gpio_blink.ihx
examples/make/i2c_scan/build/i2c_scan.ihx
examples/make/milestone1_demo/build/milestone1_demo.ihx
```

已通过 PlatformIO 编译：

```text
examples/platformio/gpio_blink/.pio/build/STC8H1K08/firmware.hex
examples/platformio/uart_hello/.pio/build/STC8H1K08/firmware.hex
examples/platformio/i2c_lines/.pio/build/STC8H1K08/firmware.hex
examples/platformio/i2c_scan/.pio/build/STC8H1K08/firmware.hex
examples/platformio/lcd1602_text/.pio/build/STC8H1K08/firmware.hex
examples/platformio/ec11_counter/.pio/build/STC8H1K08/firmware.hex
examples/platformio/adc_pot/.pio/build/STC8H1K08/firmware.hex
examples/platformio/timer_tick/.pio/build/STC8H1K08/firmware.hex
```

## 2. 板级默认配置

测试板目录：

```text
board/stc8h1k08_tssop20_demo/
```

默认配置：

| 项目 | 配置 |
| --- | --- |
| 芯片 | STC8H1K08 |
| 封装 | TSSOP20 |
| 系统时钟 | 11.0592MHz |
| UART1 波特率 | 115200 |
| LCD1602 默认地址 | 0x27 |
| LCD1602 备用常见地址 | 0x3F |

## 3. 当前引脚宏

| 功能 | 当前宏 |
| --- | --- |
| LED | P1.2 |
| I2C SDA | P3.2 |
| I2C SCL | P1.7 |
| EC11 A | P1.0 |
| EC11 B | P1.1 |
| EC11 SW | P5.4 |
| 10K 电位器 | P3.3 |
| UART1 | 默认 UART1 引脚，优先按 STC 官方 P3.0/RXD、P3.1/TXD 接线 |

当前已按核心板现状设置：LED 接 P1.2，高电平点亮。烧录或实测前，仍应按实际 TSSOP20 原理图核对 UART、电源和复位接线。

## 4. 建议测试顺序

### 4.1 `gpio_blink`

目的：

- 验证最小 GPIO、delay、端口模式配置。

观察：

- LED 先快闪 8 次，单次亮/灭间隔约 100ms。
- 暂停约 800ms。
- LED 再慢闪 3 次，单次亮/灭间隔约 500ms。
- 暂停约 1200ms，然后循环。

实测结果：

- P1.2 LED 已确认正常闪烁。
- 旧版 `gpio_blink` 为每 500ms 翻转一次，完整亮灭周期约 1s；新版改为快闪/慢闪组合，便于肉眼区分。
- 新版快闪/慢闪节奏已确认可见。
- 该项验证通过。

### 4.2 `uart_hello`

目的：

- 独立验证 UART1 发送。
- 避免 I2C 扫描结果和 UART 输出问题混在一起判断。

串口参数：

```text
115200 8N1
```

预期输出：

```text
UART hello 115200
```

PlatformIO 测试命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/uart_hello
pio run -t upload --upload-port /dev/cu.usbserial-110
pio device monitor --port /dev/cu.usbserial-110 --baud 115200
```

说明：

- UART1 已按 STC8H 官方示例口径改为 Timer1 16 位自动重装方式。
- 11.0592MHz / 115200 的 reload 为 `0xFFE8`，来自 `65536 - FOSC / baud / 4`。
- UART 初始化会显式选择 Timer1 作为 UART1 波特率源、关闭 Timer1 时钟输出、选择 UART1 默认 P3.0/P3.1 引脚组，并把 P3.0/P3.1 设为标准模式。
- 默认配置为 `11.0592MHz / 115200`。
- 如果此示例无输出，下一步优先用示波器看 P3.1/TXD 是否有波形，以区分 MCU 端 UART 和 USB 转串口接收链路问题。

### 4.3 `i2c_lines`

目的：

- 独立验证软件 I2C 使用的 SDA/SCL 两根线是否能释放为高、主动拉低。
- 排查 I2C 扫描全地址 ACK 或无 ACK 时，是总线电平问题还是 I2C 状态机问题。

预期输出：

```text
release SDA=1 SCL=1
sda_low SDA=0 SCL=1
scl_low SDA=1 SCL=0
both_low SDA=0 SCL=0
```

PlatformIO 测试命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/i2c_lines
pio run -t upload --upload-port /dev/cu.usbserial-110
pio device monitor --port /dev/cu.usbserial-110 --baud 115200
```

说明：

- STC8H 开漏输出的内部上拉默认关闭；如果外部没有有效上拉，SDA/SCL 释放后可能读不到高电平。
- `P1PU/P3PU` 和 `P1IE/P3IE` 是 XFR 扩展寄存器，访问前必须设置 `P_SW2.EAXFR=1`。
- 当前板级初始化已为 P1.7/SCL、P3.2/SDA 启用数字输入和内部 4.1K 上拉。

### 4.4 `i2c_scan`

目的：

- 验证 UART1 输出。
- 验证软件 I2C 能收到 ACK。
- 确认 LCD1602 I2C 背包实际 `addr7`。

串口参数：

```text
115200 8N1
```

预期输出：

```text
I2C scan
0x27
```

PlatformIO 测试命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/i2c_scan
pio run -t upload --upload-port /dev/cu.usbserial-110
pio device monitor --port /dev/cu.usbserial-110 --baud 115200
```

说明：

- PlatformIO 示例上传时使用 `stc8g + 38400 + -t 11059.2`。
- `-t 11059.2` 的单位是 kHz，用于把 STC8H1K08 内部 RC 修调到 11.0592MHz，使运行频率与 `STC8H_SYSCLK_HZ`、UART1 115200 配置一致。
- `i2c_scan` 会每约 2 秒重复输出一次扫描结果，打开串口监视器后不需要抢上电瞬间。

如果显示 `none`：

- 检查 SDA/SCL 是否接反。
- 检查 LCD1602 供电和 GND。
- 检查 I2C 上拉。
- 用示波器看 SCL/SDA 是否有波形。
- 如果扫描到 `0x3F`，把 `BOARD_LCD1602_ADDR7` 改为 `0x3Fu` 后再测 `milestone1_demo`。

### 4.5 `lcd1602_text`

目的：

- 验证 GPIO、UART1、软件 I2C、LCD1602 组合可用。

预期：

- 串口输出 `LCD1602 test`。
- LCD 第一行显示 `Stc8hBase`。
- LCD 第二行显示 `LCD1602 OK`。
- LED 约 500ms 翻转一次。

PlatformIO 测试命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/lcd1602_text
pio run -t upload --upload-port /dev/cu.usbserial-110
```

### 4.6 `ec11_counter`

目的：

- 验证 EC11 A/B 相方向、慢速计数和快速旋转加速。
- 验证 EC11 SW 按键短按、长按事件。

预期：

- 慢速顺时针每定位格输出 `+1`。
- 慢速逆时针每定位格输出 `-1`。
- 默认快速旋转阈值为 30ms，触发后每定位格输出 `+10/-10`。
- 当前演示板通过 `DRV_EC11_REVERSE=1` 校正方向。
- 当前默认每定位格有效跳变数为 `4`；测试例程使用 1ms 扫描和 100ms 汇总打印减少漏扫。
- SW 按钮使用独立 `drv_button`，默认消抖 10ms，长按阈值 800ms。

PlatformIO 测试命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/ec11_counter
pio run -t upload --upload-port /dev/cu.usbserial-110
pio device monitor --port /dev/cu.usbserial-110 --baud 115200
```

### 4.7 `adc_pot`

目的：

- 验证 P3.3/ADC11 采样。
- 验证 10K 电位器输入能得到 0..1023 的 10-bit ADC 原始值。

预期：

- 串口周期输出 `adc=...`。
- 旋转电位器时数值应单调变化。
- 接近 GND 时读数接近 0，接近 VCC 时读数接近 1023。

PlatformIO 测试命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/adc_pot
pio run -t upload --upload-port /dev/cu.usbserial-110
pio device monitor --port /dev/cu.usbserial-110 --baud 115200
```

### 4.8 `timer_tick`

目的：

- 验证 Timer0 1ms tick。
- 验证 Timer0 中断向量和全局中断开关。
- 验证 Timer0 与 UART1 使用的 Timer1 不冲突。

预期：

- 串口启动后输出 `Timer0 1ms tick`。
- UART1 每约 1 秒输出一次 `tick`。
- P1.2 LED 每约 500ms 翻转一次。

PlatformIO 测试命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/timer_tick
pio run -t upload --upload-port /dev/cu.usbserial-110
pio device monitor --port /dev/cu.usbserial-110 --baud 115200
```

说明：

- 当前 Timer HAL 只实现 Timer0 1ms tick 初始化、启动、停止、中断使能和清标志。
- ISR 由示例文件显式定义，HAL 不默认占用中断向量。
- Timer0 使用 12T 模式，11.0592MHz 下 1ms 重装值为 `0xFC66`。
- UART1 仍使用 Timer1 作为波特率发生器。

## 5. 本机工具状态

当前本机已发现：

```text
pio
```

当前本机未发现：

```text
c51 / C51 / uv4 / UV4 / wine
```

因此：

- SDCC + Makefile 编译已自动验证。
- PlatformIO 板型查询已确认存在 `STC8H1K08`。
- PlatformIO 已自动安装 `tool-stcgal`。
- Keil C51 编译需在有 Keil 的环境中人工验证。
- 烧录需要让芯片进入 STC ISP 握手流程，通常需要在工具等待时手动断电再上电。

## 6. 当前实测记录

当前核心板：

```text
串口：/dev/cu.usbserial-110
LED：P1.2，高电平点亮
LCD1602 背包：8574T / PCF8574T 兼容
LCD1602 I2C 地址：0x27
```

已完成：

- `gpio_blink` 已按 P1.2 重新编译。
- PlatformIO `gpio_blink` 已重新编译，Flash `927 bytes / 8192 bytes`。
- 已尝试执行：

```text
pio run -t upload --upload-port /dev/cu.usbserial-110
```

结果：

- PlatformIO 找到串口并启动 `stcgal`。
- 已完成 STC Bootloader 握手，识别到 `STC8H1K08`。
- 失败点在 `Switching to 115200 baud` 之后，报 `Serial port error: read timeout`。
- 已把 PlatformIO `gpio_blink` 示例上传速率降为 `9600`，用于规避 USB 转串口或自动复位链路在 115200 下的下载超时。
- 第二次已切到 `9600`，但在擦除前报 `Protocol error: incorrect frame start`；日志显示芯片当前频率为 `5.999MHz`，因此先取消上传时的 `-t 11059` 频率修调，避免 ISP 阶段变频后串口时序异常。
- 已检查本机 `stcgal 1.10` 源码，其自动识别规则将 `STC8H1Kxx` 映射到 `stc8g` 协议；PlatformIO 板卡默认传入 `stc8`，因此已把示例上传脚本改为 `-P stc8g`。
- 使用 `stc8g + 9600 + 不显式传入 -t` 已在两块 STC8H1K08 核心板上烧录成功。
- 默认下载速度已从 `9600` 提高到 `38400`。
- `stc8g + 38400 + 不显式传入 -t` 已烧录成功，写入速度约 `2272 Bytes/s`，总耗时约 `3.52s`。

说明：

- `stc8g` 是 `stcgal` 内部下载协议名，不表示芯片型号是 STC8G。
- 本项目芯片目标仍是 `STC8H1K08`。
- 对 STC8H1K08，实测和 `stcgal 1.10` 源码规则都表明应优先使用 `stc8g` 协议。

下一次烧录时，在出现上传等待后，需要手动给核心板断电再上电。

当前结论：

- `38400` 作为当前默认下载速度。
- 暂不继续提高到 `57600` 或 `115200`，避免为少量速度收益牺牲稳定性。
- `gpio_blink` 已完成硬件验证：P1.2 LED 快闪/慢闪正常。
- `i2c_scan` 已成功烧录到一块当前频率约 6MHz 的 STC8H1K08；首次版本打开 monitor 后无输出。
- 原因判断：首次版本只在启动时输出一次，且上传时未修调芯片到 11.0592MHz，运行频率与 UART1 115200 配置不一致。
- 已把 PlatformIO 示例改为上传时修调到 `11059.2kHz`，并让 `i2c_scan` 每约 2 秒重复输出扫描结果，等待复测。
- `i2c_scan` 复测时上传日志已确认运行频率修调到 `11.054MHz`，但 monitor 仍无输出。
- 已根据 STC8H 官方 UART 示例口径，把 UART1 从 Timer1 8 位 reload 改为 Timer1 16 位自动重装方式。
- 已新增 PlatformIO `uart_hello` 最小串口示例，等待先独立验证 UART1。
- 已再次核对官方 UART1 公式，修正 11.0592MHz / 115200 reload 为 `0xFFE8`，并补齐 `AUXR.S1ST2`、`P_SW1`、`INTCLKO`、P3.0/P3.1 模式设置。
- `uart_hello` 已硬件实测通过：串口监视器 115200 8N1 可连续收到 `UART hello 115200`。
- `i2c_scan` 曾出现 `0x08` 到 `0x77` 全地址 ACK；经 `i2c_lines` 诊断，原因为 SDA 开漏释放后仍读 0，即总线缺少有效上拉导致 ACK 假阳性。
- 已按官方资料启用 P1.7/P3.2 数字输入和内部 4.1K 上拉；`i2c_lines` 复测通过：`release SDA=1 SCL=1`，各拉低状态均正确。
- 内部上拉启用后，`i2c_scan` 不再全地址 ACK，首次结果为 `none`；检查后发现 LCD1602 有一根线接错。
- 修正 LCD1602 接线后，`i2c_scan` 已扫描到 `0x27`。
- `lcd1602_text` 已硬件实测通过：串口输出 `LCD1602 test`，LCD 显示 `Stc8hBase` / `LCD1602 OK`。
- `ec11_counter` 已编译通过，Flash `3843 bytes / 8192 bytes`，Stack start `0x52`。
- `ec11_counter` 已烧录成功，慢速顺时针输出 `+1`，慢速逆时针输出 `-1`。
- `ec11_counter` 快速顺时针已触发加速，实测输出 `+10` 步进。
- `ec11_counter` 短按已输出 `SW press` / `SW short`。
- `ec11_counter` 首轮长按实测仍输出 `SW short`；已优化按钮长按判定。
- 复测已通过：长按输出 `SW long ms=800`，松开输出 `SW release ms=1067`。
- `ec11_counter` 首轮旋转偶发漏格；已把测试例程改为 1ms 扫描、100ms 汇总打印。
- 复测已通过：慢速顺时针主要输出 `+1`，慢速逆时针主要输出 `-1`；`delta=2/-2` 表示 100ms 汇总窗口内转过两格。
- 快速旋转复测已通过：快速顺时针出现 `delta=30`，快速逆时针出现 `delta=-30`，说明 30ms 阈值和 10 步进加速生效。
- `adc_pot` 已烧录实测通过。
- P3.3/ADC11 输出覆盖低端、中间和高端：已观察到 `0`、`47`、`235`、`334`、`503`、`511`、`657`、`826`、`1023` 等读数。
- 旋转 10K 电位器时 ADC 原始值随位置变化，10-bit 右对齐读取有效。
- `timer_tick` 已烧录成功。
- `timer_tick` 串口实测通过：启动后输出 `Timer0 1ms tick`，随后每约 1 秒输出 `tick`。
- `timer_tick` 已验证 Timer0 中断、全局中断、UART1/Timer1 并行工作。
- `timer_tick` 的 P1.2 LED 每约 500ms 翻转仍需人工目视确认。
