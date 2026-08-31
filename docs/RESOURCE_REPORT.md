# 资源报告

本文件只保留资源验证结论和入口，不再记录逐次构建日志、烧录流水账或插件计划过程。

## 当前结论

- 基础库按“只编译实际使用的 `.c` 文件”控制 ROM/RAM 占用。
- 日常快检入口为 `tools/check_examples.sh`，覆盖 host 单元快检、代表性 PlatformIO 示例和 Makefile 示例。
- 完整发布前验证入口为 `tools/check_examples_full.sh`，覆盖全部 PlatformIO 示例、重型 host 行为测试、少量高风险编译检查和 OTA 链接布局检查。
- EEPROM/IAP 写擦示例默认只做安全构建；真实写擦必须先确认测试地址和硬件影响。

## 最近验证

2026-08-31：

- `h8k64u_qei_pwmb_validate` 在 PlatformIO SDCC 4.4.0 下构建通过：894 bytes ROM。
- 示例只编译 `main.c`、`stc8h_gpio.c` 和 `stc8h_qei.c`；QEI HAL 没有固定全局 RAM、缓冲或中断向量，占用整个 PWMB 计数器及通道 5/6。
- 不引用 QEI 的代表性 `gpio_blink` map 保持无 `_stc8h_qei` 符号检查。
- `tools/prepare_h8k64u_validation.sh` 和 `tools/check_examples_full.sh` 通过。

2026-06-29：

- `tools/check_examples_full.sh` 通过。
- `tools/h8k64u_uart1_ota_smoke.py` 语法检查通过。

长期保留的示例和脚本只覆盖当前正式目标、稳定硬件验证入口或可重复构建检查。一次性 probe、matrix 诊断、临时 fault 注入和调试流水账不进入资源报告。

## 资源记录规则

新增或重构模块时，只记录稳定结论：

- 示例名称。
- 目标芯片。
- ROM/RAM 关键数字。
- 参与编译的核心 `.c` 文件。
- 使用的外设、中断、Timer、GPIO。
- 必须保留的布局或符号检查。

不要把完整命令输出、临时失败过程、插件计划步骤或硬件调试流水账写入本文件。
