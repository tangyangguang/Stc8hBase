# STC 官方资料归档

本目录只保存本项目实现和硬件校验必须反复引用的 STC 官方资料。

本地资料总索引见 `docs/vendor/README.md`。

## 已归档文件

| 文件 | 来源 | 用途 | SHA-256 |
| --- | --- | --- | --- |
| `STC8H1K08_Features.pdf` | `https://www.stcmicro.com/datasheet/STC8H1K08_Features.pdf` | STC8H1K08 资源、封装、TSSOP20 引脚、GPIO/UART/I2C/ADC 等快速事实核对 | `fe83c09670fe1f87784d9afc4354f3c2f136c214f59f9c5ed6eb43e47120bcc6` |
| `STC8H8K64U_Features.pdf` | `https://www.stcmicro.com/datasheet/STC8H8K64U_Features.pdf` | STC8H8K64U 资源、LQFP48 引脚、UART2/UART3 引脚组、ADC 宽度、复位/下载注意事项快速事实核对 | `7b5e88e8b0fbb248cd839c4aeeae7b3c3078900055a222e2ff75df76b0ea8088` |
| `STC8H-en.pdf` | `https://www.stcmicro.com/datasheet/STC8H-en.pdf` | STC8H 系列寄存器、UART/Timer/GPIO/IAP 等实现细节核对 | `489d7c268263fa436605775ab7ebf5245427a49092f90a37a64904a29d60f202` |

下载日期：2026-05-10；`STC8H8K64U_Features.pdf` 下载日期：2026-06-16。

## 收录原则

- 官方 PDF 可作为硬件事实来源。
- 实现寄存器配置前，优先查本目录资料，再核对官网是否有新版。
- 不确定寄存器地址、位含义、引脚复用、电气模式、上拉/输入使能或时序公式时，不允许靠猜；先查本目录资料。
- 不收录 STC-ISP、STC-IDE、示例工程 ZIP、评估板原理图等非基础库实现必需资料。
- 如果以后加入新芯片或新外设，再按实际需要补充对应官方资料。
