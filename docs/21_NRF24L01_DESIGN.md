# nRF24L01 驱动设计

`drv_nrf24l01` 只封装 nRF24L01/nRF24L01+ 的 SPI 命令、寄存器、FIFO、IRQ、TX/RX 模式和 nRF24L01+ 可选功能。

本模块不包含配对、频道扫描、遥控器协议、业务 payload、灌溉系统协议或链路可靠性框架。

## 分层

```text
应用项目或轻量协议
  -> drv_nrf24l01
    -> stc8h_spi
```

CE、CSN 和可选 IRQ 由板级宏绑定：

```c
#define DRV_NRF24L01_CE_HIGH()
#define DRV_NRF24L01_CE_LOW()
#define DRV_NRF24L01_CSN_HIGH()
#define DRV_NRF24L01_CSN_LOW()
#define DRV_NRF24L01_IRQ_READ()
```

`drv_nrf24l01_init_pins()` 会在驱动 CE/CSN 空闲电平前调用：

```c
#define DRV_NRF24L01_CONFIGURE_PINS()
```

STC8H 上电默认高阻时，应在该 hook 中配置 CE/CSN 端口模式并预置锁存。驱动不保存端口号，也不直接知道板级引脚。

## 默认策略

- 默认 5 字节地址宽度。
- 默认固定 payload。
- 默认 1Mbps、0dBm、2-byte CRC、pipe0 auto-ack。
- dynamic payload 和 ACK payload 由应用显式启用。
- IRQ ISR 只置位，SPI 收发放在主循环。
- 不提供旧项目命名兼容层。

## API 范围

驱动提供：

- 原始寄存器和 buffer 读写。
- 频道、地址、速率、功率、CRC、auto retransmit、RX pipe 和 auto-ack 配置。
- STATUS、FIFO_STATUS、OBSERVE_TX、flush 和 IRQ 清除。
- 非阻塞写 payload、读 payload 和 CE pulse。
- dynamic payload、ACK payload。
- PTX 完成分类、dynamic RX payload 读取、PRX ACK payload 预装和基础 recover helper。

8051 地址空间相关 API 只在能明显减少 SDCC generic pointer helper 时保留，例如：

- `drv_nrf24l01_write_payload_fixed_xdata()`
- `drv_nrf24l01_read_payload_fixed_xdata()`
- `drv_nrf24l01_config_pipe0_fixed_code()`

这些 API 不改变 nRF24 语义，只把 DATA/XDATA/CODE 约束显式写进签名。

## ACK Payload

ACK payload 只作为短状态回传优化，不作为完整双向协议。

规则：

- ACK payload 必须配合 dynamic payload。
- PRX 的 ACK payload 使用 TX FIFO，最多挂起 3 个 payload。
- 正常收包后优先追加下一份 ACK payload。
- 只有启动、恢复或确认旧 ACK 堵塞时才 flush TX。
- PTX 发送后若收到 ACK payload，应读取 dynamic payload 长度再读 payload。

`drv_nrf24l01_complete_tx()` 会处理常见结果：

- `TX_DONE`
- `MAX_RT`
- `ACK_EMPTY`
- `ACK_PAYLOAD_OK`
- `ACK_PAYLOAD_INVALID`

如果 `STATUS` 只有 `TX_DS`，但 `FIFO_STATUS.RX_EMPTY=0`，helper 仍会尝试读取 RX FIFO 中的 ACK payload，避免漏读已排队的 ACK。

## 时序

- `PWR_UP` 后进入 TX/RX 前等待 `Tpd2stby`。
- PTX 单包发送 CE 高电平必须大于 10us。
- 250kbps + 32-byte ACK payload 时，`SETUP_RETR.ARD` 至少使用 1500us。
- `MAX_RT` 后必须清 IRQ，必要时 flush TX。

## 保留示例

- `examples/platformio/nrf24_fixed_ping`：固定 payload 发送和 TX 结果处理。
- `examples/platformio/nrf24_ack_payload`：dynamic payload 与 ACK payload 最小验证。
- `examples/platformio/rf_link_nrf24_small`：小内存目标上的 RF link 裁剪构建示例，不作为完整运行期链路参考。

已删除的 UART 寄存器 dump、双板自动 matrix 诊断等硬件排查程序不再作为长期维护对象。真实项目需要链路 margin 或模块寄存器排查时，应在应用项目中按目标硬件、供电、天线、距离和干扰环境做专项测试，不把大矩阵或调试串口流水固化进基础库。

## 硬件注意事项

- nRF24L01 是 2.4GHz，穿墙和楼板能力有限，远距离项目必须实测。
- 模块附近建议放 10uF 或更大电容，并并联 100nF。
- PA/LNA 模块峰值电流高，不能依赖弱 3.3V 输出。
- 异常寄存器值、ACK 不稳定或 payload 错乱时，优先检查供电、线长、CSN/CE、MISO 输入和 SPI 速度。

## 参考资料

- Nordic nRF24L01+ Product Specification v1.0。
- TMRh20/RF24 文档和 common issues。
- CircuitPython nRF24L01 文档用于交叉核对 ACK payload 与 dynamic payload 关系。
