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

## 8. 双板 RF 诊断示例

`examples/platformio/nrf24_pair_diag` 用于隔离两块板之间的 nRF24 空中 auto-ack 链路，不依赖应用项目的显示、EEPROM、绑定、输出或业务 payload。

诊断示例使用当前硬件测试接线：

```text
CE=P1.6, CSN=P1.2, SCK=P1.5, MOSI=P1.3, MISO=P1.4, IRQ=P3.2
```

示例提供两个 PlatformIO 环境：

- `ptx`：烧录到发送端，固定频道 76、地址 `TOYR1`、250kbps、0dBm、ACK payload 开启，周期发送 32 字节 payload，并通过 UART 打印 `TX_DONE`、`MAX_RETRY`、`STATUS`、`OBSERVE_TX`、ACK payload 长度和内容摘要。
- `prx`：烧录到接收端，使用同一频道和地址持续 RX，收到 payload 后打印包计数、长度、序号和 `STATUS`，并重新装载 32 字节 ACK payload。

如果单板 `nrf24_uart_diag` 通过而 `nrf24_pair_diag` 仍持续 `MAX_RETRY`，优先排查 RF 配置一致性、供电、模块、天线、距离和外部干扰；如果 `nrf24_pair_diag` 稳定成功，再回到应用项目排查接收端主循环、配置保存、绑定和输出控制对 RX 状态的影响。

## 9. 参考资料

- Nordic nRF24L01+ Product Specification v1.0：`https://docs-be.nordicsemi.com/bundle/nRF24L01P_PS_v1.0/raw/resource/enus/nRF24L01P_PS_v1.0.pdf`
- TMRh20/RF24 文档和 common issues。
- CircuitPython nRF24L01 文档用于交叉核对 ACK payload 与 dynamic payload 关系。
