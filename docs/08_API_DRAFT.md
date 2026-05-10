# 第一版 API 草案

## 1. 草案目标

本文件定义第一版接口方向。

实际实现时可以根据 STC8H1K08 头文件、SDCC/Keil 差异和编译结果微调，但不能违背以下约束：

- 不使用动态内存。
- 不引入运行期适配层。
- 不使用自动注册。
- 未使用模块不占资源。
- 高频函数尽量短小。
- 编译器差异只放在 `core` 层。

## 2. Core

### 2.1 `stc8h_compiler`

职责：

- 隔离 SDCC 和 Keil C51 关键字差异。
- 提供统一的存储类型、位类型和中断声明宏。

接口方向：

```c
#define STC8H_DATA
#define STC8H_IDATA
#define STC8H_PDATA
#define STC8H_XDATA
#define STC8H_CODE
#define STC8H_BIT

#define STC8H_SFR(name, addr)
#define STC8H_SBIT(name, addr, bit)
#define STC8H_SFRX(addr)
#define STC8H_SFR16X(addr)

#define STC8H_INTERRUPT(name, vector)
#define STC8H_INTERRUPT_USING(name, vector, reg_bank)
#define STC8H_NOP()
```

实现要求：

- 全部为编译期宏。
- 不生成任何代码。
- 不分配任何存储。
- 业务模块、HAL 模块、驱动模块不得直接使用 SDCC 或 Keil 专属关键字。
- `sfr`、`sbit`、中断声明、存储区关键字只能通过本模块宏出现。
- 普通 SFR 优先使用 STC 官方或项目确认的芯片头文件定义。
- 兼容宏只用于隔离编译器差异和必要的扩展 SFR 访问，不重新定义整套寄存器。
- 公共 API 不返回编译器专属 `bit` 类型，统一使用 `stc8h_u8` 表示布尔值。
- 公共 API 的字符串参数默认指向 RAM；如需 `code` 区字符串，单独提供带 `_code` 后缀的函数或宏。
- ISR 声明必须通过 `STC8H_INTERRUPT` 或 `STC8H_INTERRUPT_USING`。

示例方向：

```c
STC8H_INTERRUPT(timer0_isr, STC8H_VECTOR_TIMER0)
{
    /* ISR body */
}

STC8H_INTERRUPT_USING(timer0_isr, STC8H_VECTOR_TIMER0, 1)
{
    /* ISR body using register bank 1 */
}
```

展开目标：

```c
/* Keil C51 */
void timer0_isr(void) interrupt STC8H_VECTOR_TIMER0
void timer0_isr(void) interrupt STC8H_VECTOR_TIMER0 using 1

/* SDCC */
void timer0_isr(void) __interrupt(STC8H_VECTOR_TIMER0)
```

如果 SDCC 目标不支持 register bank 语法，`STC8H_INTERRUPT_USING` 在 SDCC 下退化为普通中断声明，并在文档中说明。

### 2.2 `stc8h_types`

职责：

- 提供基础整数类型和通用状态值。

接口方向：

```c
typedef unsigned char stc8h_u8;
typedef unsigned int stc8h_u16;
typedef unsigned long stc8h_u32;

typedef signed char stc8h_s8;
typedef signed int stc8h_s16;
typedef signed long stc8h_s32;

typedef enum {
    STC8H_OK = 0,
    STC8H_ERROR = 1,
    STC8H_TIMEOUT = 2,
    STC8H_BUSY = 3
} stc8h_status_t;
```

### 2.3 `stc8h_config`

职责：

- 提供基础库默认配置。
- 允许应用项目覆盖配置。

接口方向：

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_SYSCLK_HZ 11059200UL
```

实现要求：

- 默认值只作为示例和兜底。
- 应用项目可以通过自己的配置头文件覆盖。
- 覆盖入口为 `STC8H_CONFIG_INCLUDE`。
- 板级引脚宏入口为 `STC8H_PINS_INCLUDE`。
- 所有模块只包含 `stc8h_config.h`，不直接包含 `board_config.h` 或 `board_pins.h`。
- `stc8h_config.h` 负责按需包含项目配置和板级引脚宏。
- `stc8h_config.h` 顺序固定为：先包含项目配置，再填充默认值，最后包含板级引脚宏。

### 2.4 `stc8h_delay`

职责：

- 提供短延时。

接口方向：

```c
void stc8h_delay_us(stc8h_u16 us);
void stc8h_delay_ms(stc8h_u16 ms);
```

实现要求：

- 基于 `STC8H_SYSCLK_HZ`。
- 以小代码体积和可预期为主。
- 延时精度以外设驱动够用为目标，不追求计量级精度。

### 2.5 `stc8h_interrupt`

职责：

- 提供最小中断开关辅助。

接口方向：

```c
void stc8h_interrupt_enable_global(void);
void stc8h_interrupt_disable_global(void);
```

实现要求：

- 只处理全局开关。
- 具体外设中断由对应 HAL 模块处理。

## 3. HAL

### 3.1 `stc8h_gpio`

职责：

- GPIO 模式配置、读、写、翻转。

接口方向：

```c
typedef enum {
    STC8H_GPIO_MODE_QUASI = 0,
    STC8H_GPIO_MODE_PUSH_PULL,
    STC8H_GPIO_MODE_INPUT_ONLY,
    STC8H_GPIO_MODE_OPEN_DRAIN
} stc8h_gpio_mode_t;

void stc8h_gpio_set_mode(stc8h_u8 port, stc8h_u8 pin, stc8h_gpio_mode_t mode);
void stc8h_gpio_write(stc8h_u8 port, stc8h_u8 pin, stc8h_u8 value);
stc8h_u8 stc8h_gpio_read(stc8h_u8 port, stc8h_u8 pin);
void stc8h_gpio_toggle(stc8h_u8 port, stc8h_u8 pin);
```

取舍：

- 端口和引脚使用小整数，方便板级配置。
- `port + pin` 接口主要用于低频初始化、调试和非关键路径。
- 高频或时序敏感路径优先使用板级宏或内联函数绑定具体引脚。
- 软件 I2C、TM1637、红外、EC11 扫描等路径不应依赖运行期端口分派。

### 3.2 `stc8h_timer`

职责：

- 配置基础定时器。
- 支撑软件定时器和常见周期扫描。

接口方向：

```c
typedef enum {
    STC8H_TIMER0 = 0,
    STC8H_TIMER1,
    STC8H_TIMER2
} stc8h_timer_id_t;

stc8h_status_t stc8h_timer_init_1ms(stc8h_timer_id_t timer);
void stc8h_timer_start(stc8h_timer_id_t timer);
void stc8h_timer_stop(stc8h_timer_id_t timer);
void stc8h_timer_enable_interrupt(stc8h_timer_id_t timer);
void stc8h_timer_disable_interrupt(stc8h_timer_id_t timer);
void stc8h_timer_clear_flag(stc8h_timer_id_t timer);
```

取舍：

- 第一版优先满足 Timer0 1ms tick 和简单周期任务。
- 不做通用复杂定时器框架。
- 默认资源建议：Timer0 用作 1ms tick，Timer2 用于红外脉宽计时或备用载波。
- UART 波特率相关资源由 `stc8h_uart` 管理，具体是否占用某个 Timer 以 STC8H1K08 手册和实现为准。
- 具体分配以板级配置为准，未使用模块不初始化对应 Timer。
- 第一版 ISR 由示例或板级文件绑定。
- Timer HAL 只提供装载、启动、停止、清标志和必要的计数读取能力。
- 如果示例提供全局 tick 计数，ISR 向量和全局变量必须在资源声明中列明。
- 当前实现只初始化 Timer0 1ms tick；Timer1 保留给 UART1，Timer2 预留给红外脉宽测量或备用载波。

### 3.3 `stc8h_uart`

职责：

- UART 初始化、阻塞发送、可选接收。

接口方向：

```c
typedef enum {
    STC8H_UART1 = 0,
    STC8H_UART2
} stc8h_uart_id_t;

stc8h_status_t stc8h_uart_init(stc8h_uart_id_t uart);
void stc8h_uart_putc(stc8h_uart_id_t uart, char ch);
void stc8h_uart_write(stc8h_uart_id_t uart, const char *data);
void stc8h_uart_write_code(stc8h_uart_id_t uart, const STC8H_CODE char *data);
stc8h_u8 stc8h_uart_readable(stc8h_uart_id_t uart);
char stc8h_uart_getc(stc8h_uart_id_t uart);
```

取舍：

- 默认提供阻塞发送，代码最小。
- 接收缓冲使用 `util_ring_buffer` 时才引入。
- `stc8h_uart_write` 默认接收 RAM 字符串。
- `stc8h_uart_write_code` 用于 `code` 区固定字符串。
- UART 初始化使用 `STC8H_UART1_BAUD` 和编译期 reload，不做运行期波特率计算。

### 3.4 `stc8h_i2c`

职责：

- I2C 主机模式基础读写。

接口方向：

```c
void stc8h_i2c_init(void);
stc8h_status_t stc8h_i2c_write(stc8h_u8 addr7, const stc8h_u8 *data, stc8h_u8 len);
stc8h_status_t stc8h_i2c_read(stc8h_u8 addr7, stc8h_u8 *data, stc8h_u8 len);
```

取舍：

- 第一版优先主机模式。
- 不做复杂多主机或从机框架。
- 第一版优先软件 I2C 单总线。
- SDA/SCL 由 `board_pins.h` 提供编译期宏绑定。
- 默认目标速率为标准模式 100kHz 以内，具体速度由延时宏和示波器实测校准。
- 不做运行期 I2C bus 对象，不保存总线引脚结构体。
- 公共 API 使用 7-bit 未左移地址 `addr7`，例如 LCD1602 常见地址 `0x27` 或 `0x3F`。
- I2C 起始后的地址字节由底层内部左移并添加 R/W 位。

### 3.5 `stc8h_spi`

职责：

- SPI 主机模式基础收发。

接口方向：

```c
void stc8h_spi_init(void);
stc8h_u8 stc8h_spi_transfer(stc8h_u8 value);
void stc8h_spi_write(const STC8H_DATA stc8h_u8 *data, stc8h_u8 len);
```

取舍：

- 第一版只承诺 SPI 基础收发能力。
- 第一版冻结为硬件 SPI 主机轮询实现，不维护软件 SPI 双实现。
- 默认使用 P1.3/P1.4/P1.5 引脚组。
- 硬件 SS 使用 `SSIG=1` 忽略，不占用当前 P1.2 LED；片选由板级或应用代码自行控制。
- 默认 SPI mode 0，MSB first，主机模式，`SYSclk/4`。
- 不启用 SPI 中断和 DMA。

### 3.6 `stc8h_pwm`

职责：

- PWM 输出配置和占空比设置。

接口方向：

```c
stc8h_status_t stc8h_pwm_init(stc8h_u8 channel, stc8h_u16 period);
stc8h_status_t stc8h_pwm_set_duty(stc8h_u8 channel, stc8h_u16 duty);
stc8h_status_t stc8h_pwm_enable(stc8h_u8 channel);
stc8h_status_t stc8h_pwm_disable(stc8h_u8 channel);
```

取舍：

- 第一版实现 `PWMA` 1..4 的基础 PWM mode 1 输出。
- 默认引脚选择为官方默认 P1 组：PWM1P/P1.0、PWM2P/P1.2、PWM3P/P1.4、PWM4P/P1.6。
- `PWMA` 1..4 共用同一个周期；重新初始化任一通道会更新 `PWMA` 周期。
- 第一版不实现互补输出、死区、刹车、中断、PWMB 和运行期引脚自动适配。
- 用于红外发射载波和无源蜂鸣器时，必须在资源策略中声明占用。

### 3.7 `stc8h_adc`

职责：

- ADC 单通道采样。

接口方向：

```c
void stc8h_adc_init(void);
stc8h_u16 stc8h_adc_read(stc8h_u8 channel);
```

实现约束：

- 第一版按 STC8H1K08 10-bit ADC 实现。
- 默认使用右对齐结果，返回范围 `0..1023`。
- ADC 初始化按官方建议设置 `ADCTIM=0x3f`，ADC 时钟默认 `SYSclk/2/16`，上电后等待约 1ms。
- 应用或板级代码必须把 ADC 引脚配置为高阻输入，并关闭不需要的数字输入和上拉。
- 无效通道或转换超时返回 `STC8H_ADC_INVALID_VALUE`。

### 3.8 `stc8h_eeprom`

职责：

- EEPROM/IAP 参数读写。

接口方向：

```c
stc8h_status_t stc8h_eeprom_read(stc8h_u16 addr, STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
stc8h_status_t stc8h_eeprom_write(stc8h_u16 addr, const STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
stc8h_status_t stc8h_eeprom_erase_sector(stc8h_u16 addr);
```

实现约束：

- STC8H1K08 EEPROM/IAP 按官方资料确认为 4KB，地址范围 `0x0000..0x0FFF`。
- 擦除粒度为 512 字节扇区，`stc8h_eeprom_erase_sector()` 的地址必须 512 字节对齐。
- 读写函数不隐式擦除；写入前由调用方明确擦除对应扇区。
- IAP 触发按官方流程写 `0x5A`、`0xA5`，触发窗口内临时关闭全局中断，完成后恢复。
- `IAP_TPS` 按系统时钟编译期确定；第一版默认支持 `11.0592MHz` 和 `12MHz`。
- 遵守 `docs/14_EEPROM_IAP_POLICY.md`。
- 示例必须显式定义测试地址范围，默认构建不执行写擦。

## 4. Drivers

### 4.1 `drv_button`

职责：

- 按键消抖。
- 短按、长按、释放事件。

接口方向：

```c
typedef enum {
    DRV_BUTTON_EVENT_NONE = 0,
    DRV_BUTTON_EVENT_PRESS,
    DRV_BUTTON_EVENT_RELEASE,
    DRV_BUTTON_EVENT_SHORT,
    DRV_BUTTON_EVENT_LONG
} drv_button_event_t;

typedef struct {
    stc8h_u8 active_level;
    stc8h_u8 stable_state;
    stc8h_u8 last_raw;
    stc8h_u8 long_reported;
    drv_button_event_t event;
    stc8h_u16 debounce_ms;
    stc8h_u16 press_ms;
    stc8h_u16 long_press_ms;
} drv_button_t;

void drv_button_init(drv_button_t *button, stc8h_u8 active_level);
void drv_button_set_timing(drv_button_t *button, stc8h_u16 debounce_ms, stc8h_u16 long_press_ms);
void drv_button_scan(drv_button_t *button, stc8h_u8 raw_level, stc8h_u16 elapsed_ms);
drv_button_event_t drv_button_get_event(drv_button_t *button);
stc8h_u16 drv_button_get_press_ms(const drv_button_t *button);
```

取舍：

- 状态由调用方持有，RAM 占用显式。
- 扫描频率由应用项目决定。
- 驱动不保存 `port/pin`，不直接读 GPIO。
- 板级代码或示例通过编译期宏读取引脚，然后把 `raw_level` 传入扫描函数。
- 默认消抖时间由 `DRV_BUTTON_DEBOUNCE_MS` 配置，建议 10ms。
- 默认长按阈值由 `DRV_BUTTON_LONG_PRESS_MS` 配置，建议 800ms。
- `drv_button_set_timing` 可在运行中修改当前按钮对象的消抖时间和长按阈值。
- 长按事件在稳定按下超过阈值时输出；如果主循环未及时取到事件，释放时仍应避免被误判为短按。
- `drv_button_get_event` 读取后清除当前事件。

### 4.2 `drv_ec11`

职责：

- EC11 A/B 相旋转方向识别。
- 可选按键通过 `drv_button` 组合实现。

接口方向：

```c
typedef struct {
    stc8h_u8 last_state;
    stc8h_u8 has_last_detent;
    stc8h_u8 reverse;
    stc8h_u8 steps_per_detent;
    stc8h_s8 step_accum;
    stc8h_s8 fast_step;
    stc8h_u16 detent_elapsed_ms;
    stc8h_u16 fast_threshold_ms;
    stc8h_s16 delta;
} drv_ec11_t;

void drv_ec11_init(drv_ec11_t *ec11);
void drv_ec11_set_fast(drv_ec11_t *ec11, stc8h_u16 threshold_ms, stc8h_s8 fast_step);
void drv_ec11_set_reverse(drv_ec11_t *ec11, stc8h_u8 reverse);
void drv_ec11_set_steps_per_detent(drv_ec11_t *ec11, stc8h_u8 steps_per_detent);
void drv_ec11_scan(drv_ec11_t *ec11, stc8h_u8 a_level, stc8h_u8 b_level, stc8h_u16 elapsed_ms);
stc8h_s16 drv_ec11_get_delta(drv_ec11_t *ec11);
```

取舍：

- 驱动不保存 A/B 相引脚。
- 板级代码通过编译期宏读取 A/B 电平后传入。
- 这样 EC11 高频扫描路径不依赖运行期端口分派。
- 默认每个定位格输出 `+1` 或 `-1`。
- 支持轻量加速：两次有效定位格间隔小于等于 `DRV_EC11_FAST_THRESHOLD_MS` 时，当前定位格输出 `DRV_EC11_FAST_STEP` 倍增量。
- 默认 `DRV_EC11_FAST_THRESHOLD_MS=30ms`，`DRV_EC11_FAST_STEP=10`；两个值均可在项目配置中覆盖。
- 方向可通过 `DRV_EC11_REVERSE` 配置。驱动默认 `0`，当前演示板在 `board_config.h` 中配置为 `1`，使实测顺时针输出为正数。
- 运行中可用 `drv_ec11_set_fast` 修改当前对象的快速阈值和快速步进值。
- 运行中可用 `drv_ec11_set_reverse` 修改当前对象方向；通常只在初始化或用户设置变更时调用，不建议在旋转过程中频繁切换。
- 每定位格需要的有效相位跳变数可通过 `DRV_EC11_STEPS_PER_DETENT` 或 `drv_ec11_set_steps_per_detent` 配置，默认 `4`，可选范围 `1..4`。如果旋转不灵敏，可降低；如果一次旋转计数过多，可提高。
- 加速只影响输出增量，不改变 A/B 相解码逻辑。
- `drv_ec11_get_delta` 读取后清零累计增量。

### 4.3 `drv_lcd1602`

职责：

- I2C LCD1602 初始化。
- 写命令、写数据、定位、写字符串。

接口方向：

```c
void drv_lcd1602_init(stc8h_u8 addr7);
void drv_lcd1602_clear(void);
void drv_lcd1602_set_cursor(stc8h_u8 row, stc8h_u8 col);
void drv_lcd1602_putc(char ch);
void drv_lcd1602_write_string(const char *text);
void drv_lcd1602_write_string_code(const STC8H_CODE char *text);
```

取舍：

- 第一版面向 I2C 转接板，常见 4 线连接：`VCC`、`GND`、`SDA`、`SCL`。
- LCD1602 本体通常仍是 4 位并口模式，但由 I2C 转接芯片完成引脚扩展。
- I2C 地址由初始化参数 `addr7` 或板级配置提供，使用 7-bit 未左移地址，默认 `0x27`，备用 `0x3F`。
- I2C 底层依赖 `stc8h_i2c`。
- 第一版默认支持常见 PCF8574 I2C 背包映射。
- RS/RW/EN/BL/D4-D7 位通过编译期宏配置，默认不做运行期映射对象。

默认映射方向：

```c
#define DRV_LCD1602_I2C_BIT_RS 0
#define DRV_LCD1602_I2C_BIT_RW 1
#define DRV_LCD1602_I2C_BIT_EN 2
#define DRV_LCD1602_I2C_BIT_BL 3
#define DRV_LCD1602_I2C_BIT_D4 4
#define DRV_LCD1602_I2C_BIT_D5 5
#define DRV_LCD1602_I2C_BIT_D6 6
#define DRV_LCD1602_I2C_BIT_D7 7
```

- 默认不读 LCD busy flag，使用固定延时满足时序。
- `drv_lcd1602_write_string` 默认接收 RAM 字符串。
- `drv_lcd1602_write_string_code` 用于 `code` 区固定字符串。

### 4.4 `drv_led`

职责：

- LED 有效电平计算和状态辅助。

接口方向：

```c
typedef struct {
    stc8h_u8 active_level;
} drv_led_t;

void drv_led_init(drv_led_t *led, stc8h_u8 active_level);
stc8h_u8 drv_led_level_on(const drv_led_t *led);
stc8h_u8 drv_led_level_off(const drv_led_t *led);
```

取舍：

- LED 驱动只计算逻辑电平，不直接操作引脚。
- 板级宏负责实际写 GPIO。
- 开关、翻转、闪烁放在示例或板级代码中实现。

### 4.5 `drv_buzzer`

职责：

- 有源蜂鸣器开关。
- 无源蜂鸣器可通过 PWM 模块驱动。

接口方向：

```c
typedef struct {
    stc8h_u8 active_level;
} drv_buzzer_t;

void drv_buzzer_init(drv_buzzer_t *buzzer, stc8h_u8 active_level);
stc8h_u8 drv_buzzer_level_on(const drv_buzzer_t *buzzer);
stc8h_u8 drv_buzzer_level_off(const drv_buzzer_t *buzzer);
```

取舍：

- 有源蜂鸣器驱动只提供电平逻辑。
- 实际 GPIO 输出由板级宏完成。
- 无源蜂鸣器通过 PWM 模块驱动，不在本驱动中混入 PWM 控制。

### 4.6 `drv_relay`

职责：

- 继电器开关控制。

接口方向：

```c
typedef struct {
    stc8h_u8 active_level;
} drv_relay_t;

void drv_relay_init(drv_relay_t *relay, stc8h_u8 active_level);
stc8h_u8 drv_relay_level_on(const drv_relay_t *relay);
stc8h_u8 drv_relay_level_off(const drv_relay_t *relay);
```

取舍：

- 继电器驱动只提供有效电平计算。
- 实际 GPIO 输出由板级宏完成。

### 4.7 `drv_tm1637`

职责：

- TM1637 数码管显示。

接口方向：

```c
void drv_tm1637_init(void);
void drv_tm1637_set_brightness(stc8h_u8 brightness);
void drv_tm1637_set_display(stc8h_u8 on);
stc8h_status_t drv_tm1637_display_raw(const stc8h_u8 *segments, stc8h_u8 len);
stc8h_status_t drv_tm1637_display_digits(const stc8h_u8 *digits, stc8h_u8 len);
stc8h_status_t drv_tm1637_display_number(stc8h_s16 value);
stc8h_u8 drv_tm1637_encode_digit(stc8h_u8 digit);
stc8h_status_t drv_tm1637_clear(void);
```

取舍：

- 第一版优先常见 4 位数码管。
- 引脚映射通过板级配置宏提供。
- TM1637 不是 I2C，不复用软件 I2C API。
- 写命令和写显示数据检查 ACK，ACK 失败返回 `STC8H_ERROR`。
- `drv_tm1637_display_number()` 不使用除法库，十进制拆分用小循环换取 ROM 可控。

### 4.8 `drv_ir_tx`

职责：

- 红外遥控发射。
- 第一版优先支持 NEC 协议。
- 支持发送地址和命令。

接口方向：

```c
void drv_ir_tx_init(void);
void drv_ir_tx_send_nec(stc8h_u16 address, stc8h_u8 command);
void drv_ir_tx_carrier_on(void);
void drv_ir_tx_carrier_off(void);
```

取舍：

- NEC 协议作为第一版默认协议。
- 38kHz 载波默认优先使用 PWM 产生。
- 如果硬件 PWM 引脚不适合测试板，允许使用 Timer 产生载波。
- 如果项目资源紧张，可以提供 GPIO 软件调制实现，但要在文档中说明 CPU 占用。
- 引脚映射通过板级配置宏提供。
- 不做通用红外协议框架，但模块边界保持独立，方便以后按需新增其他协议。

### 4.9 `drv_ir_rx`

职责：

- 红外遥控接收。
- 第一版优先支持 NEC 协议解码。
- 输出地址、命令和重复码事件。

接口方向：

```c
typedef enum {
    DRV_IR_RX_EVENT_NONE = 0,
    DRV_IR_RX_EVENT_FRAME,
    DRV_IR_RX_EVENT_REPEAT,
    DRV_IR_RX_EVENT_ERROR
} drv_ir_rx_event_t;

typedef struct {
    stc8h_u16 address;
    stc8h_u8 command;
    stc8h_u8 command_inv;
    stc8h_u8 repeat;
} drv_ir_rx_frame_t;

void drv_ir_rx_init(void);
void drv_ir_rx_feed_pulse(stc8h_u8 level, stc8h_u16 width_us);
drv_ir_rx_event_t drv_ir_rx_get_event(drv_ir_rx_frame_t *frame);
void drv_ir_rx_reset(void);
```

取舍：

- 接收头按 `VS1838B`/`HS0038` 这类常见解调接收头设计，通常输出解调后的低有效脉冲，驱动按脉宽解码。
- 脉宽测量默认使用外部中断边沿触发 + Timer 计时。
- 如果实际引脚不支持外部中断，允许固定周期采样实现，但必须说明采样周期和 CPU 占用。
- 第一版优先低 RAM 状态机，不保存完整原始波形数组。
- 不做学习型遥控码库，不做多协议自动识别，但保留独立模块边界，方便以后新增协议实现。
- 引脚映射和中断/定时器资源通过板级配置指定。
- ISR/Timer/采样代码属于板级绑定层，负责测量边沿间隔并调用 `drv_ir_rx_feed_pulse`。
- NEC 解码状态机不直接访问寄存器。

## 5. Utils

### 5.1 `util_ring_buffer`

职责：

- 小型字节环形缓冲。

接口方向：

```c
typedef struct {
    STC8H_DATA stc8h_u8 *buffer;
    stc8h_u8 size;
    stc8h_u8 head;
    stc8h_u8 tail;
} util_ring_buffer_t;

void util_ring_buffer_init(util_ring_buffer_t *rb, STC8H_DATA stc8h_u8 *buffer, stc8h_u8 size);
stc8h_u8 util_ring_buffer_push(util_ring_buffer_t *rb, stc8h_u8 value);
stc8h_u8 util_ring_buffer_pop(util_ring_buffer_t *rb, stc8h_u8 *value);
stc8h_u8 util_ring_buffer_available(const util_ring_buffer_t *rb);
```

取舍：

- 默认使用 internal DATA RAM 缓冲，避免通用指针带来的代码和周期成本。
- 缓冲采用保留一个空位的方式区分空/满，实际可存字节数为 `size - 1`。
- `size` 必须大于等于 2。
- 不使用 `%` 取模，避免拉入除法/取模库。
- 不内置中断保护；ISR 与主循环并发访问时，由调用方在读取多字节状态或批量操作时处理临界区。

### 5.2 `util_soft_timer`

职责：

- 基于 tick 的非阻塞时间判断。

接口方向：

```c
typedef struct {
    stc8h_u16 last;
    stc8h_u16 interval;
} util_soft_timer_t;

void util_soft_timer_start(util_soft_timer_t *timer, stc8h_u16 now, stc8h_u16 interval);
stc8h_u8 util_soft_timer_expired(util_soft_timer_t *timer, stc8h_u16 now);
```

取舍：

- 使用 16-bit tick，单个 timer 对象占 4 字节 RAM。
- 适合 1ms tick 下 65 秒以内的非阻塞间隔。
- 不支持长时间任务调度框架；更长时间由应用层按业务组合。

### 5.3 `util_crc`

职责：

- 常见 CRC/校验。

接口方向：

```c
stc8h_u8 util_checksum8(const STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
stc8h_u16 util_crc16_modbus(const STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
```

取舍：

- 第一版只做 RAM 数据校验，不做 `code` 指针重载。
- CRC16/MODBUS 使用逐位算法，不使用查表常量。
- 适合 EEPROM 参数、低速串口协议和小数据包校验。

### 5.4 `util_filter`

职责：

- 小型采样滤波。

接口方向：

```c
stc8h_u16 util_filter_limit_u16(stc8h_u16 value, stc8h_u16 min_value, stc8h_u16 max_value);
stc8h_u16 util_filter_iir_u16(stc8h_u16 current, stc8h_u16 input, stc8h_u8 shift);
```

取舍：

- 不做复杂滤波框架。
- 优先提供项目中最常重复的简单函数。
- 不做任意长度平均数组，避免除法库和额外样本缓存。
- `util_filter_iir_u16` 使用移位平滑，适合 ADC、电位器和慢速传感器输入。
