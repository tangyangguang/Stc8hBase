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
examples/platformio/i2c_scan/.pio/build/STC8H1K08/firmware.hex
examples/platformio/lcd1602_text/.pio/build/STC8H1K08/firmware.hex
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
- 默认配置为 `11.0592MHz / 115200`。
- 如果此示例无输出，优先检查核心板 USB 转串口是否连接到应用运行时的 UART1 引脚。

### 4.3 `i2c_scan`

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

### 4.4 `lcd1602_text`

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
