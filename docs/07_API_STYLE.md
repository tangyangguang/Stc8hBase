# API 风格

## 1. 通用规则

- 公共头文件保持简短。
- 配置项必须可见。
- 不使用动态内存分配。
- 除非能明显减少重复代码，否则避免回调。
- 默认避免函数指针接口。
- 不可能合理失败的操作使用 `void` 返回。
- 正常运行中可能失败的操作使用小型状态枚举。
- API 设计要方便新增模块，但不能为了扩展性引入运行期开销。
- 未使用模块不能因为公共头文件或自动注册机制产生资源占用。
- 高频或时序敏感路径优先使用编译期绑定的板级宏或内联函数。
- `port + pin` 运行期参数接口只用于低频、调试或非关键路径。

## 2. 头文件布局

推荐头文件顺序：

1. Include guard。
2. 必要 include。
3. 公共宏。
4. 公共类型。
5. 公共函数。

公共头文件不定义可分配存储的全局对象。需要存储的对象应在 `.c` 文件中定义，或由调用方显式提供。

## 3. 配置方式

固件固定的选择使用项目级配置。

示例：

```c
#define STC8H_SYSCLK_HZ 11059200UL
#define STC8H_UART1_BAUD 115200UL
#define DRV_BUTTON_SCAN_PERIOD_MS 10u
```

启动后不会变化的值，避免做成每次调用都传入。

唯一配置入口为 `core/stc8h_config.h`。

推荐覆盖机制：

```c
#ifdef STC8H_CONFIG_INCLUDE
#include STC8H_CONFIG_INCLUDE
#endif

#ifndef STC8H_SYSCLK_HZ
#define STC8H_SYSCLK_HZ 11059200UL
#endif

#ifdef STC8H_PINS_INCLUDE
#include STC8H_PINS_INCLUDE
#endif
```

Makefile、PlatformIO、Keil 工程通过编译宏定义：

```c
#define STC8H_CONFIG_INCLUDE "board_config.h"
#define STC8H_PINS_INCLUDE "board_pins.h"
```

所有模块只包含 `stc8h_config.h`，不直接包含项目配置文件或板级引脚文件。需要板级引脚宏的模块，也通过 `stc8h_config.h` 间接获得 `STC8H_PINS_INCLUDE`。

顺序固定为：

1. 先包含项目配置 `STC8H_CONFIG_INCLUDE`。
2. 再用 `#ifndef` 提供基础库默认值。
3. 最后包含板级引脚宏 `STC8H_PINS_INCLUDE`。

## 4. 函数形态

优先使用简单函数形态：

```c
void stc8h_gpio_write(...);
stc8h_u8 stc8h_gpio_read(...);
void drv_button_scan(...);
unsigned char drv_button_get_event(...);
```

公共 API 不使用编译器专属 `bit` 类型。

字符串参数默认按 RAM 指针处理。需要直接输出 `code` 区字符串的模块，应提供 `_code` 后缀接口，避免字符串字面量导致 RAM 浪费或编译器差异。

高频路径可以使用板级宏形态：

```c
#define BOARD_I2C_SDA_HIGH()
#define BOARD_I2C_SDA_LOW()
#define BOARD_I2C_SCL_HIGH()
#define BOARD_I2C_SCL_LOW()
#define BOARD_I2C_SDA_READ()
```

这类宏由应用项目的 `board_pins.h` 提供，基础库驱动只调用宏，不保存运行期引脚对象。

按键、EC11、TM1637、软件 I2C、红外收发等高频或时序敏感模块，公共 API 不应要求驱动对象保存 `port/pin`。推荐由板级代码读取或写入具体引脚，再把电平值传给驱动状态机。

## 5. 对象归属

可复用驱动优先使用调用方持有状态：

```c
drv_button_t button;
drv_button_init(&button, ...);
drv_button_scan(&button);
```

这样 RAM 占用显式，也允许多个实例，而不是在模块内部隐藏全局数组。

硬件天然单例的模块，可以使用模块级状态。

8051 存储区选择必须显式。较大的缓冲区默认不放入内部 RAM，是否使用 `xdata` 由调用方或板级配置决定。

## 6. 阻塞与非阻塞

简单驱动中，如果延时是硬件时序必需且很短，可以使用阻塞函数。

例如：

- LCD1602 命令时序。
- 小的 GPIO 稳定等待。

较长或重复等待，应该在可行时提供非阻塞方案。

## 7. 中断

中断驱动 API 应该是可选能力。

默认模块尽可能支持轮询模式。

如果模块需要中断服务函数，文档必须说明：

- 使用哪个中断向量。
- 修改哪些全局状态。
- 需要什么扫描或服务频率。
