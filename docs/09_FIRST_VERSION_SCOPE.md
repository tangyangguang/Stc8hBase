# 第一版实现范围

## 1. 原则

第一版不做“以后再实现”的功能规划，但采用里程碑交付。

当前已确认且符合高效率、低占用、可复用原则的基础能力和外设支持，纳入第一版能力集合。

以后只有遇到新的真实外设或新功能需求时，才新增模块。

第一版主支持芯片为 `STC8H1K08`。其他芯片只在不增加代码复杂度、不增加主支持芯片资源占用的前提下兼容。

第一版可以参考外部成熟库的工具链和兼容层设计，但实现、API 和模块边界按本库目标重新设计。

第一版范围表示“基础库提供的能力集合”，不表示每个应用项目都要启用全部模块。

具体应用项目只编译和初始化实际用到的模块。未使用模块必须保持零 ROM、零 RAM、零外设、零中断占用。

里程碑 1 完成并通过资源报告后，即可作为可用基础库发布。后续里程碑按硬件验证成熟度继续补齐第一版能力集合，未验证成熟的模块不得标记为稳定。

## 2. 实现里程碑

第一版支持范围不拆分为未来版本，但交付过程按里程碑推进，避免一次性铺开导致模块半成品。

里程碑 1：

- `core/stc8h_compiler`
- `core/stc8h_types`
- `core/stc8h_config`
- `core/stc8h_delay`
- `core/stc8h_interrupt`
- `hal/stc8h_gpio`
- `hal/stc8h_uart`
- `hal/stc8h_i2c`
- `drivers/drv_lcd1602`

里程碑 2：

- `hal/stc8h_timer`
- `utils/util_soft_timer`
- `drivers/drv_button`
- `drivers/drv_ec11`
- `drivers/drv_led`
- `drivers/drv_buzzer`
- `drivers/drv_relay`

里程碑 3：

- `hal/stc8h_pwm`
- `drivers/drv_ir_tx`
- `drivers/drv_ir_rx`
- `drivers/drv_tm1637`

里程碑 4：

- `hal/stc8h_spi`
- `hal/stc8h_adc`
- `hal/stc8h_eeprom`
- `utils/util_ring_buffer`
- `utils/util_crc`
- `utils/util_filter`

## 3. 第一版模块

Core：

- `core/stc8h_compiler`
- `core/stc8h_types`
- `core/stc8h_config`
- `core/stc8h_delay`
- `core/stc8h_interrupt`

HAL：

- `hal/stc8h_gpio`
- `hal/stc8h_timer`
- `hal/stc8h_uart`
- `hal/stc8h_i2c`
- `hal/stc8h_spi`
- `hal/stc8h_pwm`
- `hal/stc8h_adc`
- `hal/stc8h_eeprom`

Drivers：

- `drivers/drv_button`
- `drivers/drv_ec11`
- `drivers/drv_lcd1602`
- `drivers/drv_led`
- `drivers/drv_buzzer`
- `drivers/drv_relay`
- `drivers/drv_tm1637`
- `drivers/drv_ir_tx`
- `drivers/drv_ir_rx`

Utils：

- `utils/util_ring_buffer`
- `utils/util_soft_timer`
- `utils/util_crc`
- `utils/util_filter`

## 4. 验收口径

### 4.1 里程碑验收

每个里程碑完成后，需要满足：

- 该里程碑内模块源码完成。
- 对应最小示例通过 SDCC + Makefile 编译。
- 资源占用记录完成。
- 必要硬件验证完成。
- 未使用模块不进入链接产物。

里程碑 1 通过后，可以发布为“基础可用版”。

### 4.2 第一版能力集合验收

第一版能力集合满足以下示例能在真实或仿真的 STC8H 环境中编译和运行，即视为完整达标：

- GPIO 闪灯。
- UART 回显。
- I2C 基础读写。
- SPI 基础收发。
- PWM 输出。
- ADC 采样。
- EEPROM 参数读写。
- 按键短按/长按事件打印。
- EC11 旋转计数打印。
- I2C LCD1602 文本输出。
- LED 闪烁。
- 蜂鸣器控制。
- 继电器控制。
- TM1637 数码管显示。
- 红外 NEC 协议发射。
- 红外 NEC 协议接收解码。
- I2C LCD1602 + 按键 + EC11 组合示例。

同时需要验证：只编译其中一个示例时，未加入工程的模块不会产生 ROM/RAM 占用，也不会占用未使用的外设或中断资源。

## 5. 实现顺序

实现顺序以第 2 节里程碑为准。

每个里程碑必须完成：

1. 模块源码。
2. 最小示例。
3. SDCC + Makefile 编译验证。
4. 资源占用记录。
5. 必要的硬件验证。

## 6. 实现前核对项

以下事项不改变设计方向，但实现前需要核对：

- STC8H 厂商头文件来源。
- 是否需要在第一版中实际验证 `STC8H1K17`。
- 默认 UART 通道。
- UART1/UART2 在 `11.0592MHz` 下实现 `115200` 的配置方式。
- UART 是否占用 Timer，或是否使用独立波特率发生器。
- 如果 `115200` 无法稳定通过，默认降级到 `9600` 还是编译时报错。
- STC8H1K08 TSSOP20 测试板具体引脚分配。
- SPI 第一版已冻结为硬件 SPI 主机轮询实现，不维护软件 SPI 双实现。
- 红外发射 PWM 引脚是否适合测试板。
