# 测试计划

本文件只定义测试分层和准入标准，不记录调试过程、逐条硬件日志或一次性验证脚本。

## 目标

- 确认基础库源码可被 SDCC/PlatformIO 和 Makefile 构建。
- 确认代表性 host 单元快检通过。
- 确认示例只编译实际使用的 `.c` 文件，未使用模块零占用。
- 确认高风险能力有发布前专项检查，例如 EEPROM/IAP、OTA 链接布局、nRF24 尺寸边界。

测试不能把项目变成流程框架。新增测试必须服务于真实缺陷风险、资源边界或公共 API 行为。

## 分层入口

日常快检：

```sh
tools/check_host_tests.sh
tools/check_examples.sh
```

发布前全量检查：

```sh
tools/check_host_tests_full.sh
tools/check_examples_full.sh
```

专项入口：

```sh
tools/check_nrf24_examples.sh
tools/prepare_h8k64u_validation.sh
```

`tools/prepare_h8k64u_validation.sh` 只构建 H8K64U 示例并输出人工硬件验证命令模板，不自动上传。

## Host 单元测试

Host 测试只覆盖可在宿主机可靠判断的纯逻辑：

- 小工具算法，例如 CRC。
- 协议编解码，例如 OTA frame、OTA manifest、RF link 包。
- 驱动状态机或轻量逻辑，例如 EC11、RS485 UART wrapper、nRF24 核心错误路径。

Host 测试不模拟完整 MCU、寄存器时序或硬件物理现象。无法稳定模拟的内容放到示例构建或硬件验证。

`tools/check_host_tests.sh` 只保留日常快检集合：小工具、基础协议编解码和少量轻量驱动逻辑。较重的 OTA 状态机、nRF24 ACK/dynamic payload 路径、RF-link 地址空间路径放在 `tools/check_host_tests_full.sh`。

## 示例构建

示例分为三类：

- 最小功能示例：GPIO、UART、I2C、SPI、PWM、ADC、EEPROM、WDT、CRC、filter、button、EC11、TM1637、IR。
- 目标芯片示例：`STC8H1K08` 和 `STC8H8K64U-45I-LQFP48` 的代表性构建。
- 专项示例：nRF24、RF link、OTA bootloader、H8K64U IAP/OTA 参数路径。

新增示例必须满足：

- 入口明确，文件少，默认不执行破坏性写擦。
- PlatformIO/Makefile 只编译该示例实际使用的源文件。
- 不把一次性排查过程、串口日志解析或硬件矩阵测试固化成默认示例。

## 发布前专项

发布前才运行的检查可以覆盖：

- 全部 PlatformIO 示例构建。
- Makefile 示例构建。
- 重型 OTA/nRF24/RF-link host 行为测试。
- EEPROM/IAP 写擦环境的显式构建。
- OTA app `0x0200` 链接、bootloader `0xB400` 链接和 `0xFC00..0xFFFF` 参数区边界。
- 少量高风险 IAP/地址空间编译检查。

专项检查不能替代代码设计。若一个功能必须靠大量脚本约束才不出错，优先简化实现或缩小功能范围。

## 硬件验证

硬件验证清单见 `docs/16_HARDWARE_TEST.md`。本文件不保存硬件日志。

高影响操作必须人工确认：

- EEPROM/IAP 写擦。
- OTA 参数区写擦。
- bootloader 烧录和跳转。
- 看门狗复位。
- 修改 STC code/EEPROM split。

硬件验证完成后，只记录稳定结论、日期、板卡/芯片型号、示例或环境名，以及对后续设计有影响的事实。

## 准入规则

新增测试或脚本必须满足至少一条：

- 覆盖公共 API 行为。
- 覆盖资源受限 MCU 上的尺寸、链接或地址空间风险。
- 覆盖曾经真实发生且可能复发的缺陷。
- 覆盖破坏性硬件操作的安全边界。

以下内容不进入长期测试体系：

- 一次性调试脚本。
- 大量重复命令输出。
- 只证明旧过程正确的流水账。
- 与当前正式目标芯片无关的矩阵探索。
- 为过度抽象或过度配置服务的测试。

## 记录规则

`docs/RESOURCE_REPORT.md` 只记录稳定资源结论。测试计划只记录入口和准入标准。具体模块的设计原因放在对应模块文档，不在测试计划里堆清单。
