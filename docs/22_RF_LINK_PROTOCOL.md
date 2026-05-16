# RF Link 协议设计

## 1. 目标

`proto_rf_link` 是面向 nRF24L01 等小包射频模块的通用可靠链路层。它服务于遥控、状态监控、远程控制等项目，但不理解具体业务字段。

本协议不保留旧项目兼容，不直接发送 C struct，不内置电机、浇花、电池显示等业务语义。

## 2. 包格式

链路层固定使用 32 字节包：

```text
byte0  magic
byte1  version
byte2  type
byte3  seq
byte4  ack_seq
byte5  flags
byte6  src_id
byte7  dst_id
byte8  len
byte9..31 payload
```

payload 最大 23 字节。应用业务必须手工打包为定长字节，不跨编译器直接发送结构体。

## 3. 包类型

第一版只定义最小类型：

```text
HELLO
HELLO_ACK
DATA
ACK
HEARTBEAT
STATUS
```

命令、配置、传感器数据和遥控数据都放在 `DATA` payload 中，由应用项目解释。协议层只负责链路可靠性。

## 4. 状态机

第一版状态：

```text
IDLE
CONNECTING
CONNECTED
LOST
```

`proto_rf_link_tick()` 由应用周期调用，用于连接超时和心跳计时。本模块不占用 Timer，不创建 ISR。

## 5. 确认与重发边界

包头包含 `seq` 和 `ack_seq`。第一版提供确认语义和超时状态，不实现自动重发队列。是否重发、重发间隔和业务命令是否幂等由应用项目决定。

这能同时支持两类项目：

- 遥控项目：高频发送最新状态，旧包无需排队重发。
- 浇花/状态项目：关键命令由应用等待 ACK 后决定是否重发。

## 6. 频道与绑定

第一版只支持固定频道。频道扫描、绑定和持久化放在后续版本。

持久化绑定信息会涉及 EEPROM/IAP 结构和迁移，实施前必须单独确认版本、CRC、备份和恢复默认值策略。

## 7. API 形态

调用方持有 `proto_rf_link_t` 状态，RAM 占用显式。公共 API 使用完整前缀 `proto_rf_link_`。

协议层只生成和解析 32 字节包，不直接调用驱动发送。应用负责把包交给 `drv_nrf24l01` 或其他射频驱动。
