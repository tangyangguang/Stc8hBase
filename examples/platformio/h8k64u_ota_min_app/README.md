# H8K64U OTA Application 最小示例

该示例证明 Application 从 `0x6C00` 链接，并演示 Trial 的 mark-valid、显式升级请求和受控复位。它不包含产品业务或真实输出安全策略。

## 环境

```sh
# 只验证高地址链接；mark-valid 是不持久化的 stub。
pio run -e STC8H8K64U

# 编译真实双槽 Params/IAP 控制 API，运行时会写 0xFC00/0xFE00。
pio run -e STC8H8K64U_mark_valid_iap
```

`STC8H8K64U_mark_valid_iap` 启动后：

1. 建立示例安全态（核心板无负载，所以是 no-op）；
2. 初始化 UART1；
3. 尝试 `stc8h_boot_mark_app_valid()`；
4. 持续喂 Bootloader 留下的 Watchdog。

实验室可向 UART1 发送：

```text
"OTA!" + 4-byte little-endian non-zero session_id
```

Application 持久化升级请求、返回 `OTA ACK`，再受控复位。该 ASCII 入口只用于验收，产品必须换成经过认证/授权的业务命令。

## 产品复用要点

- 保留 `link_app_base.py` 和 `board_upload.maximum_size=33792`；
- 产品最早启动路径先关闭危险输出，再做最小健康检查和 mark-valid；
- 正常循环必须持续喂 Watchdog；
- 先持久化 request，再发送 ACK，最后 reset；ACK 失败且尚未复位时可 cancel；
- 未启用 OTA 的构建不要创建 IAP/Params wrapper，保持零占用。

完整复制清单、代码骨架、打包、工厂安装和状态处理见：

- [`docs/28_H8K64U_OTA_PORTING_GUIDE.md`](../../../docs/28_H8K64U_OTA_PORTING_GUIDE.md)
- [`docs/20_APPLICATION_INTEGRATION.md`](../../../docs/20_APPLICATION_INTEGRATION.md)
