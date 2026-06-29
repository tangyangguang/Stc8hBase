# 资源报告

本文件只保留资源验证结论和入口，不再记录逐次构建日志、烧录流水账或插件计划过程。

## 当前结论

- 基础库按“只编译实际使用的 `.c` 文件”控制 ROM/RAM 占用。
- 日常快检入口为 `tools/check_examples.sh`，覆盖 host 单元快检、代表性 PlatformIO 示例和 Makefile 示例。
- 完整发布前验证入口为 `tools/check_examples_full.sh`，覆盖全部 PlatformIO 示例、重型 host 行为测试、少量高风险编译检查和 OTA 链接布局检查。
- EEPROM/IAP 写擦示例默认只做安全构建；真实写擦必须先确认测试地址和硬件影响。

## 最近一次验证

2026-06-29：

- `tools/check_host_tests.sh` 通过。
- 原全量 `tools/check_examples.sh` 通过；清理后该重型入口改名为 `tools/check_examples_full.sh`。
- `docs/25_H8K64U_OTA_DESIGN.md` 已压缩为当前设计基线；一次性 `tools/upload_delay_probe.sh` 已删除。
- `docs/11_TEST_PLAN.md` 和 `docs/13_RESOURCE_POLICY.md` 已压缩为入口、准入和资源原则；一次性 UART1 OTA fault 脚本已删除。
- `nrf24_pair_diag` 自动 matrix 诊断示例和配套 host 测试已删除，保留 `nrf24_uart_diag`、`nrf24_fixed_ping`、`nrf24_ack_payload` 三个直接示例。
- `tools/check_host_tests.sh` 改为日常快检集合；OTA 状态机、nRF24 ACK/dynamic payload 和 RF-link 地址空间测试转入 `tools/check_host_tests_full.sh`。

## 资源记录规则

新增或重构模块时，只记录稳定结论：

- 示例名称。
- 目标芯片。
- ROM/RAM 关键数字。
- 参与编译的核心 `.c` 文件。
- 使用的外设、中断、Timer、GPIO。
- 必须保留的布局或符号检查。

不要把完整命令输出、临时失败过程、插件计划步骤或硬件调试流水账写入本文件。
