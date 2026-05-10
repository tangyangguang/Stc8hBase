# 参考项目与吸收原则

## 1. 目的

本库坚持自己实现，优先满足自己的项目习惯和使用效率。

网上成熟库只作为参考，用来完善建设要求、校准工具链方案和避免重复踩坑。

不直接照搬外部库的整体架构。

## 2. 主要参考

### 2.1 FwLib_STC8

项目地址：

```text
https://github.com/IOsetting/FwLib_STC8
```

可借鉴点：

- 支持 STC8G/STC8H 系列。
- 同时兼容 SDCC 和 Keil C51。
- 使用宏隔离编译器关键字差异。
- 使用 Makefile 作为 SDCC 主要构建入口。
- 提供 Keil C51 接入方式。
- 提供 STC8H1K08 相关配置示例。
- 明确面向受限 8 位 MCU 资源。

不直接采用的点：

- 本库不做通用 STC8G/STC8H 全系列库。
- 本库主目标是 `STC8H1K08`。
- 本库需要包含常用项目级外设驱动，如按钮、EC11、I2C LCD1602、TM1637。
- 本库 API 和目录按自己的项目习惯设计。

### 2.2 STC 官方资料

参考来源：

```text
https://www.stcmicro.com/cn/stc/stc8h1k08.html
```

用途：

- 确认 `STC8H1K08` 寄存器和外设行为。
- 确认 Flash、RAM、I/O、定时器、UART、SPI、I2C、PWM、ADC 等资源。
- 确认 IAP/EEPROM 写入规则。
- 校准官方示例中的初始化顺序和寄存器配置。

原则：

- 官方资料作为硬件事实来源。
- 官方示例可以参考，但不直接决定本库 API。

### 2.3 PlatformIO STC8H1K08 支持

参考来源：

```text
https://docs.platformio.org/en/latest/boards/intel_mcs51/STC8H1K08.html
```

用途：

- 作为新项目工程组织参考。
- 提供 `STC8H1K08` 示例工程入口。
- 验证 SDCC 构建参数和 `board_build.f_cpu` 配置。

原则：

- PlatformIO 可以作为示例工程入口。
- 基础库源码不依赖 PlatformIO。
- 如果 PlatformIO 和 Makefile 行为不一致，以 Makefile 的透明构建结果作为基础验证。

## 3. 吸收为本库建设要求

### 3.1 编译器差异必须集中隔离

本库必须提供 `core/stc8h_compiler.h`。

以下差异不得散落在业务模块、HAL 模块或外设驱动中：

- `bit`
- `sfr`
- `sbit`
- `data`
- `idata`
- `pdata`
- `code`
- `xdata`
- `interrupt`
- `using`
- `nop`
- xdata SFR 访问

所有这些都通过编译期宏处理，不能产生运行期开销。

### 3.2 Makefile 是基础验证入口

第一版必须提供 Makefile 示例或构建入口。

原因：

- 编译了哪些 `.c` 文件更透明。
- 更容易验证未使用模块零占用。
- 不绑定 IDE。
- 适合 macOS 命令行开发。

### 3.3 PlatformIO 是辅助工程入口

第一版可以提供 PlatformIO 示例。

要求：

- `platformio.ini` 中明确芯片、频率和编译宏。
- 示例只加入实际用到的模块。
- 不让基础库源码依赖 PlatformIO 目录结构。

### 3.4 Keil C51 接入必须保留

老项目仍然以 Keil C51 为主。

本库必须支持：

- Keil 工程按需添加 `.c/.h`。
- 通过预处理宏切换 Keil 关键字。
- 不要求老项目迁移到 SDCC 才能使用。

### 3.5 配置参数集中化

芯片、频率、时钟分频、UART 默认波特率、模块开关等配置必须集中。

不允许每个模块各自散落重复宏。

推荐入口：

```text
core/stc8h_config.h
board/board_config.h
```

### 3.6 VS Code/SDCC 语法兼容要有说明

SDCC 的 8051 语法可能导致 VS Code C/C++ 插件误报。

文档中需要说明：

- 如何配置 include path。
- 如何添加语法兼容宏。
- 编辑器误报不等于编译失败。

### 3.7 不扩大芯片范围

参考库支持更多 STC8 型号，但本库不跟随扩大范围。

本库继续保持：

```text
正式主支持：STC8H1K08
可评估兼容：STC8H1K17
```

其他型号只有在真实项目需要时再加入。

## 4. 参考库评估结论

本库继续自己实现。

推荐吸收：

- 编译器兼容层思路。
- Makefile 优先验证思路。
- Keil/SDCC 双兼容要求。
- PlatformIO 作为辅助入口。
- 配置集中化。

不推荐吸收：

- 全系列通用适配目标。
- 总入口式大 HAL。
- 为了泛化增加函数指针表或运行期适配。
- 与本项目常用外设无关的大量模块。
