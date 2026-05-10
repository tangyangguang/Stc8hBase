# 本地参考资料索引

本目录只归档本项目高频使用、实现或硬件调试必须反复引用的资料。

## 收录原则

- 芯片和器件优先使用官方 datasheet/manual。
- 找不到官方可下载源时，可收录厂家 datasheet 的权威镜像，并在来源中明确说明。
- 每个文件必须记录来源、用途、下载日期和 SHA-256。
- 不收录 IDE、下载工具安装包、示例工程压缩包、无关评估板资料。
- 新增芯片或外设时，先补资料索引，再写代码。

## 已归档资料

| 文件 | 来源级别 | 来源 | 用途 | SHA-256 |
| --- | --- | --- | --- | --- |
| `stc/STC8H1K08_Features.pdf` | STC 官方 | `https://www.stcmicro.com/datasheet/STC8H1K08_Features.pdf` | STC8H1K08 资源、封装、TSSOP20 引脚和外设快速核对 | `fe83c09670fe1f87784d9afc4354f3c2f136c214f59f9c5ed6eb43e47120bcc6` |
| `stc/STC8H-en.pdf` | STC 官方 | `https://www.stcmicro.com/datasheet/STC8H-en.pdf` | STC8H 系列寄存器、UART/Timer/GPIO/IAP 等实现细节核对 | `489d7c268263fa436605775ab7ebf5245427a49092f90a37a64904a29d60f202` |
| `stc/stc8g-stc8h-lib-demo-code.rar` | STC 官方 | `https://www.stcmicro.com/rar/demo/stc8g-stc8h-lib-demo-code.rar` | STC8G/STC8H 官方库函数和独立示例，用于核对外设初始化顺序、寄存器位和示例资源用法；不直接照搬架构 | `95a759ea0cd5ec9e2e4d8183256da4ad7c7e863687c0bb1cfc0d36cfe926b703` |
| `ti/PCF8574_TI.pdf` | TI 官方 | `https://www.ti.com/lit/ds/symlink/pcf8574.pdf` | LCD1602 I2C 背包常用 PCF8574 I/O 扩展器，核对 I2C 地址、准双向 I/O、上电状态和 ACK 行为 | `f0ee68a2517d84256fc3d32084f6d35fd9fc442ff64fd9f4c90460781ef30ee4` |
| `lcd/HD44780_Hitachi_via_Adafruit.pdf` | Hitachi datasheet 权威镜像 | `https://cdn-shop.adafruit.com/datasheets/HD44780.pdf` | LCD1602 兼容控制器命令、初始化时序、DDRAM 地址和 4-bit 模式核对 | `4324d67ec14fc8e7ba8cf7db7256749c9b80b94b3f8bedfa614a0d2839c7dea9` |
| `encoder/ALPS_Alpine_EC11E_Series.pdf` | Alps Alpine 官方 | `https://tech.alpsalpine.com/cms.media/product_catalog_ec_01_ec11e_en_611f078659.pdf` | EC11 系列旋转编码器两相 A/B 输出、额定参数、定位数/脉冲数核对；具体器件以实物和实际型号为准 | `08dae9fa97a2a97d9c9954e87455aff6f0cc557be1fb84fcb0f11608937ba241` |

下载日期：2026-05-10。
