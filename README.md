# Stc8hBase

STC8H 单片机项目可复用基础库。

## 1. 目标

Stc8hBase 用来沉淀 STC8H 项目中反复出现的基础能力和外设驱动，让具体应用项目可以直接复用稳定代码，而不是每次重新复制、改写一套相似实现。

这个库面向资源受限的 8051 类 MCU 项目。运行效率、代码体积、RAM 占用和行为可预期性优先级高于“大而全”的通用抽象。

## 2. 设计优先级

1. 运行效率高。
2. ROM/RAM 占用低。
3. API 简单，便于内联、检查和移植。
4. 配置显式，尽量减少隐藏行为。
5. 模块可复用，但不包含具体应用业务逻辑。
6. 方便扩展新的基础功能和外设驱动。
7. 实际使用时，未使用的模块不应占用任何 ROM、RAM、外设或中断资源。

第一版支持范围表示基础库能力集合，不表示每个应用项目都要启用全部模块。应用项目只编译和初始化实际使用的模块。

## 3. 范围

基础库应该包含：

- 芯片基础支持：通用类型、编译器兼容、时钟相关辅助、延时辅助、中断辅助。
- HAL 模块：GPIO、定时器、UART、软件 I2C、SPI 基础收发、PWM、ADC、EEPROM/IAP、看门狗、外部中断和低功耗入口。
- 外设驱动：按钮、EC11 编码器、I2C LCD1602、LED、蜂鸣器、继电器、TM1637、红外遥控发射/接收。
- 工具模块：环形缓冲、软件定时器、CRC/校验、简单滤波。
- 每个稳定模块对应一个最小示例。

基础库不应该包含：

- 产品专用 UI 流程。
- 应用菜单逻辑。
- 业务状态机。
- 大型通用框架。
- 为旧项目结构保留的不合理兼容层。

## 4. 建议目录结构

```text
Stc8hBase/
  core/        MCU 和编译器基础能力
  hal/         薄硬件抽象模块
  drivers/     可复用外设驱动
  utils/       小型通用工具
  examples/    最小可编译示例
  docs/        设计和使用文档
```

## 5. 当前状态

当前仓库已进入第一版功能实现后的收尾评审阶段。SDCC + PlatformIO/Makefile 示例已可全量构建；Keil C51 兼容入口已准备，但真实 Keil 编译仍需在 Windows + Keil C51 环境验证。

已实现并完成硬件实测的主要内容：

- core 编译器兼容、类型、配置、延时、中断辅助。
- GPIO。
- UART。
- 软件 I2C。
- I2C LCD1602。
- Button / EC11。
- LED / 有源蜂鸣器 / 继电器有效电平辅助已完成编译、资源检查和硬件实测。
- TM1637 数码管驱动已完成编译、资源检查和硬件实测。
- NEC 红外协议编码/解码层已完成编译和资源检查，真实硬件接收、发射、中断接收与休眠唤醒示例已完成实测。
- ADC。
- Timer0 1ms tick 和 12T 16-bit free-run 计时。
- PWMA 1..4 基础 PWM 输出已完成编译、资源检查和 P1.2 LED 呼吸实测。
- SPI 硬件主机轮询基础收发已完成编译、资源检查和回环实测。
- EEPROM/IAP 基础读写擦除已完成编译、资源检查和受控写擦实测；默认 PlatformIO 环境不执行写擦。
- WDT 看门狗最小封装、INT0/INT1 外部中断辅助、idle/power-down 低功耗入口已完成 SDCC 编译、资源检查和 WDT 复位实测。
- 16-bit soft timer 工具已完成编译和回绕测试。
- Ring buffer 工具已完成编译和回绕测试。
- CRC/checksum 工具已完成编译和标准向量测试。
- Filter 工具已完成编译和边界测试。

全量示例构建和关键符号检查：

```sh
tools/check_examples.sh
```

## 6. 文档索引

- [01 已确认决策](docs/01_DECISIONS.md)
- [02 设计原则](docs/02_DESIGN.md)
- [03 芯片支持范围](docs/03_CHIP_SUPPORT.md)
- [04 开发工具链](docs/04_TOOLCHAIN.md)
- [05 使用方式](docs/05_USAGE.md)
- [06 模块规划](docs/06_MODULES.md)
- [07 API 风格](docs/07_API_STYLE.md)
- [08 第一版 API 草案](docs/08_API_DRAFT.md)
- [09 第一版实现范围](docs/09_FIRST_VERSION_SCOPE.md)
- [10 参考项目与吸收原则](docs/10_REFERENCES.md)
- [11 测试计划](docs/11_TEST_PLAN.md)
- [12 测试板建议](docs/12_BOARD_DEMO.md)
- [13 资源占用策略](docs/13_RESOURCE_POLICY.md)
- [14 EEPROM/IAP 策略](docs/14_EEPROM_IAP_POLICY.md)
- [15 剩余工作](docs/15_REMAINING_WORK.md)
- [16 硬件测试清单](docs/16_HARDWARE_TEST.md)
- [19 新增模块准入标准](docs/19_MODULE_ACCEPTANCE.md)
- [20 应用项目引用模式](docs/20_APPLICATION_INTEGRATION.md)
- [资源报告](docs/RESOURCE_REPORT.md)
