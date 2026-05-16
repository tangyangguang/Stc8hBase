# 资源占用策略

## 1. 目标

本文件定义第一版模块对 MCU 资源的占用原则。

目标：

- 未使用模块零占用。
- 已使用模块资源占用显式。
- 避免外设之间隐式抢资源。
- 优先保证 `STC8H1K08 TSSOP20` 小引脚项目可用。

## 2. 总原则

每个模块必须说明可能占用：

- GPIO。
- UART。
- Timer。
- PWM。
- I2C。
- SPI。
- ADC。
- 外部中断。
- RAM 缓冲区。

模块不得在未被应用项目编译和初始化时占用任何资源。

SDCC/8051 项目以“少编译源文件 + 编译期裁剪分支”为主要减小 ROM 的方式。通用 HAL 默认完整可用；小容量项目可通过显式宏限制端口、通道和 API 子集，但这些宏只裁剪芯片级能力，不承载应用业务逻辑。

业务可能不用的可裁 API 必须独立源文件实现，避免 SDCC/sdld 以 `.rel` 粒度拉入同文件中的未调用函数。典型例子：`stc8h_gpio_toggle()`、`stc8h_exti_disable()`、`stc8h_exti_clear_flags()`、`stc8h_power_idle()`。示例或应用只在实际调用这些 API 时编译对应小文件。

当前可裁剪项：

| 模块 | 宏 | 默认 | 作用 |
| --- | --- | --- | --- |
| GPIO | `STC8H_GPIO_PORT_MASK` | `0x3F` | 限制 P0..P5 的 switch 分支 |
| PWM | `STC8H_PWM_GROUP_MASK` | `0x03` | 限制 `PWMA/PWMB` 组分支 |
| PWM | `STC8H_PWM_A_CHANNEL_MASK` | `0x0F` | 限制 `PWMA` channel 1..4 分支 |
| PWM | `STC8H_PWM_B_CHANNEL_MASK` | `0x0F` | 限制 `PWMB` channel 5..8 分支，bit0 对应 channel 5 |
| PWM | `STC8H_PWM_ENABLE_DISABLE` | `1` | 是否编译 PWM disable API |
| nRF24L01 | `DRV_NRF24L01_ENABLE_RAW_API` | `1` | 是否公开原始 register/buffer/command API；关闭后仅作为内部 static helper |
| nRF24L01 | `DRV_NRF24L01_ENABLE_READ_FIFO_STATUS` | `1` | 是否编译 FIFO status 诊断 API |
| nRF24L01 | `DRV_NRF24L01_ENABLE_READ_OBSERVE_TX` | `1` | 是否编译 OBSERVE_TX 诊断 API |
| nRF24L01 | `DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD` | `1` | 是否编译独立 dynamic payload API |
| nRF24L01 | `DRV_NRF24L01_ENABLE_ACK_PAYLOAD` | `1` | 是否编译 ACK payload 启用 API |
| nRF24L01 | `DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD` | `ACK payload` | 是否编译写 ACK payload API |
| nRF24L01 | `DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD` | `ACK payload` | 是否编译关闭 ACK payload API |
| nRF24L01 | `DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE` | `Dynamic/ACK payload` | 是否编译读取动态 payload 长度 API |
| TM1637 | `DRV_TM1637_ENABLE_DISPLAY_DIGITS` | `1` | 是否编译数字数组格式化显示 API |
| TM1637 | `DRV_TM1637_ENABLE_DISPLAY_NUMBER` | `1` | 是否编译有符号整数格式化显示 API |
| TM1637 | `DRV_TM1637_ENABLE_ENCODE_DIGIT` | `1` | 是否编译单个数字段码转换 API |
| TM1637 | `DRV_TM1637_ENABLE_CLEAR` | `1` | 是否编译清屏便捷 API |
| EC11 | `DRV_EC11_ENABLE_SET_FAST` | `1` | 是否编译快速步进运行期配置 API |
| EC11 | `DRV_EC11_ENABLE_SET_REVERSE` | `1` | 是否编译反向运行期配置 API |
| EC11 | `DRV_EC11_ENABLE_SET_STEPS_PER_DETENT` | `1` | 是否编译每格步数运行期配置 API |
| Timer | `STC8H_TIMER_ENABLE_1MS` | `1` | 是否编译 Timer0 1ms 初始化 |
| Timer | `STC8H_TIMER_ENABLE_TIMER0_FREE_RUN` | `1` | 是否编译 Timer0 12T free-run/read/us 换算 |
| Timer | `STC8H_TIMER_ENABLE_TIMER0_RESET` | `1` | 是否编译 Timer0 reset |
| Timer | `STC8H_TIMER_ENABLE_RUN_CONTROL` | `1` | 是否编译 Timer start/stop |
| Timer | `STC8H_TIMER_ENABLE_INTERRUPT_CONTROL` | `1` | 是否编译 Timer 中断开关和清标志 |
| UART | `STC8H_UART_ASSUME_UART1` | `0` | 只使用 UART1 时省去 id 检查 |
| UART | `STC8H_UART_ENABLE_WRITE_RAM` | `1` | 是否编译 RAM 字符串输出 |
| UART | `STC8H_UART_ENABLE_RX` | `1` | 是否编译轮询接收 |
| IR RX | `DRV_IR_RX_ENABLE_PULSE` | `1` | 是否编译 NEC mark/space pulse 解码 |
| IR RX | `DRV_IR_RX_ENABLE_FALLING` | `1` | 是否编译 falling interval 解码 |

裁剪宏应放在板级配置或构建系统里，并随固件资源报告记录；基础库源码不应为某个应用硬编码固定端口或固定命令表。

## 3. 资源声明

每个驱动头文件或模块文档应写明资源需求。

示例：

```text
drv_ir_tx:
  必需：1 个 GPIO 输出
  推荐：1 路 PWM 或 1 个定时器产生 38kHz 载波
  RAM：无大型缓冲
```

## 4. 默认资源策略

### 4.1 UART

默认用于调试和示例输出。

默认波特率：

```text
115200
```

保留：

```text
9600
```

UART 接收缓冲只有在示例或应用显式启用 `util_ring_buffer` 时才占用 RAM。

### 4.2 Timer

Timer 是紧张资源，必须谨慎分配。

默认资源分配建议：

| 资源 | 默认用途 |
| --- | --- |
| Timer0 | 1ms 系统 tick、按键/EC11 扫描时间基准，或 12T free-run 红外接收脉宽计时 |
| Timer2 | 红外发射备用载波或后续项目备用 |

不允许每个模块私自占用一个 Timer。

Timer 资源由板级配置决定，驱动只声明需求。

UART 波特率相关资源由 UART 模块负责，是否占用 Timer 以 STC8H1K08 手册和具体实现为准，不在资源策略中预先绑定 Timer1。

如果某个项目不使用软件定时器或红外模块，对应 Timer 不应被初始化。

Timer ISR 默认由示例或板级文件绑定。Timer HAL 不默认占用中断向量。

Timer0 free-run 只初始化硬件计数器，不占用 Timer0 中断；调用方在 GPIO/外部中断边沿中读取 `stc8h_timer0_read()` 并使用 16-bit 回绕差值。

Timer0 1T 微秒级阻塞延时由 `stc8h_delay_timer0_1t_init()` 显式占用 Timer0，不启用中断，适合 NEC 红外发射这类协议时序。它不能和 Timer0 1ms tick、Timer0 free-run 红外接收同时使用。

需要显式声明 Timer0 角色时，可在构建系统定义：

- `STC8H_TIMER0_ROLE_FREE_RUN`：Timer0 由 `stc8h_timer0_init_free_run_12t()` 作为 12T free-run 使用。
- `STC8H_TIMER0_ROLE_1T_DELAY`：Timer0 由 `stc8h_delay_timer0_1t_init()` 作为 1T 阻塞延时使用。

如果同一固件同时编译冲突角色，基础库在编译期报错。使用 1T delay 且仍需编译 `hal/stc8h_timer.c` 的项目，应关闭 `STC8H_TIMER_ENABLE_TIMER0_FREE_RUN`，避免把 free-run API 暴露进同一固件。

真实应用中的分配示例：红外接收夜灯使用 Timer0 12T free-run 测量接收头边沿脉宽；红外遥控器发送端使用 Timer0 1T 阻塞延时产生 NEC `mark/space` 时间。二者都合理，但不能在同一个固件里同时初始化 Timer0 为这两种模式。

如果某示例提供 1ms 全局 tick，资源报告必须记录：

- 使用的 Timer。
- 使用的中断向量。
- tick 全局变量的存储区和大小。

### 4.3 PWM

PWM 优先用于：

- 红外发射 38kHz 载波。
- 无源蜂鸣器。

如果红外发射和无源蜂鸣器同时需要 PWM，应用项目必须明确选择资源分配。

有源蜂鸣器不占 PWM。

第一版默认红外发射优先使用 PWM 产生 38kHz 载波。

如果硬件 PWM 引脚不适合 `TSSOP20` 测试板，允许改用 Timer 产生载波，但必须在板级配置和资源报告中记录。

当前 `stc8h_pwm` 支持 `PWMA` channel 1..4 和 `PWMB` channel 5..8 的基础 PWM mode 1 输出。每组各有一个 ARR 周期和 prescaler，允许 `PWMA` 与 `PWMB` 同时使用不同频率；同一组内仍共享周期和 prescaler。

`stc8h_pwm_set_period()` 和 `stc8h_pwm_set_prescaler()` 在对应组已有输出使能且新值会改变运行参数时返回 `STC8H_ERROR`，防止运行中静默改掉同组其他通道频率。需要切换同组周期或 prescaler 时，应用必须先禁用同组输出，再重新设置周期、prescaler 和通道。

### 4.4 I2C

I2C 用于：

- I2C LCD1602。

第一版优先保证 I2C LCD1602 可用。

第一版默认软件 I2C 单总线，SDA/SCL 通过 `board_pins.h` 宏在编译期绑定。

不提供运行期 I2C bus 对象。

默认目标速率为标准模式 100kHz 以内，最终以示波器实测波形为准。

### 4.5 SPI

SPI 第一版提供基础能力。

SPI 第一版已冻结为硬件 SPI 主机轮询实现。

默认使用 P1.3/P1.4/P1.5 引脚组，硬件 SS 使用 `SSIG=1` 忽略，不占用当前 P1.2 LED。

片选由板级或应用代码自行控制，HAL 不保存 CS 引脚，不做运行期引脚分派。

第一版不同时维护硬件 SPI 和软件 SPI 两套实现。

### 4.6 ADC

ADC 示例只占用一个测试通道。

ADC 模块不应隐藏开启多个通道。

### 4.7 EEPROM/IAP

EEPROM/IAP 示例必须指定测试地址范围。

本项目不考虑老项目历史格式，不做旧数据迁移。

但示例不能静默擦写未知区域。

STC8H1K08 EEPROM/IAP 第一版按官方资料限定为 4KB，地址范围 `0x0000..0x0FFF`，512 字节扇区擦除。示例默认构建和上传不执行写擦；真实写擦测试必须显式选择写擦环境，并在资源报告中记录测试扇区。

EEPROM/IAP HAL 的 `read`、`write`、`erase_sector` 必须支持按 API 关闭编译。固定小配置块应优先使用 `STC8H_EEPROM_ENABLE_FIXED_BLOCK`，避免应用项目复制 IAP 实现或承担未使用 public API 的 ROM/RAM 成本。

EEPROM/IAP 触发窗口会临时关闭全局中断。写擦期间不得同时执行红外收发等严格时序任务。

### 4.8 红外接收

`drv_ir_rx` 是 NEC 解码状态机，不直接占用 GPIO、Timer 或中断。

`drv_ir_rx` 只保留 1 个未读事件，不分配队列。应用未调用 `drv_ir_rx_get_event()` 前，新事件会被丢弃，避免已解出的 `FRAME` 被后续 `REPEAT` 或 `ERROR` 覆盖。

真实硬件接收默认使用：

```text
VS1838B / HS0038
```

协议：

```text
NEC
```

资源需求：

- 1 个 GPIO 输入。
- 默认使用外部中断边沿触发 + Timer 脉宽计时。
- 小状态机 RAM。
- 不保存完整原始波形数组。

GPIO、中断和 Timer 属于板级捕获层资源。板级捕获层可以测得 `level + width_us` 后调用 `drv_ir_rx_feed_pulse()`；如果 INT0 双边沿捕获在目标硬件上不稳定，也可以只捕获 NEC 下降沿间隔并调用 `drv_ir_rx_feed_nec_falling_interval()`。

P3.2/INT0 红外低功耗示例默认使用下降沿中断 + Timer0 free-run：下降沿间隔约 13.5ms 为普通帧引导，约 1.125ms/2.25ms 为 bit0/bit1，约 11.25ms 为 repeat。10ms..12.5ms 首间隔不会立即发布 repeat；若后续出现 bit 间隔则优先解完整 frame，空闲超时调用 `drv_ir_rx_finish_nec_falling_interval()` 后才确认单独 repeat。该路径不保存原始波形数组，也不依赖 INT0 双边沿行为。

如果实际引脚无法使用外部中断，允许使用固定周期采样，但必须说明采样周期、CPU 占用和可靠性限制。

### 4.9 红外发射

`drv_ir_tx` 是 NEC `mark/space` 时序编码器，不直接占用 GPIO、PWM、Timer 或中断。

真实硬件发射默认使用：

- 1 个 GPIO 输出。
- 外部三极管或 MOS 管驱动红外 LED。
- 38kHz 载波。

优先使用 PWM 或 Timer 产生载波。

如果使用 GPIO 软件调制，必须在文档中说明 CPU 占用。

红外发射默认只在发送期间占用载波资源，发送结束后关闭 PWM/Timer 输出。NEC `mark/space` 持续时间使用 `stc8h_delay_timer0_1t_us()` 这类硬件计时延时，不使用粗略 C 空循环。

协议编码层输出 `DRV_IR_TX_MARK` 时打开载波，输出 `DRV_IR_TX_SPACE` 时关闭载波。普通帧和标准 NEC repeat 帧都由同一个 `drv_ir_tx_nec_next()` 迭代输出；repeat 帧为 9ms mark、2.25ms space、562us mark。载波生成和红外 LED 驱动属于板级发送层资源，不能由协议层隐藏初始化。

### 4.10 LCD1602

LCD1602 第一版只支持 I2C 转接板。

默认地址：

```text
0x27
```

备用地址：

```text
0x3F
```

必须提供 I2C 地址扫描示例。

所有 I2C 地址文档和 API 均使用 7-bit 未左移地址。底层发送时内部左移并添加 R/W 位。

第一版默认支持常见 PCF8574 背包映射，RS/RW/EN/BL/D4-D7 位通过编译期宏覆盖。

LCD busy flag 默认不读，使用固定延时。

## 5. 冲突处理

如果多个模块需要同一资源，按以下顺序处理：

1. 板级配置中显式分配。
2. 示例中只启用必要模块。
3. 不能隐式抢占资源。
4. 如果资源冲突无法同时满足，示例或文档明确说明不能同时使用。

### 5.1 默认资源冲突矩阵

| 组合 | 第一版默认策略 |
| --- | --- |
| 1ms tick + UART | 允许同时使用。UART 波特率资源按芯片手册由 UART 模块实现决定。 |
| 1ms tick + 按键/EC11 | 允许同时使用。按键/EC11 使用 tick 或主循环 elapsed 时间。 |
| IR RX + IR TX | 默认不同时工作。IR RX 默认占边沿中断 + Timer2，IR TX 默认占 PWM；如果 IR TX fallback 到 Timer2，则与 IR RX 互斥。 |
| IR TX + 无源蜂鸣器 | 默认互斥。二者都可能需要 PWM。 |
| IR TX + 有源蜂鸣器 | 允许同时存在，但示例不要求同时鸣叫和发射红外。 |
| 软件 I2C + TM1637 | 允许同时存在，但各自时序函数阻塞执行，不做并发调度。 |
| EEPROM/IAP + 中断时序模块 | EEPROM/IAP 写擦期间应避免运行红外等时序敏感任务。 |

### 5.2 默认不支持同时运行的组合

第一版默认不支持以下组合同时运行：

- 红外接收和使用 Timer2 fallback 载波的红外发射。
- 红外发射和无源蜂鸣器同时占用同一路 PWM。
- EEPROM/IAP 写擦期间同时执行红外收发。

如果应用项目确实需要这些组合，必须在板级配置和资源报告中重新分配资源。

## 6. 资源报告

每个示例编译后，应记录：

- 编译工具链。
- 系统时钟。
- 已编译模块。
- 已编译 `.c` 文件清单。
- ROM/code 占用。
- RAM/data/idata/xdata 占用。
- 外设和中断占用。
- map 文件或符号表中未使用模块前缀是否缺失。

资源报告文件：

```text
docs/RESOURCE_REPORT.md
```

该文件在实现和实测阶段生成。
