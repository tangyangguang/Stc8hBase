# STC8H8K64U Remote OTA Foundation

## 1. 范围与安全声明

本设计只覆盖 STC8H8K64U 的可复用单 Application OTA 核心和第一阶段 PC→USB-RS485→UART2 Sender。它不包含云下载、ESP32 暂存/发送、业务升级窗口、433 模块适配、签名验签或 Bootloader 远程更新。

关键约束：

- 操作者先获得完整 `.stcota` 包并完成本机检查，再用明确命令启动更新。
- 上电、联网、串口活动、广播、探测和发现新版本都不得自动擦写。
- Application 有效时，Bootloader 只接受 Application 已持久化的同一非零 `session_id`；Recovery/空参数状态下仍需 PC 明确执行 `update` 或 `resume`。
- `--restart` 会丢弃当前断点并重新擦除 Application；无该参数时，不允许其他 session 接管。
- 第一阶段 CRC/UID/目标字段防误传和损坏，不抵御总线主动攻击者，不宣称安全启动。

## 2. 固定 Flash 契约

| 区域 | 地址 | 大小 | 远程可写 |
|---|---:|---:|---|
| Bootloader | `0x0000..0x6BFF` | 27 KiB | 否 |
| Application | `0x6C00..0xEFFF` | 33 KiB | 是 |
| 产品数据 | `0xF000..0xFBFF` | 3 KiB | 否 |
| OTA Params A | `0xFC00..0xFDFF` | 512 B | 是 |
| OTA Params B | `0xFE00..0xFFFF` | 512 B | 是 |

STC ISP 的 `program_eeprom_split` 必须为十进制 `27648`（`0x6C00`）；示例上传脚本通过 `custom_stcgal_eeprom_split = 27648` 显式传给 stcgal。Bootloader 从 `0x0200` 链接；`0x0000` 固定 `LJMP 0x0200`。低地址中断槽 0..44 固定转发到 `0x6C00 + vector_offset`（SDCC 对大于 31 的 ISR 仍需按芯片手册使用汇编入口）。Application 同样以 `--code-loc 0x6C00` 链接，SDCC 会把应用向量放到相同偏移。

Bootloader 当前 SDCC 构建为 `25951 / 27648 bytes`（93.9%，余 1697 bytes）；带显式 UART1 lab request 的 mark-valid IAP 最小 Application 为 `9789 / 33792 bytes`。后续新增能力必须重新检查两边余量，不得侵占 `0xF000..0xFFFF`。

## 3. 分层

- `proto_ota_frame`：Transport-neutral 帧、CRC16、collector。
- `stc8h_ota_format`：固定 31-byte Manifest 和 36-byte Params wire format。
- `stc8h_ota`：顺序写入、断点、读回、整镜像 CRC32、状态机。
- `stc8h_ota_receiver`：将已解析的单播命令映射到 OTA Core，并校验目标 UID；不拥有串口。
- `stc8h_ota_params_store`：双 512-byte 槽、generation、CRC、最后提交标记。
- `stc8h_iap_program` / `stc8h_iap_ota_params`：受编译期边界约束的 IAP backend。
- `stc8h_chip_identity`：读取 7-byte CHIPID 并生成 16-byte canonical UID。
- `h8k64u_rs485_ota_bootloader`：UART2 polling Transport Adapter、Watchdog、safe-output hook、ACK 后复位。
- `tools/stc8h_ota.py`：pack、inspect、factory、info、update、resume。

其他可靠双向 Transport 可复用 Receiver/Core，但必须自己提供单播、ACK、超时、重试、去重和分包。只有单向 433 发射模块不能承载本协议。

## 4. Wire protocol v1

每帧：

```text
SOF[2]="OT", version, command, flags, dst, src, reserved,
session_id:u32le, sequence:u16le, offset:u16le, length:u16le,
payload[0..128], crc16_modbus:u16le
```

命令：`INFO`、`BEGIN`、`DATA`、`VERIFY`、`ACTIVATE`、`ABORT`、`STATUS`。响应统一为 `STATUS`。普通状态 payload 20 bytes，`INFO` 为 36 bytes并附完整 16-byte UID。`BEGIN` payload 固定为 31-byte Manifest + 从 INFO 原样回送的 16-byte UID；目标 UID 不匹配时，在任何擦除之前拒绝。

Host 使用 window=1、128-byte 顺序块和有限重试。同一 `session/sequence/command` 的重发返回 DUPLICATE ACK；已提交区域的数据重发只有逐字节一致时才成功。

## 5. Manifest 和包

Manifest 固定字段包括：magic/format、chip、board/hardware/app ID、`app_base`、`app_size`、CRC32、语义版本、最低 Bootloader 版本、build、flags 和 CRC16。Bootloader 要求：

- chip=`STC8H8K64U`，base=`0x6C00`；
- size 非零且不越过 `0xEFFF`；
- board/hardware/app ID 与构建期目标一致（产品集成必须配置）；
- 最低 Bootloader 版本可满足；
- Manifest CRC16 有效。

`.stcota` 包含 package header、canonical Manifest、连续 Application bytes 和 SHA-256。SHA-256 在 PC 侧检查；STC 侧执行 Manifest CRC16、每帧 CRC16、每块写后读回和整镜像 CRC32。

## 6. Params 和掉电恢复

Params 状态：

```text
EMPTY -> APP_VALID -> UPDATE_REQUESTED -> PREPARING -> RECEIVING
RECEIVING -> VERIFIED -> TRIAL_PENDING -> TRIAL_STARTED -> APP_VALID
PREPARING/RECEIVING/FAILED -> RECOVERY
```

每次更新写入非活动槽：擦除目标槽、写前 34 bytes、读回比较、最后写 2-byte commit marker、再完整解码确认。选择规则：

- 仅一个有效槽时选它；
- 两槽有效时按 16-bit modular generation 选择较新者；
- generation 相同视为歧义，停留 Recovery；
- 新槽 torn write/CRC/marker 损坏时保留旧槽；
- generation 从 `0xFFFF` 回绕到 `0x0000`。

每 2048 bytes 和镜像末尾持久化 `committed_offset`。掉电后只信任该断点；未持久化尾部允许 Host 重发并逐字节比较。单 Application 无法在本地保存上一版，Recovery 只保证 Bootloader 可继续接收。

## 7. Trial、Watchdog 与输出安全

每次 reset 先执行 Bootloader：

- `APP_VALID` 且整镜像 CRC32 正确：跳转 Application。
- `TRIAL_PENDING` 且 CRC32 正确：先持久化 `TRIAL_STARTED`，再跳转。
- `TRIAL_STARTED`、损坏镜像、更新中或失败：停留 Bootloader。

Bootloader 开启 Watchdog，并在 erase/write/read/CRC 长循环喂狗。Application 接管时 Watchdog 仍在运行；它必须先建立安全输出、完成最小健康检查、调用 `stc8h_boot_mark_app_valid()`，随后持续喂狗。trial 未确认前复位不会自动再试。

产品 Bootloader 必须实现 `H8K64U_OTA_SAFE_OUTPUTS_OFF()`，且不得分配资源或开启中断。核心板示例因无受控负载明确使用 no-op。

## 8. PC 工具

构建包：

```sh
python3 tools/stc8h_ota.py pack \
  --hex path/to/application.hex \
  --output build/app.stcota \
  --board-id 1 --hardware-revision 1 --app-id 1 \
  --version 1.2.3 --build 42
python3 tools/stc8h_ota.py inspect build/app.stcota
```

生成首次 UART1/STC ISP 工厂镜像（Bootloader + APP_VALID Application + Params A）：

```sh
python3 tools/stc8h_ota.py factory \
  --boot-hex examples/platformio/h8k64u_rs485_ota_bootloader/.pio/build/STC8H8K64U/firmware.hex \
  --package build/app.stcota --output build/factory.hex
```

Bootloader 已在总线上等待时：

```sh
python3 tools/stc8h_ota.py probe --port /dev/cu.usbserial-X --address 34
python3 tools/stc8h_ota.py update --port /dev/cu.usbserial-X --address 34 \
  --file build/app.stcota
python3 tools/stc8h_ota.py resume --port /dev/cu.usbserial-X --address 34 \
  --file build/app.stcota
```

交互命令要求输入 `UPDATE <address>`；CI/已审核脚本必须显式给 `--yes`。只有确认丢弃当前断点时才加 `--restart`；旧 build 还必须显式加 `--allow-downgrade`。`--no-activate` 可停在 VERIFIED；ACTIVATE 始终是独立协议步骤。

Application 正常运行时不会响应 OTA INFO。产品协议应先调用 `stc8h_boot_request_update(session_id)`，发送 ACK 后再受控复位；PC 第一阶段也可在预先进入 Bootloader/Recovery 的台架上操作。`h8k64u_ota_min_app` 的 mark-valid 环境仅为硬件验收提供 UART1 lab request：发送 ASCII `OTA!` 后紧跟 4-byte little-endian 非零 session，持久化成功后返回 `OTA ACK` 并复位；该入口不是产品协议，不应复制到产品固件。RS485 总线上不得同时存在 ESP32 和 PC 两个主站。

## 9. 验证与未完成边界

自动 Gate：

```sh
tools/check_host_tests_full.sh
tools/check_examples_full.sh
```

Gate 覆盖 frame/collector、Manifest/Params、双槽 torn write、generation wrap/歧义、显式请求、checkpoint/resume、重复块、readback、CRC 失败、restart/abort、UID 绑定、工具包/工厂镜像，以及 Bootloader reset/vector/分区边界。SDCC 全示例已通过；Keil C51 仅保持源码语法边界，仍需 Windows+Keil 真编译。

真实硬件写擦尚须单独授权并记录：首次 ISP 工厂安装、正常 update、断电 resume、错误 UID/包、重复块、CRC 失败、trial 未确认和 mark-valid。完成这些之前，不宣称生产闭环。ESP32 Sender 和 433 Adapter 不在本阶段。