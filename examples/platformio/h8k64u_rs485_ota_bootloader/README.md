# H8K64U RS485 OTA Bootloader 示例

这是 `STC8H8K64U-45I-LQFP48` 的低地址常驻 Bootloader 参考实现，不是可直接用于任意产品板的成品固件。

## 直接复用前必须修改

- `src/boot_config.h`：board/hardware/app ID、`H8K64U_OTA_SAFE_OUTPUTS_OFF()`；
- `H8K64U_OTA_LOCAL_ADDR`：产品单播地址；
- 板级配置：11.0592 MHz、UART2 baud、pin group；
- `src/drv_rs485_uart_wrap.c`：自动收发或真实 DE/RE；
- 产品应用包必须使用相同 ID、地址约定和 `0x6C00` 布局。

产品板不得沿用 safe-output no-op。

## 构建

```sh
pio run
```

构建必须满足：

- HOME=`0x0200`；
- reset=`LJMP 0x0200`；
- 低地址向量 0..44 转发到 `0x6C00 + offset`；
- 镜像不越过 `0x6BFF`；
- `program_eeprom_split=27648`。

完整检查：

```sh
../../../tools/check_examples_full.sh
```

## 首次安装和应用接入

不要把跨 split 的组合 HEX 直接作为 stcgal code image。打包、分离 code/eeprom、两阶段 ISP 安装、Application API、Sender 命令和硬件验收步骤见：

- [`docs/28_H8K64U_OTA_PORTING_GUIDE.md`](../../../docs/28_H8K64U_OTA_PORTING_GUIDE.md)
- [`docs/25_H8K64U_OTA_DESIGN.md`](../../../docs/25_H8K64U_OTA_DESIGN.md)

UART1 只用于首次安装、Bootloader 更新和救援；远程 OTA 数据走 UART2/可靠双向 Transport。Bootloader 不应加入业务逻辑、联网下载或自动版本发现。
