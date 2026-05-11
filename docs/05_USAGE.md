# 使用方式

## 1. 使用目标

本基础库的使用目标是：

- 开发期方便引用最新基础库代码。
- 正式发布时可以复制当前稳定版本，冻结依赖。
- 每个应用项目只编译实际使用的模块。
- 老 Keil C51 项目可以逐步接入。
- 新项目优先使用 macOS + SDCC 工作流。

## 2. 推荐使用模式

### 2.1 开发期：公共库引用

开发期推荐把 `Stc8hBase` 作为公共库引用。

可以使用：

- Git 子模块。
- 固定公共目录引用。
- 项目内 `lib/Stc8hBase` 软引用或同步副本。

推荐目录：

```text
MyProject/
  app/
  board/
  lib/
    Stc8hBase/
  Makefile 或 platformio.ini
```

开发期这样做的好处是：多个小项目可以快速吃到基础库修复和新增模块。

### 2.2 正式发布：复制冻结

正式发布版本建议把当时验证过的基础库源码复制进项目，并随项目一起归档。

这样可以避免以后基础库更新影响已经发布的固件。

发布项目应该记录：

- 使用的基础库版本或提交号。
- 使用的芯片型号。
- 使用的系统时钟。
- 使用的编译器和版本。
- 实际加入编译的基础库模块列表。

## 3. 按需引入模块

应用项目只加入实际使用的 `.c` 文件。

例如只使用按键、EC11 和 I2C LCD1602 时，加入：

```text
core/stc8h_compiler.h
core/stc8h_types.h
core/stc8h_config.h
core/stc8h_delay.c
hal/stc8h_gpio.c
hal/stc8h_timer.c
drivers/drv_button.c
drivers/drv_ec11.c
drivers/drv_lcd1602.c
```

不使用的模块不加入工程，也不调用初始化函数。

这样可以做到：

- 未使用模块不占 ROM。
- 未使用模块不占 RAM。
- 未使用模块不占外设。
- 未使用模块不占中断。

## 4. 项目自有配置

每个应用项目应该自己提供板级配置。

推荐：

```text
MyProject/
  board/
    board_config.h
    board_pins.h
    board_init.c
```

基础库提供通用能力，应用项目决定：

- 芯片型号。
- 系统时钟。
- 引脚分配。
- 外设是否启用。
- UART 默认通道和波特率。
- LCD1602 I2C 地址。

`board_init.c` 不应提供“一次初始化全部外设”的隐藏入口。推荐按资源拆分为 `board_led_init()`、`board_i2c_init()`、`board_ec11_init()`、`board_adc_init()` 这类小函数，应用只调用实际使用的初始化函数。

## 5. 老 Keil C51 项目接入

老项目可以继续使用 Keil C51。

接入方式：

1. 复制或引用 `Stc8hBase` 源码。
2. 只把需要的 `.c` 文件加入 Keil 工程。
3. 配置 include 路径。
4. 选择对应的 STC8H 头文件。
5. 在项目自己的 `board_config.h` 中指定芯片和时钟。

老项目不需要一次性迁移到 SDCC。

如果某个老项目需要较大改造，可以考虑用新方式重写：

```text
SDCC + Makefile/PlatformIO + Stc8hBase
```

## 6. 新项目推荐方式

新项目优先推荐：

```text
macOS
  + VS Code / Cursor
  + SDCC
  + Makefile 或 PlatformIO
  + stcgal
```

Makefile 更透明，适合小项目和基础库验证。

PlatformIO 项目管理更完整，适合多个示例、多个板级配置和自动化构建。

第一版可以同时保留 Makefile 示例和 PlatformIO 示例，但基础库源码不依赖任何一种工程系统。

## 7. 当前默认习惯

当前设计按以下用户习惯优先：

- 老项目：Keil C51。
- 新项目：接受 SDCC + Makefile/PlatformIO。
- 主芯片：`STC8H1K08`。
- 常用封装：`TSSOP20`。
- 默认系统时钟：11.0592MHz。
- 默认 UART 波特率：115200。
- 默认 I2C LCD1602 地址：0x27。

11.0592MHz 适合传统串口波特率计算，尤其是 9600、19200、115200 等常见波特率，因此作为第一版默认时钟。

12MHz 延时和定时计算更直观，保留为可选配置。

第一版示例默认按 11.0592MHz 组织，同时保留 12MHz 配置选项。

6MHz 可用于低功耗调试配置。UART1 使用 Timer1 1T 波特率发生器时，`STC8H_UART1_BAUD` 可选 9600、19200、38400、57600、115200；这些配置的理论误差约 +0.1603%。

## 8. Timer0 free-run 最小用法

红外接收这类边沿脉宽测量可以使用 Timer0 12T 16-bit free-run：

```c
#include "stc8h_timer.h"

static stc8h_u16 last_ticks;

void board_timer_init(void)
{
    (void)stc8h_timer0_init_free_run_12t();
    last_ticks = stc8h_timer0_read();
    stc8h_timer_start(STC8H_TIMER0);
}

void on_edge(void)
{
    stc8h_u16 now_ticks;
    stc8h_u16 width_ticks;
    stc8h_u16 width_us;

    now_ticks = stc8h_timer0_read();
    width_ticks = (stc8h_u16)(now_ticks - last_ticks);
    width_us = stc8h_timer0_12t_ticks_to_us(width_ticks);
    last_ticks = now_ticks;
}
```

12T free-run 的换算公式是：

```text
us = ticks * 12 * 1000000 / STC8H_SYSCLK_HZ
```

在 6MHz 下 1 tick = 2us；在 12MHz 下 1 tick = 1us；在 11.0592MHz 下约 1.085us。示例 `examples/platformio/ir_nec_rx` 和 `examples/platformio/ir_nec_rx_int_sleep` 已使用该 HAL，不再在示例业务层复制 Timer0 寄存器配置。

## 9. Timer0 1T 微秒级阻塞延时

NEC 红外发射这类协议时序不要使用粗略 C 空循环。需要 562us、1687us、2250us、4500us、9000us 这类码元时，使用 Timer0 1T 延时：

```c
#include "stc8h_delay.h"

void board_ir_tx_init(void)
{
    (void)stc8h_delay_timer0_1t_init();
}

void send_mark(void)
{
    /* 打开 38kHz 载波 */
    stc8h_delay_timer0_1t_us(562u);
    /* 关闭 38kHz 载波 */
}
```

该接口会占用 Timer0，并把 Timer0 配置为 1T 16-bit non-auto-reload，不启用中断。已经使用 Timer0 free-run 接收红外、1ms tick 或其他计时功能的项目，不能同时使用这个阻塞延时；需要在板级资源表中明确二选一。
