# 应用项目引用模式

本文记录真实应用项目接入 `Stc8hBase` 的推荐方式。基础库只提供芯片级能力、薄 HAL、可复用外设协议和工具；应用项目继续负责板级接线、电平、低功耗前后 IO 状态、业务命令映射和产品状态机。

## 1. Makefile 直接编译源文件

适合小型 SDCC/Make 项目。应用项目在 `CFLAGS` 中加入基础库 `core/hal/drivers/utils` include path，并只把实际使用的 `.c` 文件放入 `SRCS`。

这种方式的优点是构建关系直观、未使用模块天然零占用。项目侧仍应用 `STC8H_CONFIG_INCLUDE` 和 `STC8H_PINS_INCLUDE` 指定自己的 `board_config.h`、`board_pins.h`。

## 2. PlatformIO wrapper include

适合 PlatformIO 项目。应用项目在 `src/base/*.c` 中用 wrapper 引入基础库实现，例如：

```c
#define STC8H_CONFIG_INCLUDE "board_config.h"
#define STC8H_PINS_INCLUDE "board_pins.h"
#include "../../../Stc8hBase/hal/stc8h_gpio.c"
```

wrapper 方式避免复制基础库源码，也能让 PlatformIO 自动编译项目 `src/` 下的入口。只为实际使用的基础库源文件创建 wrapper；例如普通 GPIO 只需要 `stc8h_gpio.c`，需要 `P1/P3/P5` 上拉或数字输入 mask 操作时才额外加入 `stc8h_gpio_xfr.c`。

## 3. 边界原则

- 基础库负责 GPIO、EXTI、Timer、PWM、UART、power-down 入口，以及 NEC `mark/space` 编码和解码状态机。
- 板级代码负责引脚组、电平有效性、上拉/数字输入、未用脚低功耗收敛、唤醒源和外设资源分配。
- 应用代码负责产品命令表、repeat 策略、灯光状态机、按键优先级和调试日志。
- Timer0 模式由单个固件统一分配；不要同时把 Timer0 用作 1T 阻塞延时和 12T free-run 接收计时。
- `PWMA` 1..4 共用周期；同一固件需要多个 PWM 通道时，必须选择同一个频率。

## 4. 来自真实项目的结论

红外夜灯项目使用 Makefile 直接编译基础库源文件，接收端用 Timer0 12T free-run + INT0 边沿捕获解码 NEC。红外遥控器项目使用 PlatformIO wrapper include，发送端用 PWM 产生 38kHz 载波，用 Timer0 1T 阻塞延时输出 NEC 帧。

这两个项目验证了当前基础库边界是合适的：不需要把遥控器或夜灯业务沉淀进基础库；需要沉淀的是 GPIO XFR mask、EXTI 批量清标志、PWM 共享周期语义和 Timer0 资源说明这类小而明确的基础能力。
