# Titan Micro 资料归档

本目录保存 TM1637 显示驱动芯片资料。

本地资料总索引见 `docs/vendor/README.md`。

## 已归档文件

| 文件 | 来源 | 用途 | SHA-256 |
| --- | --- | --- | --- |
| `TM1637_TitanMicro.pdf` | Titan Micro datasheet 权威镜像：`https://www.makerguides.com/wp-content/uploads/2019/08/TM1637-Datasheet.pdf` | TM1637 二线接口、显示寄存器地址、数据命令、显示控制命令和 ACK 时序核对 | `5f0e107b0398111395d660baa421ca3826278eb2838f14f1742758667106b128` |

下载日期：2026-05-10。

## 收录说明

- TM1637 原厂官网直链不稳定，本项目保存厂家 datasheet 的权威镜像。
- 实现 `drv_tm1637` 前必须核对本资料中的命令字、ACK 和显示寄存器地址。
