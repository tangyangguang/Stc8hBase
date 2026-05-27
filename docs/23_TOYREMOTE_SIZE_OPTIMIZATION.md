# ToyRemote 固件尺寸优化设计

## 目标

相邻项目 `Stc8hToyRemote` 在 STC8H1K08 8KB flash / 1.25KB RAM 上余量很小。本轮优化只面向已确认的固定使用路径，不改变业务功能、无线 payload 格式或基础库默认 API 行为。

## 依据

- STC8H1K08 的 PWM、ADC、EEPROM/IAP 寄存器和约束沿用 `docs/10_REFERENCES.md` 已记录的 STC 官方资料结论。
- nRF24L01+ 的 FEATURE、DYNPD、ACK payload、fixed payload 规则沿用 `docs/10_REFERENCES.md` 与 `docs/21_NRF24L01_DESIGN.md` 已记录的 Nordic nRF24L01+ datasheet 结论。
- ToyRemote 当前 controller 仅使用 fixed payload send，receiver 仅使用 fixed payload poll；receiver PWM 仅使用 PWMA channel 1 与 PWMB channel 6/7/8。

## 方案

1. PWM 保持通用 API 默认开启，新增 fixed-channel API 与 fixed-only 编译开关。ToyRemote receiver 用固定函数替换通用 `set_prescaler/set_period/init_channel/set_duty/enable` 调用，避免 group/channel/pin 分派代码进入固件。
2. `proto_rf_link_send_data_fixed` 新增 fixed fast path。该路径仍写出相同 32-byte packet 格式：固定 header、固定 payload 长度、未使用尾部清零，但不再调用通用 packet builder。
3. `drv_nrf24l01` 增加 fixed payload read/write API 和可选 FEATURE 快速启用宏。fixed payload API 不改变 SPI 命令或 payload 内容；FEATURE 快速启用会跳过 readback/ACTIVATE fallback，默认关闭，只能在应用确认 nRF24L01+ 硬件且接受较弱启动检测时启用。
4. `stc8h_eeprom` 固定块读写保留空指针、IAP 时钟和 IAP 命令失败返回。当前 ToyRemote 已关闭通用 read/write/erase，固定块实现已经只保留必要固定地址/长度路径；继续减少状态变量会弱化 IAP 失败返回或收益很小，本轮不额外改 EEPROM 持久化路径。
5. `stc8h_adc` 增加关闭通道合法性检查的编译开关。默认保留检查；ToyRemote 只读固定 ADC channel，可选择关闭以节省 ROM，但这会降低错误检测能力。

## 风险和默认值

- 所有新增裁剪开关默认保持现有安全行为。
- 跳过 nRF24 FEATURE readback/ACTIVATE fallback 会影响旧 nRF24 非 plus 兼容性和 FEATURE 写入错误检测，作为应用显式 opt-in，不在基础库默认启用。
- 关闭 ADC channel 检查会让非法 channel 不再返回 `STC8H_ADC_INVALID_VALUE`，作为应用显式 opt-in，不在基础库默认启用。
- PWM fixed-only API 假设调用方传入的 group/channel/pin 已由板级代码固定；ToyRemote receiver 的 pin 与 channel 来自 `app_outputs.c` 的固定初始化。

## 验证

- 在基础库运行 host/编译期裁剪检查，确认被关闭的通用 helper 不再出现在符号表。
- 在 `Stc8hToyRemote` 运行 `sh tools/check_all.sh`。
- 报告 controller/receiver 的 flash、stack、largest spare internal RAM 前后对比。
