# nRF24L01 驱动设计

## 1. 目标

`drv_nrf24l01` 是 nRF24L01/nRF24L01+ 芯片驱动，只封装芯片寄存器、SPI 命令、FIFO、IRQ、TX/RX 模式和 nRF24L01+ 可选功能。

本模块不包含配对、频道扫描、遥控器协议、浇花系统协议或业务 payload 格式。应用项目和上层链路协议跟随本模块 API 重写，不保留旧项目兼容层。

## 2. 分层边界

```text
应用项目
  proto_rf_link
    drv_nrf24l01
      stc8h_spi
```

`drv_nrf24l01` 依赖现有硬件 SPI HAL。CE、CSN 和可选 IRQ 使用板级宏绑定：

```c
#define DRV_NRF24L01_CE_HIGH()
#define DRV_NRF24L01_CE_LOW()
#define DRV_NRF24L01_CSN_HIGH()
#define DRV_NRF24L01_CSN_LOW()
#define DRV_NRF24L01_IRQ_READ()
```

`drv_nrf24l01_init_pins()` 会在驱动 CE/CSN 空闲电平前调用可选板级 hook：

```c
#define DRV_NRF24L01_CONFIGURE_PINS()
```

STC8H 等上电默认高阻的芯片应在该 hook 中配置 CE/CSN 端口模式并预置锁存；驱动不直接保存端口号或引脚号。

不新增软件 SPI，不隐藏全局 TX/RX 缓冲区。

## 3. 默认策略

- 默认地址宽度 5 字节。
- 默认固定 payload，大小由应用或链路层设置。
- 默认 1Mbps、0dBm、2-byte CRC、auto-ack pipe0。
- dynamic payload 和 ACK payload 默认关闭，由应用明确启用。
- IRQ 中断服务函数只置位，SPI 读写在主循环执行。

## 4. API 形态

公共 API 使用完整前缀 `drv_nrf24l01_`，不提供旧命名兼容。

驱动提供六类能力：

- 原始命令和寄存器读写，用于调试和高级用法。
- 模式与参数配置，包括频道、地址、速率、功率、CRC、auto retransmit、RX pipe enable 和 auto-ack。
- FIFO 和 IRQ 状态处理，包括 `STATUS`、`FIFO_STATUS`、`OBSERVE_TX`、flush 和 clear IRQ。
- 非阻塞收发动作，包括写 payload、读 payload 和 CE 脉冲。
- nRF24L01+ 可选能力，包括 dynamic payload 和 ACK payload。
- 小型稳定性 helper，包括 PTX 结果分类、动态 RX payload 校验读取、PRX ACK payload 预装和统一 FIFO/IRQ 恢复。

`drv_nrf24l01_set_rx_pipes()` 只写 `EN_RXADDR`；`drv_nrf24l01_set_auto_ack()` 只写 `EN_AA`。这两个寄存器不能再绑在一个 API 里，否则无法诊断“PRX 仍接收，但只关闭 ACK payload 或 auto-ack”的配置。

`drv_nrf24l01_check_present()` 会用 `RX_ADDR_P0` 做 5 字节写读探测，并在返回前恢复原 pipe0 地址。matrix 快速诊断为了 STC8H1K08 的 8KB ROM 余量关闭了每 stage 重复探测；单模块 diag 和普通 pair diag 仍执行该检查。

发送完成后应用可直接读取 `STATUS` 自行处理，也可调用 `drv_nrf24l01_complete_tx()`。该 helper 会把结果分类为：

- `DRV_NRF24L01_TX_DONE`：发送完成，不读取 ACK payload。
- `DRV_NRF24L01_TX_MAX_RT`：达到最大重发，函数会 flush TX 并清 `MAX_RT`。
- `DRV_NRF24L01_TX_ACK_EMPTY`：发送成功但没有 ACK payload。
- `DRV_NRF24L01_TX_ACK_PAYLOAD_OK`：发送成功并已读出 ACK payload。
- `DRV_NRF24L01_TX_ACK_PAYLOAD_INVALID`：ACK payload 宽度非法或超过调用方缓冲区，函数会 flush RX 并清 IRQ。

ACK payload 预期路径下，`drv_nrf24l01_complete_tx()` 不只依赖调用方传入的 `STATUS.RX_DR`。如果 `STATUS` 只有 `TX_DS`，但 `FIFO_STATUS.RX_EMPTY=0`，helper 仍会读取 RX FIFO 中的 ACK payload。实测 250kbps + 32-byte ACK payload 曾出现 `STATUS=0x20` 但 `FIFO_STATUS=0x10`，只看旧 `STATUS` 会误计为 `ACK_EMPTY`。

`drv_nrf24l01_read_rx_packet()` 只用于 dynamic payload 模式。它优先看 `STATUS.RX_DR`，若 `RX_DR` 未置位但 `FIFO_STATUS.RX_EMPTY=0`，仍按 RX FIFO 非空读取，避免日志或主循环延迟后漏读已排队 payload。随后用 `R_RX_PL_WID` 读取宽度，宽度为 0、超过 32 或超过调用方缓冲区时按 Nordic 要求 flush RX。

`drv_nrf24l01_preload_ack_payload(pipe, data, len, replace_pending)` 用于 PRX。正常收包后推荐 `replace_pending=0` 追加下一份 ACK payload，TX FIFO 满时返回 `STC8H_BUSY`；启动、链路恢复或确认无 ACK 正在发送时才用 `replace_pending=1` 清掉旧的三层 ACK FIFO。不要在每次 `RX_DR` 后立刻 `FLUSH_TX` 再写 ACK payload，250kbps 大 ACK payload 下这会撞上当前 ACK 发送窗口，造成 PTX 看到空 ACK。

`drv_nrf24l01_recover(mode)` 会 CE low、flush TX/RX、清 IRQ，然后进入 standby/PTX/PRX 目标模式。

## 5. ACK Payload 规则

ACK payload 只作为短状态回传优化。它不是复杂双向协议的唯一通道。

使用 ACK payload 时必须同时启用 dynamic payload。PRX 端的 ACK payload 会占用 TX FIFO，最多挂起 3 个 payload。同一 pipe 多个 pending payload 按 FIFO 发送。单 peer 状态回传的稳妥策略是：上电/恢复时先 flush TX 并预装一份 ACK；之后每次收到新包后只追加下一份 ACK。链路断开、旧 ACK 堵塞或长时间 `STC8H_BUSY` 时，再进入恢复路径 flush TX。

PTX 发送后如果 `STATUS` 同时包含 `TX_DONE` 和 `RX_READY`，表示发送成功且收到 ACK payload。此时应读取 dynamic payload 长度，再读取 RX payload。

## 6. 时序规则

- `PWR_UP` 从 0 变为 1 后，进入 TX/RX 前必须等待 nRF24L01+ datasheet 的 `Tpd2stby`。驱动默认按 5ms 处理，项目确认晶体参数后可用 `DRV_NRF24L01_POWER_UP_DELAY_US` 下调。
- PTX 发送单包时，CE 高电平必须大于 10us。驱动按 `STC8H_SYSCLK_HZ` 计算 CE 脉冲循环，且不低于旧 11.0592MHz/SDCC 实测路径的 64 次循环。
- 250kbps + ACK payload 时，PTX `SETUP_RETR.ARD` 必须按 ACK payload 长度选择。32-byte ACK payload 需要至少 1500us。
- PRX 端 ACK payload 使用 TX FIFO，最多预装 3 个。正常收包路径不应 `FLUSH_TX`；链路断开、FIFO 堵塞或恢复路径才用 `drv_nrf24l01_flush_tx()`。

## 7. 硬件注意事项

- nRF24L01 是 2.4GHz，穿墙和楼板能力有限，远距离项目必须实测。
- 模块附近建议放 10uF 电解或钽电容，并并联 100nF。
- PA/LNA 模块峰值电流高，不能依赖弱 3.3V 输出。
- 异常寄存器值、ACK 不稳定、payload 错乱时，优先检查供电、线长和 SPI 速度。
- `MAX_RETRY` 后必须清 IRQ，必要时 flush TX。

## 8. UART 单板诊断

`examples/platformio/nrf24_uart_diag` 只验证单模块 SPI、寄存器和 FEATURE/DYNPD，不进行空中收发。

默认 PCB 引脚：

```text
CE=P1.6, CSN=P1.2, SCK=P1.5, MOSI=P1.3, MISO=P1.4, IRQ=P3.2
```

构建：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/nrf24_uart_diag
pio run -e STC8H1K08
pio run -e STC8H1K08 -t upload
```

UART PASS 判断：

- `STATUS=0x0E` 或其他非 `0x00/0xFF` 的合理 nRF24 状态。
- `CHECK_PASS=8/8`。
- `DPL: OK` 和 `ACK_PAYLOAD: OK`，或按编译宏配置的 `ENABLE_DPL/ENABLE_ACK_PAYLOAD: OK`。
- `FEATURE` 和 `DYNPD` 与配置一致。

FAIL 方向：

- `STATUS=0x00`：优先查 MISO 数字输入、CSN、MISO 线、模块供电。
- `STATUS=0xFF`：优先查 CSN 未选中、MISO 上拉/断线、模块未供电。
- `CHECK_PASS` 不是 `8/8`：SPI 寄存器写读仍不稳定，不应继续调应用协议。

## 9. 双板 RF 诊断示例

`examples/platformio/nrf24_pair_diag` 用于隔离两块板之间的 nRF24 空中 auto-ack 链路，不依赖应用项目的显示、EEPROM、绑定、输出或业务 payload。

诊断示例使用当前硬件测试接线：

```text
CE=P1.6, CSN=P1.2, SCK=P1.5, MOSI=P1.3, MISO=P1.4, IRQ=P3.2
```

默认 `ptx`/`prx` 环境是推荐起点：频道 76、地址 `TOYR1`、1Mbps、0dBm、15-byte payload、ACK payload 开、dynamic payload 开、ARD=500us、ARC=15、每 100 包汇总一次。

常用构建/烧录命令：

```sh
cd /Users/tyg/dir/codex_dir/Stc8hBase/examples/platformio/nrf24_pair_diag
pio run -e prx
pio run -e prx -t upload
pio run -e ptx
pio run -e ptx -t upload
```

验证矩阵环境：

```sh
pio run -e ptx && pio run -e prx
pio run -e ptx_1m_no_ack && pio run -e prx_1m_no_ack
pio run -e ptx_250k_15ack && pio run -e prx_250k_15ack
pio run -e ptx_250k_32ack && pio run -e prx_250k_32ack
pio run -e ptx_2m_no_ack && pio run -e prx_2m_no_ack
```

一键快测环境会在同一次烧录后自动跑完整矩阵，PTX 用当前阶段配置发送切换包，PRX 收到切换包后再改下一阶段配置，避免两边速率或 ACK payload 配置不同步：

```sh
pio run -e prx_matrix_fast -t upload --upload-port /dev/cu.usbserial-120
pio run -e ptx_matrix_fast -t upload --upload-port /dev/cu.usbserial-110
pio device monitor -p /dev/cu.usbserial-120 -b 115200
pio device monitor -p /dev/cu.usbserial-110 -b 115200
```

`matrix_fast` 默认每 5ms 发一包、每 1000 包汇总一次，阶段为：1Mbps+15-byte ACK payload、1Mbps+no ACK payload、250kbps+15-byte ACK payload、250kbps+32-byte ACK payload/1500us、250kbps+32-byte ACK payload/2000us、250kbps+32-byte ACK payload/2500us、2Mbps+no ACK payload。所有阶段都启用 auto-ack；`no ACK payload` 只表示关闭 ACK payload，不表示关闭 auto-ack。普通阶段 2000 包，250kbps 32-byte 的 2000us/2500us margin 阶段各 5000 包。每个阶段正式计数前会先发送 16 个 `tx_count=0` 预热包；PRX 不把这些包计入 `rx_count/lost/dup`，用于避开阶段切换后首包同步瞬态。ACK payload 阶段的 `ack_load` 会包含预热包触发的 ACK 预装次数，因此可能比 `rx_count+1` 多 16 左右。matrix 会额外输出 `MATRIX_WARMUP`，暴露预热阶段 PTX 的 `warmup_tx/max_rt/ack_empty/ack_bad` 和 PRX 的 `warmup_rx/bad_width/ack_busy/ack_fail`。为给 STC8H1K08 8KB flash 留出余量，matrix 环境关闭逐阶段 write/read presence check；单板 `nrf24_uart_diag` 和普通 `ptx`/`prx` 环境仍执行 presence check。

每个环境烧录时先烧 PRX，再烧 PTX。上传命令在环境名后加 `-t upload`。

汇总输出：

- PTX：`PTX_SUM tx_count=... tx_ok=... max_rt=... ack_ok=... ack_empty=... ack_bad=... OBSERVE_TX=0x.. STATUS=0x.. FIFO_STATUS=0x..`
- PRX：`PRX_SUM rx_count=... seq=0x.. lost=... dup=... ptx_reset=... bad_width=... STATUS=0x.. FIFO_STATUS=0x.. ack_load=... ack_busy=... ack_fail=...`
- 矩阵快测：`MATRIX_STAGE_BEGIN` 表示阶段配置，`MATRIX_WARMUP` 表示正式计数前的预热结果，`MATRIX_PTX_SUM`/`MATRIX_PRX_SUM` 是阶段内汇总，`MATRIX_STAGE_END` 是阶段结论，`MATRIX_DONE` 表示全部阶段结束。

PASS 判断：

- 推荐默认配置连续几百到几千包，`max_rt` 为 0 或极低，且不成串增长。
- ACK payload 开启时，PTX `ack_ok` 应持续增长；matrix 模式下 `ack_ok` 还要求 ACK payload 长度匹配、前三字节为 `ACK`，正式计数包还要求 ACK 中的 `last_seq` 对应上一包序号；`ack_empty` 若持续增长，说明 PRX 没有及时预装 ACK 或 ACK FIFO/时序异常。
- ACK payload 关闭时，PTX `tx_ok` 应持续增长，`ack_ok`、`ack_empty` 和 PRX `ack_load` 维持 0 是正常的。
- PRX `lost/dup` 应为 0 或极低；PTX 重新烧录/复位会计入 `ptx_reset`，不再混入 `lost`；`bad_width` 必须为 0。
- `matrix_fast` 每个阶段看对应的 `MATRIX_WARMUP` 和 `MATRIX_STAGE_END`：预热阶段 `max_rt/ack_empty/ack_bad/ack_busy/ack_fail` 应为 0 或极低；ACK payload 阶段要求 `ack_ok` 接近 `tx_ok`、`ack_empty=0` 或极低、`ack_bad=0`；no ACK payload 阶段要求 `tx_ok` 持续增长且 `ack_ok=0`；PRX 侧要求 `lost=0`、`dup=0`、`bad_width=0`。

FAIL 方向：

- 单板 diag PASS，但 pair diag `max_rt` 连续增长：优先查频道/速率/地址两端是否一致、距离、天线、2.4GHz 干扰和 3.3V 供电瞬态。
- `ack_empty` 高而 `rx_count` 正常：重点查 PRX ACK preload 逻辑和三层 TX FIFO 是否堵塞；尤其确认正常收包路径没有每包后 `FLUSH_TX`。
- `OBSERVE_TX` 高 nibble `PLOS_CNT` 增长：链路层已有丢包，写 `RF_CH` 会清此计数；用它比较不同频道/速率。
- `bad_width` 增长：dynamic payload 收到非法宽度，优先查 SPI/RF 噪声、供电、DPL 配置一致性。

## 10. 稳定性验证矩阵

| 场景 | 环境 | 推荐 ARD/ARC | 说明 |
| --- | --- | --- | --- |
| 1Mbps + 15-byte ACK payload | `ptx` / `prx` | 500us / 15 | 默认推荐配置。Nordic 说明 1Mbps 下 ACK payload 超过 5 字节时 ARD 至少 500us。 |
| 1Mbps + no ACK payload | `ptx_1m_no_ack` / `prx_1m_no_ack` | 500us / 15 | 隔离 ACK payload FIFO，只验证普通 auto-ack。 |
| 250kbps + 15-byte ACK payload | `ptx_250k_15ack` / `prx_250k_15ack` | 1000us / 15 | Nordic 表格要求 250kbps、5-byte 地址、ACK payload 小于 16 字节时 ARD 至少 1000us。 |
| 250kbps + 32-byte ACK payload | `ptx_250k_32ack` / `prx_250k_32ack` | 1500us / 15 | 规范基线；Nordic 表格要求全 ACK payload 长度用 1500us，RF24/CircuitPython 也把 1500us 作为可靠性默认。 |
| 250kbps + 32-byte ACK payload, margin | `ptx_250k_32ack_2ms` / `prx_250k_32ack_2ms` | 2000us / 15 | 硬件余量测试；如果 1500us 少量 `MAX_RT` 而 2000us 清零，可作为当前 PCB/供电/RF 环境的长期应用值。 |
| 250kbps + 32-byte ACK payload, margin | `ptx_250k_32ack_2500us` / `prx_250k_32ack_2500us` | 2500us / 15 | 更保守余量测试；用于确认 2000us 是否已经足够，不作为通用默认。 |
| 2Mbps + no ACK payload | `ptx_2m_no_ack` / `prx_2m_no_ack` | 500us / 15 | 验证高数据率下普通 auto-ack；2Mbps 链路预算低于 1Mbps/250kbps。 |

`nrf24_pair_diag` 会在编译期检查 Nordic ARD 下限，避免组合出数据手册不允许的 ACK payload/速率/ARD 配置。基础驱动 `drv_nrf24l01_set_auto_retransmit()` 仍保持寄存器级 API，不根据速率或 payload 长度猜默认值。

2026-05-21 真机记录：接收机 `/dev/cu.usbserial-120` 烧录 `prx_matrix_fast`，遥控器 `/dev/cu.usbserial-110` 烧录 `ptx_matrix_fast`，两端完整跑到 `MATRIX_DONE`。PTX 七个 stage 均 `tx_ok == tx_count`、`max_rt=0`、`ack_bad=0`；PRX 七个 stage 均 `lost=0`、`dup=0`、`bad_width=0`、`ack_busy=0`、`ack_fail=0`。其中 250kbps + 32-byte ACK payload 的 1500us/2000us/2500us 阶段分别跑 2000/5000/5000 包均无 `MAX_RT`。当前硬件已在 nRF24 VCC/GND 近端补 100uF 固态电容；长期应用默认建议 250kbps + 32-byte ACK payload + ARD 2000us / ARC 15，若追求最大余量可用 2500us。

如果默认配置稳定而 ToyRemote 仍不稳定，再回到应用项目接入；如果默认配置也不稳定，先不要改应用代码，按 UART 日志定位到 SPI、RF、供电或 ACK payload 方向。

## 11. 编译宏

`nrf24_pair_diag` 支持这些宏：

- `NRF24_PAIR_CHANNEL`
- `NRF24_PAIR_DATA_RATE`: `NRF24_PAIR_RATE_250KBPS` / `NRF24_PAIR_RATE_1MBPS` / `NRF24_PAIR_RATE_2MBPS`
- `NRF24_PAIR_RF_POWER`: `NRF24_PAIR_POWER_NEG18DBM` / `NEG12DBM` / `NEG6DBM` / `0DBM`
- `NRF24_PAIR_PAYLOAD_SIZE`: 8..32，常用 8/15/32
- `NRF24_PAIR_ACK_PAYLOAD`
- `NRF24_PAIR_DYNAMIC_PAYLOAD`
- `NRF24_PAIR_AUTO_ACK`
- `NRF24_PAIR_RETRANSMIT_DELAY_CODE`
- `NRF24_PAIR_RETRANSMIT_COUNT_CODE`
- `NRF24_PAIR_SEND_PERIOD_MS`
- `NRF24_PAIR_SUMMARY_INTERVAL`
- `NRF24_PAIR_LOG_EACH_PACKET`
- `NRF24_PAIR_MATRIX_DIAG`: 设为 1 时启用自动矩阵快测；配套环境为 `ptx_matrix_fast` / `prx_matrix_fast`。

`nrf24_uart_diag` 使用同名风格的 `NRF24_UART_DIAG_*` 宏，适合先做单模块寄存器验证。

## 12. 参考资料

- Nordic nRF24L01+ Product Specification v1.0：`https://docs.nordicsemi.com/bundle/nRF24L01P_PS_v1.0/resource/nRF24L01P_PS_v1.0.pdf`
- TMRh20/RF24 文档和 common issues。
- CircuitPython nRF24L01 文档用于交叉核对 ACK payload 与 dynamic payload 关系。
