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

发送完成后应用可直接读取 `STATUS` 自行处理，也可调用 `drv_nrf24l01_complete_tx()`。该 helper 会把结果分类为：

- `DRV_NRF24L01_TX_DONE`：发送完成，不读取 ACK payload。
- `DRV_NRF24L01_TX_MAX_RT`：达到最大重发，函数会 flush TX 并清 `MAX_RT`。
- `DRV_NRF24L01_TX_ACK_EMPTY`：发送成功但没有 ACK payload。
- `DRV_NRF24L01_TX_ACK_PAYLOAD_OK`：发送成功并已读出 ACK payload。
- `DRV_NRF24L01_TX_ACK_PAYLOAD_INVALID`：ACK payload 宽度非法或超过调用方缓冲区，函数会 flush RX 并清 IRQ。

`drv_nrf24l01_read_rx_packet()` 只用于 dynamic payload 模式。它先用 `R_RX_PL_WID` 读取宽度，宽度为 0、超过 32 或超过调用方缓冲区时按 Nordic 要求 flush RX。

`drv_nrf24l01_preload_ack_payload(pipe, data, len, replace_pending)` 用于 PRX。单 PTX 诊断推荐 `replace_pending=1`，先 flush TX 再写最新 ACK，避免三层 ACK FIFO 里残留旧状态；多 PTX 场景可用 `replace_pending=0`，TX FIFO 满时返回 `STC8H_BUSY`。

`drv_nrf24l01_recover(mode)` 会 CE low、flush TX/RX、清 IRQ，然后进入 standby/PTX/PRX 目标模式。

## 5. ACK Payload 规则

ACK payload 只作为短状态回传优化。它不是复杂双向协议的唯一通道。

使用 ACK payload 时必须同时启用 dynamic payload。PRX 端的 ACK payload 会占用 TX FIFO，最多挂起 3 个 payload。链路断开、同一 pipe 多个旧 ACK 排队或 payload 堵塞时，应用需要 flush TX 恢复；单 peer 状态回传推荐每次 RX 后用 `drv_nrf24l01_preload_ack_payload(..., replace_pending=1)` 保留最新 ACK。

PTX 发送后如果 `STATUS` 同时包含 `TX_DONE` 和 `RX_READY`，表示发送成功且收到 ACK payload。此时应读取 dynamic payload 长度，再读取 RX payload。

## 6. 时序规则

- `PWR_UP` 从 0 变为 1 后，进入 TX/RX 前必须等待 nRF24L01+ datasheet 的 `Tpd2stby`。驱动默认按 5ms 处理，项目确认晶体参数后可用 `DRV_NRF24L01_POWER_UP_DELAY_US` 下调。
- PTX 发送单包时，CE 高电平必须大于 10us。驱动按 `STC8H_SYSCLK_HZ` 计算 CE 脉冲循环，且不低于旧 11.0592MHz/SDCC 实测路径的 64 次循环。
- 250kbps + ACK payload 时，PTX `SETUP_RETR.ARD` 必须按 ACK payload 长度选择。32-byte ACK payload 需要至少 1500us。
- PRX 端 ACK payload 使用 TX FIFO，最多预装 3 个。链路断开或 FIFO 堵塞时，应用需要 `drv_nrf24l01_flush_tx()` 恢复。

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

每个环境烧录时先烧 PRX，再烧 PTX。上传命令在环境名后加 `-t upload`。

汇总输出：

- PTX：`PTX_SUM tx_count=... tx_ok=... max_rt=... ack_ok=... ack_empty=... ack_bad=... OBSERVE_TX=0x.. STATUS=0x.. FIFO_STATUS=0x..`
- PRX：`PRX_SUM rx_count=... seq=0x.. lost=... dup=... bad_width=... STATUS=0x.. FIFO_STATUS=0x.. ack_load=... ack_busy=... ack_fail=...`

PASS 判断：

- 推荐默认配置连续几百到几千包，`max_rt` 为 0 或极低，且不成串增长。
- ACK payload 开启时，PTX `ack_ok` 应持续增长；`ack_empty` 若持续增长，说明 PRX 没有及时预装 ACK 或 ACK FIFO/时序异常。
- ACK payload 关闭时，PTX `tx_ok` 应持续增长，`ack_ok` 维持 0 是正常的。
- PRX `lost/dup` 应为 0 或极低；`bad_width` 必须为 0。

FAIL 方向：

- 单板 diag PASS，但 pair diag `max_rt` 连续增长：优先查频道/速率/地址两端是否一致、距离、天线、2.4GHz 干扰和 3.3V 供电瞬态。
- `ack_empty` 高而 `rx_count` 正常：重点查 PRX ACK preload 逻辑和三层 TX FIFO 是否堵塞。
- `OBSERVE_TX` 高 nibble `PLOS_CNT` 增长：链路层已有丢包，写 `RF_CH` 会清此计数；用它比较不同频道/速率。
- `bad_width` 增长：dynamic payload 收到非法宽度，优先查 SPI/RF 噪声、供电、DPL 配置一致性。

## 10. 稳定性验证矩阵

| 场景 | 环境 | 推荐 ARD/ARC | 说明 |
| --- | --- | --- | --- |
| 1Mbps + 15-byte ACK payload | `ptx` / `prx` | 500us / 15 | 默认推荐配置。Nordic 说明 1Mbps 下 ACK payload 超过 5 字节时 ARD 至少 500us。 |
| 1Mbps + no ACK payload | `ptx_1m_no_ack` / `prx_1m_no_ack` | 500us / 15 | 隔离 ACK payload FIFO，只验证普通 auto-ack。 |
| 250kbps + 15-byte ACK payload | `ptx_250k_15ack` / `prx_250k_15ack` | 1000us / 15 | Nordic 表格要求 250kbps、5-byte 地址、ACK payload 小于 16 字节时 ARD 至少 1000us。 |
| 250kbps + 32-byte ACK payload | `ptx_250k_32ack` / `prx_250k_32ack` | 1500us / 15 | 最容易暴露供电/RF/时序问题；250kbps 空中时间最长，Nordic 表格要求全 ACK payload 长度用 1500us。 |
| 2Mbps + no ACK payload | `ptx_2m_no_ack` / `prx_2m_no_ack` | 500us / 15 | 验证高数据率下普通 auto-ack；2Mbps 链路预算低于 1Mbps/250kbps。 |

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

`nrf24_uart_diag` 使用同名风格的 `NRF24_UART_DIAG_*` 宏，适合先做单模块寄存器验证。

## 12. 参考资料

- Nordic nRF24L01+ Product Specification v1.0：`https://docs.nordicsemi.com/bundle/nRF24L01P_PS_v1.0/resource/nRF24L01P_PS_v1.0.pdf`
- TMRh20/RF24 文档和 common issues。
- CircuitPython nRF24L01 文档用于交叉核对 ACK payload 与 dynamic payload 关系。
