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
docs/vendor/titan/TM1637_TitanMicro.pdf
```

用途：

- 确认 `STC8H1K08` 寄存器和外设行为。
- 确认 Flash、RAM、I/O、定时器、UART、SPI、I2C、PWM、ADC 等资源。
- 确认 IAP/EEPROM 写入规则。
- 校准官方示例中的初始化顺序和寄存器配置。
- UART1 默认实现按官方 STC8H 串口示例校准：Timer1 作为波特率发生器，`AUXR.S1ST2=0`，Timer1 1T，Timer1 mode0 16 位自动重装，`BRT = 65536 - FOSC / baud / 4`。
- UART1 6MHz 调试波特率按同一公式补表：9600 reload `0xFF64`、19200 `0xFFB2`、38400 `0xFFD9`、57600 `0xFFE6`、115200 `0xFFF3`，理论误差均约 +0.1603%。
- Timer0 1ms tick 按官方 STC8H Timer0 mode0 16-bit auto-reload 示例校准：11.0592MHz/12T/1ms reload 为 `0xFC66`，中断向量为 1。
- Timer0 free-run 按官方 STC8H Timer0 mode1 16-bit non-auto-reload 示例方向实现：`TMOD[1:0]=01`、12T、关闭时钟输出，调用方用 16-bit 回绕差值测量边沿间隔。
- I2C 当前板级配置按官方 STC8H GPIO 资料校准：开漏输出需要上拉；`P1PU/P3PU` 内部 4.1K 上拉和 `P1IE/P3IE` 数字输入使能均属于 XFR 扩展寄存器，访问前必须设置 `P_SW2.EAXFR=1`。
- ADC 当前实现按官方 STC8H ADC 资料校准：STC8H1K08 为 10-bit ADC，P3.3 对应 ADC11；ADC 引脚需配置为高阻输入；ADC 上电后等待约 1ms；`ADCTIM` 推荐 `0x3f`；10-bit ADC 速度不高于 500kHz。
- SPI 当前实现按官方 STC8H SPI 资料校准：`SPSTAT/SPCTL/SPDAT` 为普通 SFR，`P_SW1[3:2]` 选择引脚组，`SPIF/WCOL` 写 1 清除；第一版采用硬件 SPI 主机轮询，不启用中断和 DMA。
- EEPROM/IAP 当前实现按官方 STC8H 资料和官方库校准：STC8H1K08 为 4KB EEPROM/IAP，地址 `0x0000..0x0FFF`，512 字节扇区擦除，IAP 触发序列为 `0x5A`、`0xA5`，触发窗口临时关闭全局中断。
- PWM 当前实现按官方 STC8H PWM 资料和 `STC8H_PWM.c/.h` 校准：支持 `PWMA` 1..4 与 `PWMB` 5..8、PWM mode 1、P 输出和显式引脚组选择；不启用互补输出、死区、刹车、中断、捕获或同步。
- WDT 当前实现按官方 `WDT_CONTR` 位定义和官方库函数校准：`WDT_FLAG` 位于 bit7，`EN_WDT` 位于 bit5，`CLR_WDT` 位于 bit4，`IDLE_WDT` 位于 bit3，分频系数使用 bit2..0，喂狗通过置位 `CLR_WDT` 完成。
- 外部中断当前实现按 STC8H 官方手册和 STC8H1K08 资料校准：INT0(P3.2)/INT1(P3.3) 支持上升/下降沿模式和下降沿模式，`EX0/EX1` 控制使能，`IE0/IE1` 为中断标志；INT2(P3.6)/INT3(P3.7)/INT4(P3.0) 只支持下降沿，使能位为 `INTCLKO` bit4/5/6，请求标志为 `AUXINTIF` bit4/5/6，向量分别为 10/11/16。
- 低功耗当前实现按官方 `PCON` 定义和官方休眠示例校准：`PCON.IDL` 进入 idle，`PCON.PD` 进入 power-down；P3.2/INT0、P3.3/INT1 可作为 power-down 唤醒源。
- NEC 红外协议当前实现按 Infineon 官方应用笔记校准：普通 NEC 帧为 9ms 引导低有效脉冲、4.5ms 空闲、32bit LSB first 数据和结尾脉冲；重复码使用 9ms 引导低有效脉冲、约 2.25ms 空闲和 562us 结尾脉冲。协议层只输出或消费脉宽，不直接占用 PWM、Timer 或 GPIO。对 INT0 红外接收，基础库示例优先使用下降沿间隔解码：普通帧引导约 13.5ms，bit0 约 1.125ms，bit1 约 2.25ms，repeat 约 11.25ms；10ms..12.5ms 首间隔先作为候选 repeat，后续若跟随 bit 间隔则按完整 frame 优先解码。
- STC8G/STC8H 官方库函数包用于核对库函数层面的外设初始化顺序。已解压到临时目录分析，不把展开源码纳入仓库；仓库仅保留官方压缩包和吸收记录。
- PCF8574 资料用于核对 LCD1602 I2C 背包的 I/O 扩展器行为。
- HD44780 资料用于核对 LCD1602 初始化、命令和显示地址行为。
- Alps Alpine EC11E 资料用于核对 EC11 系列旋转编码器两相 A/B 输出、额定参数、定位数/脉冲数。用户手头 EC11 模块不一定是 Alps 原厂同型号，驱动只依赖通用两相增量输出和按键触点行为。
- TM1637 Titan Micro datasheet 权威镜像用于核对二线接口、显示寄存器地址 `0xC0..0xC5`、数据命令 `0x40`、显示控制命令 `0x88..0x8F` 和 ACK 时序。

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
- UART1 可按 `MAIN_Fosc / 4 / baud` 计算 1T 定时器重装值；官方库运行期计算，本库为节省 ROM 对常用 11.0592MHz 和 6MHz 调试波特率使用编译期表。
- ADC 官方库使用 `ADCTIM` 设置采样时序，`ADC_CONTR` 启动转换并轮询 `ADC_FLAG`，错误值返回 4096；本库对应使用 `ADCTIM=0x3f`、右对齐 10-bit 结果、超时返回 `STC8H_ADC_INVALID_VALUE`。
- EEPROM/IAP 官方库在触发 IAP 前关闭全局中断，并按 `0x5A`、`0xA5` 顺序写 `IAP_TRIG`；本库 `stc8h_eeprom` 沿用这个保护思路，并要求示例默认不执行写擦。
- 软件 I2C 官方示例每个相位都有短延时，ACK 通过释放 SDA 后读电平判断；本库继续保留板级宏绑定，并要求 SDA/SCL 释放后能读到高电平。
- TM1637 数据手册说明其二线接口不是标准 I2C，无从地址；本库 `drv_tm1637` 采用独立 bit-bang 时序，不复用 `stc8h_i2c`。
- PWM 官方库覆盖 PWMA/PWMB、互补输出、死区、刹车和中断；本库第一版只吸收基础输出所需寄存器顺序和位定义。
- 红外 NEC 协议层没有 STC 专属寄存器；真实硬件接收/发射层后续实现时再按 STC 官方资料核对外部中断、Timer 和 PWM 资源。
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
其他型号：第一版不承诺
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

## 6. nRF24L01 / RF24 资料

参考来源：

```text
https://devzone.nordicsemi.com/cfs-file/__key/communityserver-discussions-components-files/4/nRF24L01P_5F00_PS_5F00_v1.0.pdf
https://nrf24.github.io/RF24/classRF24.html
https://nrf24.github.io/RF24/md_COMMON__ISSUES.html
https://docs.circuitpython.org/projects/nrf24l01/en/latest/
```

用途：

- 核对 nRF24L01+ SPI 命令、寄存器、FIFO、IRQ 和 TX/RX 状态机。
- 核对 ACK payload 与 dynamic payload 的依赖关系。
- 核对 `TX_DS`、`RX_DR`、`MAX_RT`、`FIFO_STATUS` 和 `OBSERVE_TX` 的恢复策略。
- 记录常见硬件问题：供电噪声、PA/LNA 电流、线长、SPI 速度和 2.4GHz 穿透限制。

吸收结论：

- `drv_nrf24l01` 只做芯片驱动，不继承旧项目 API 或业务协议。
- ACK payload 只作为短状态回传优化，启用时必须同时启用 dynamic payload。
- PRX 的 ACK payload 会占用 TX FIFO，堵塞时需要 `FLUSH_TX`。
- ISR 只置位，不在 ISR 中执行 SPI 收发。
