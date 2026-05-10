# Keil C51 编译验证

## 1. 当前状态

当前仓库已准备 Keil C51 模块编译验证入口：

```text
examples/keil_c51/module_compile_check/
```

本机是 macOS 环境，未安装 Keil C51，因此尚不能给出“Keil C51 已编译通过”的结论。

## 2. 验证范围

验证入口覆盖当前已实现模块：

- `core/stc8h_delay`
- `core/stc8h_interrupt`
- `hal/stc8h_gpio`
- `hal/stc8h_uart`
- `hal/stc8h_i2c`
- `hal/stc8h_timer`
- `hal/stc8h_adc`
- `hal/stc8h_spi`
- `hal/stc8h_eeprom`
- `hal/stc8h_pwm`
- `drivers/drv_button`
- `drivers/drv_ec11`
- `drivers/drv_lcd1602`
- `drivers/drv_led`
- `drivers/drv_buzzer`
- `drivers/drv_relay`
- `drivers/drv_tm1637`
- `drivers/drv_ir_tx`
- `drivers/drv_ir_rx`
- `utils/util_ring_buffer`
- `utils/util_soft_timer`
- `utils/util_crc`
- `utils/util_filter`

## 3. 验证方式

在 Windows + Keil C51 环境中执行：

```bat
cd examples\keil_c51\module_compile_check
build_c51.bat
```

如果 `C51.exe` 不在 `PATH` 中，先按本机安装位置设置：

```bat
set PATH=C:\Keil_v5\C51\BIN;%PATH%
```

## 4. 设计说明

验证入口使用 wrapper 源文件：

```c
#define STC8H_CONFIG_INCLUDE "board_config.h"
#define STC8H_PINS_INCLUDE "board_pins.h"
#include "../../../../hal/stc8h_gpio.c"
```

这样做的原因：

- 避免在 Keil 命令行中传递带引号宏，减少批处理转义差异。
- 编译的仍是原始库源码，不复制模块实现。
- 可以按模块独立编译，便于定位 `code`、`xdata`、`interrupt`、SFR 等 C51 兼容问题。

## 5. 判定标准

通过：

- 所有 wrapper 源文件编译成功。
- 没有 memory qualifier、SFR、interrupt、code 字符串接口相关错误。

需要修正：

- `code`/`xdata`/`data` 等关键字位置不被 Keil C51 接受。
- `sfr`/`sbit`/xdata SFR 声明冲突。
- ISR 宏展开不符合 C51 语法。
- `const code` 字符串或表格声明无法编译。

## 6. 资料依据

- Keil 官方 Cx51 用户手册：命令行 directive 可在文件名后指定，例如 `C51 testfile.c SYMBOLS CODE DEBUG`。
- Keil 官方 Cx51 `const` 文档：固定字符串和查表数据可放入 `code` 区。
- Keil 官方 C51 指针说明：memory-specific pointer 比 generic pointer 更小更快，适合 8051 资源受限场景。

## 7. 待完成

- 在 Windows + Keil C51 环境运行 `build_c51.bat`。
- 将实际结果回填到 `docs/RESOURCE_REPORT.md` 和 `docs/15_REMAINING_WORK.md`。
