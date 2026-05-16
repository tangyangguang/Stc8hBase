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

不新增软件 SPI，不隐藏全局 TX/RX 缓冲区。

## 3. 默认策略

- 默认地址宽度 5 字节。
- 默认固定 payload，大小由应用或链路层设置。
- 默认 1Mbps、0dBm、2-byte CRC、auto-ack pipe0。
- dynamic payload 和 ACK payload 默认关闭，由应用明确启用。
- IRQ 中断服务函数只置位，SPI 读写在主循环执行。

## 4. API 形态

公共 API 使用完整前缀 `drv_nrf24l01_`，不提供旧命名兼容。

驱动提供五类能力：

- 原始命令和寄存器读写，用于调试和高级用法。
- 模式与参数配置，包括频道、地址、速率、功率、CRC、auto retransmit。
- FIFO 和 IRQ 状态处理，包括 `STATUS`、`FIFO_STATUS`、`OBSERVE_TX`、flush 和 clear IRQ。
- 非阻塞收发动作，包括写 payload、读 payload 和 CE 脉冲。
- nRF24L01+ 可选能力，包括 dynamic payload 和 ACK payload。

发送完成后应用读取 `STATUS`，自行处理 `TX_DONE`、`MAX_RETRY` 和可能同时出现的 `RX_READY`。

## 5. ACK Payload 规则

ACK payload 只作为短状态回传优化。它不是复杂双向协议的唯一通道。

使用 ACK payload 时必须同时启用 dynamic payload。PRX 端的 ACK payload 会占用 TX FIFO，最多挂起 3 个 payload。链路断开或 payload 堵塞时，应用需要 `drv_nrf24l01_flush_tx()` 恢复。

PTX 发送后如果 `STATUS` 同时包含 `TX_DONE` 和 `RX_READY`，表示发送成功且收到 ACK payload。此时应读取 dynamic payload 长度，再读取 RX payload。

## 6. 硬件注意事项

- nRF24L01 是 2.4GHz，穿墙和楼板能力有限，远距离项目必须实测。
- 模块附近建议放 10uF 电解或钽电容，并并联 100nF。
- PA/LNA 模块峰值电流高，不能依赖弱 3.3V 输出。
- 异常寄存器值、ACK 不稳定、payload 错乱时，优先检查供电、线长和 SPI 速度。
- `MAX_RETRY` 后必须清 IRQ，必要时 flush TX。

## 7. 参考资料

- Nordic nRF24L01+ Product Specification v1.0。
- TMRh20/RF24 文档和 common issues。
- CircuitPython nRF24L01 文档用于交叉核对 ACK payload 与 dynamic payload 关系。
