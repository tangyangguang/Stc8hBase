# 参考项目与吸收原则

## 1. 目的

本库坚持自己实现，优先满足自己的项目习惯和使用效率。

网上成熟库只作为参考，用来完善建设要求、校准工具链方案和避免重复踩坑。

不直接照搬外部库的整体架构。

## 2. 资料使用原则

- 涉及芯片、寄存器、特殊功能寄存器、外设协议、电气特性、时序和下载工具行为时，必须先查官方资料。
- 找不到官方资料时，必须使用权威资料，例如芯片厂家资料、器件 datasheet、主流工具官方文档或可交叉验证的成熟开源实现，并在本文档记录来源。
- 编码和调试时不凭经验猜寄存器位、地址、引脚复用、时序公式和电气约束。
- 如果实测现象与当前实现冲突，先回到资料核对硬件事实，再修改代码。
- 高频使用且体积合理的资料可以存入 `docs/vendor/`；低频、体积大或容易过期的资料只记录链接。
- 本地资料需要记录来源、用途、下载日期和校验值。
- `docs/vendor/README.md` 是本地资料总索引，新增本地资料必须同步更新该文件。
- 不把 STC-ISP、IDE 安装包、示例压缩包、无关原理图等大文件放入仓库，除非它们成为实现或验收的必要依据。

## 3. 主要参考

### 3.1 FwLib_STC8

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

### 3.2 STC 官方资料

参考来源：

```text
https://www.stcmicro.com/cn/stc/stc8h1k08.html
docs/vendor/stc/STC8H1K08_Features.pdf
docs/vendor/stc/STC8H-en.pdf
docs/vendor/stc/stc8g-stc8h-lib-demo-code.rar
docs/vendor/ti/PCF8574_TI.pdf
docs/vendor/lcd/HD44780_Hitachi_via_Adafruit.pdf
docs/vendor/encoder/ALPS_Alpine_EC11E_Series.pdf
```

用途：

- 确认 `STC8H1K08` 寄存器和外设行为。
- 确认 Flash、RAM、I/O、定时器、UART、SPI、I2C、PWM、ADC 等资源。
- 确认 IAP/EEPROM 写入规则。
- 校准官方示例中的初始化顺序和寄存器配置。
- UART1 默认实现按官方 STC8H 串口示例校准：Timer1 作为波特率发生器，`AUXR.S1ST2=0`，Timer1 1T，Timer1 mode0 16 位自动重装，`BRT = 65536 - FOSC / baud / 4`。
- Timer0 当前实现按官方 STC8H Timer0 mode0 16-bit auto-reload 示例校准：11.0592MHz/12T/1ms reload 为 `0xFC66`，中断向量为 1。
- I2C 当前板级配置按官方 STC8H GPIO 资料校准：开漏输出需要上拉；`P1PU/P3PU` 内部 4.1K 上拉和 `P1IE/P3IE` 数字输入使能均属于 XFR 扩展寄存器，访问前必须设置 `P_SW2.EAXFR=1`。
- ADC 当前实现按官方 STC8H ADC 资料校准：STC8H1K08 为 10-bit ADC，P3.3 对应 ADC11；ADC 引脚需配置为高阻输入；ADC 上电后等待约 1ms；`ADCTIM` 推荐 `0x3f`；10-bit ADC 速度不高于 500kHz。
- STC8G/STC8H 官方库函数包用于核对库函数层面的外设初始化顺序。已解压到临时目录分析，不把展开源码纳入仓库；仓库仅保留官方压缩包和吸收记录。
- PCF8574 资料用于核对 LCD1602 I2C 背包的 I/O 扩展器行为。
- HD44780 资料用于核对 LCD1602 初始化、命令和显示地址行为。
- Alps Alpine EC11E 资料用于核对 EC11 系列旋转编码器两相 A/B 输出、额定参数、定位数/脉冲数。用户手头 EC11 模块不一定是 Alps 原厂同型号，驱动只依赖通用两相增量输出和按键触点行为。

原则：

- 官方资料作为硬件事实来源。
- 官方示例可以参考，但不直接决定本库 API。
- 当本库实现与官方寄存器示例冲突时，先修正寄存器事实，再重新评估 API。
- 已把必要官方 PDF 归档到 `docs/vendor/stc/`，用于离线复核；实现前仍应留意官网是否有新版。

### 3.3 PlatformIO STC8H1K08 支持

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

### 3.4 按键和 EC11 行为参考

参考来源：

```text
https://docs.qmk.fm/features/encoders
https://docs.qmk.fm/feature_debounce_type
https://docs.splitkb.com/troubleshooting/encoder-skipping
https://main.rmk.rs/guide/input_devices/encoder
docs/vendor/stc/STC8H-en.pdf
```

用途：

- 校准 EC11 方向、每定位格跳变数、快速旋转加速和按钮消抖的设计边界。
- 核对 STC8H 官方硬件正交编码器模式，判断是否适合本项目第一版 EC11 UI 输入。

吸收结论：

- EC11 的 A/B 相旋转和 SW 按压是两个独立输入能力。`drv_ec11` 只处理 A/B 相方向和增量，SW 通过 `drv_button` 组合实现短按、长按和释放事件。
- EC11 旋转漏格的常见调节点不是按钮消抖，而是每定位格有效跳变数。QMK 使用 `ENCODER_RESOLUTION`，RMK 使用 `resolution`；本库对应为 `DRV_EC11_STEPS_PER_DETENT` 和 `drv_ec11_set_steps_per_detent`。
- 方向反转应可配置。QMK 使用方向翻转宏，本库对应为 `DRV_EC11_REVERSE` 和 `drv_ec11_set_reverse`。
- 按钮消抖应是时间参数，默认 10ms，且允许项目宏和运行时 API 覆盖。长按阈值默认 800ms。
- 示例程序不能逐格阻塞打印，否则快速旋转会因为 UART 阻塞导致漏扫。`ec11_counter` 采用 1ms 扫描、100ms 汇总打印。
- STC8H 官方手册提供 PWMA/TIM1 硬件正交编码器模式，适合高速编码器或电机类输入；第一版 EC11 人机输入继续采用轻量轮询状态机，避免占用高级定时器/PWM 资源。

### 3.5 STC8G/STC8H 官方库函数包

参考来源：

```text
https://www.stcmicro.com/slcx.html
https://www.stcmicro.com/rar/demo/stc8g-stc8h-lib-demo-code.rar
docs/vendor/stc/stc8g-stc8h-lib-demo-code.rar
```

包内主要内容：

- `库函数/`：GPIO、Timer、UART、ADC、EEPROM、SPI、硬件 I2C、软件 I2C、PWM、PCA、比较器、DMA、RTC、WDT 等库函数。
- `独立程序/`：GPIO 跑马灯、Timer、外部中断、UART、ADC、EEPROM、PWM、SPI、I2C、RTC、DMA 等独立例程。
- `STC8G-8H库函数使用说明-20240429.pdf`：官方库函数说明文档，已随压缩包保存。

已吸收的实现事实：

- GPIO 模式位与本库一致：准双向 `M1=0/M0=0`，高阻输入 `M1=1/M0=0`，开漏 `M1=1/M0=1`，推挽 `M1=0/M0=1`。
- UART1 可按 `MAIN_Fosc / 4 / baud` 计算 1T 定时器重装值；官方库运行期计算，本库为节省 ROM 对常用 11.0592MHz/115200 等配置使用编译期表。
- ADC 官方库使用 `ADCTIM` 设置采样时序，`ADC_CONTR` 启动转换并轮询 `ADC_FLAG`，错误值返回 4096；本库对应使用 `ADCTIM=0x3f`、右对齐 10-bit 结果、超时返回 `STC8H_ADC_INVALID_VALUE`。
- EEPROM/IAP 官方库在触发 IAP 前关闭全局中断，并按 `0x5A`、`0xA5` 顺序写 `IAP_TRIG`；后续实现 EEPROM 模块必须沿用这个保护思路，并遵守本项目 EEPROM/IAP 策略。
- 软件 I2C 官方示例每个相位都有短延时，ACK 通过释放 SDA 后读电平判断；本库继续保留板级宏绑定，并要求 SDA/SCL 释放后能读到高电平。
- 硬件 I2C、SPI、PWM、DMA 的官方库可作为后续模块寄存器顺序参考，但本库第一版仍只实现实际用到且验证过的最小能力。

不直接采用的点：

- 官方库为通用实验箱/多型号/多外设设计，结构体配置和运行期分派较多；本库继续坚持按需编译、低占用、板级显式绑定。
- 官方库包含大量全局缓冲区和中断模式示例；本库只有在应用确实需要时才引入 ring buffer 或中断驱动。
- 官方独立示例面向 Keil 工程组织；本库保留 SDCC/PlatformIO/Makefile 透明构建入口，同时保持 Keil C51 源码兼容目标。

## 4. 吸收为本库建设要求

### 4.1 编译器差异必须集中隔离

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

### 4.2 Makefile 是基础验证入口

第一版必须提供 Makefile 示例或构建入口。

原因：

- 编译了哪些 `.c` 文件更透明。
- 更容易验证未使用模块零占用。
- 不绑定 IDE。
- 适合 macOS 命令行开发。

### 4.3 PlatformIO 是辅助工程入口

第一版可以提供 PlatformIO 示例。

要求：

- `platformio.ini` 中明确芯片、频率和编译宏。
- 示例只加入实际用到的模块。
- 不让基础库源码依赖 PlatformIO 目录结构。

### 4.4 Keil C51 接入必须保留

老项目仍然以 Keil C51 为主。

本库必须支持：

- Keil 工程按需添加 `.c/.h`。
- 通过预处理宏切换 Keil 关键字。
- 不要求老项目迁移到 SDCC 才能使用。

### 4.5 配置参数集中化

芯片、频率、时钟分频、UART 默认波特率、模块开关等配置必须集中。

不允许每个模块各自散落重复宏。

推荐入口：

```text
core/stc8h_config.h
board/board_config.h
```

### 4.6 VS Code/SDCC 语法兼容要有说明

SDCC 的 8051 语法可能导致 VS Code C/C++ 插件误报。

文档中需要说明：

- 如何配置 include path。
- 如何添加语法兼容宏。
- 编辑器误报不等于编译失败。

### 4.7 不扩大芯片范围

参考库支持更多 STC8 型号，但本库不跟随扩大范围。

本库继续保持：

```text
正式主支持：STC8H1K08
可评估兼容：STC8H1K17
```

其他型号只有在真实项目需要时再加入。

## 5. 参考库评估结论

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
