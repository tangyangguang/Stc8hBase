# Infineon 资料

本目录保存红外遥控协议相关权威资料。

## 已归档文件

| 文件 | 来源 | 用途 | SHA-256 |
| --- | --- | --- | --- |
| `Infineon_AN2023_03_IR_Remote_Control.pdf` | Infineon 官方应用笔记：`https://www.infineon.com/dgdl/Infineon-AN2023-03_Infrared_Remote_Control_and_Saving_Last_Speed_Setting-ApplicationNotes-v01_00-EN.pdf?fileId=8ac78c8c8d1b852e018d21ff0aa71feb` | NEC 红外遥控引导码、重复码、数据位脉宽和载波事实核对 | `f07b3b420e2b65b6c34795d97d66f04b99521d1c83d501b00e261026e965aa9b` |

下载日期：2026-05-10。

## 使用原则

- 本资料用于核对 NEC 协议事实；具体接收头和红外 LED 驱动电路仍需按实际器件资料和示波器实测确认。
- 本库第一版只实现 NEC 普通 8-bit 地址格式，不实现学习型遥控或多协议自动识别。
