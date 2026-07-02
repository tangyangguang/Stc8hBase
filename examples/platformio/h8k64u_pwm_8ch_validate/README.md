# H8K64U 8 路 PWM 验证

这个 PlatformIO 示例只用于 `STC8H8K64U-45I-LQFP48` 的 PWM 基础能力硬件验证，不包含业务逻辑。

## 覆盖范围

- 通过 `board/stc8h8k64u_lqfp48_base` 显式选择 `STC8H8K64U`。
- 初始化 8 路 PWM P 输出通道：
  - `PWMA1..4`: P1.0, P1.2, P1.4, P1.6.
  - `PWMB5..8`: P2.0, P2.1, P2.2, P2.3.
- 两组使用明确不同的周期：
  - `PWMA`: `1023` ticks.
  - `PWMB`: `2047` ticks.
- 循环执行 `stc8h_pwm_set_duty()`、`stc8h_pwm_enable()` 和
  `stc8h_pwm_disable()`，便于示波器或逻辑分析仪观察。

## 共用周期限制

STC8H 高级 PWM 按组共享周期：

- `PWMA1..4` 共用一个 `PWMA` 周期和预分频。
- `PWMB5..8` 共用一个 `PWMB` 周期和预分频。
- `PWMA` 与 `PWMB` 可以使用不同周期，但同一组内的通道不能独立设置 PWM 频率。

本示例直接按这个限制配置：所有 `PWMA` 通道使用 `1023` ticks，所有 `PWMB` 通道使用 `2047` ticks。
