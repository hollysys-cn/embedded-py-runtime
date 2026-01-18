# 技术研究：PLCopen 基础功能块库

**日期**: 2026-01-18
**功能**: PLCopen 基础功能块库
**分支**: `002-plcopen-function-blocks`

## 概要

本文档记录为 PLCopen 功能块库做出的技术决策、研究发现和替代方案分析。所有决策均基于工业控制最佳实践、嵌入式系统约束和项目 Constitution 要求。

---

## 决策 1: PID 微分项先行滤波（Derivative on Measurement）

### 决策内容

PID 控制器的微分项仅对过程测量值（PV）求微分，而不对误差（error = SP - PV）求微分。

### 数学表达

**传统微分**:
```
D_term = Kd * d(error)/dt = Kd * d(SP - PV)/dt
```

**微分项先行滤波**:
```
D_term = -Kd * d(PV)/dt
```

### 理由

1. **避免设定值突变冲击**: 当设定值（setpoint）发生阶跃变化时，传统微分项会产生无穷大的尖峰（理论上），导致输出剧烈振荡。
2. **工业标准实践**: 这是 PLCopen 和 IEC 61131-3 标准推荐的方法，广泛应用于商业PLC中。
3. **更好的扰动抑制**: 对测量值求微分可以有效响应负载扰动，而不会对设定值变化过度反应。
4. **平滑的控制输出**: 减少了输出跳变，延长执行机构寿命。

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| 标准微分（对误差） | 理论完整，教科书标准 | 设定值突变时输出尖峰 | 不适合工业应用 |
| 微分 + 低通滤波器 | 可平滑尖峰 | 增加滤波器参数调整复杂度 | 微分项先行更简洁 |
| 不实现微分项（仅PI） | 简化实现 | 失去快速响应能力 | 需求明确要求PID |

### 参考资料

- IEC 61131-3 标准第 2.5.3 节：Function Block 实现
- "A PID Controller with Derivative On Measurement", Astrom & Hagglund
- PLCopen Motion Control Part 1

---

## 决策 2: 条件积分法（Conditional Integration）实现抗饱和

### 决策内容

采用条件积分法实现积分抗饱和：当输出达到限幅时，停止积分项累加。

### 实现逻辑

```c
// 计算期望输出
float desired_output = P_term + I_term + D_term;

// 检查是否限幅
if (desired_output > out_max) {
    output = out_max;
    // 停止积分累加（不更新 integral）
} else if (desired_output < out_min) {
    output = out_min;
    // 停止积分累加
} else {
    output = desired_output;
    // 允许积分累加
    integral += Ki * error * sample_time;
}
```

### 理由

1. **简单高效**: 实现最简单，计算开销最小，适合实时嵌入式系统。
2. **有效防止饱和**: 输出限幅时积分项不再增长，避免积分器"饱和"。
3. **快速恢复**: 一旦输出离开限幅区域，积分器立即恢复工作。
4. **嵌入式友好**: 无需复杂的反馈计算或额外状态变量。

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| 反向计算法（Back Calculation） | 平滑的抗饱和响应 | 需要额外增益参数Kb调整 | 增加复杂度和调参负担 |
| 限幅追踪法（Tracking Mode） | 精确跟踪实际输出 | 实现复杂，需要反馈信号 | 过度设计 |
| 积分项独立限幅 | 简单 | 无法解决总输出饱和问题 | 效果不佳 |

### 参考资料

- "Anti-windup for PID Controllers", Karl Johan Astrom, 2005
- Embedded Control Systems Design: Windup and Anti-Windup

---

## 决策 3: PT1 滤波器使用前向欧拉离散化

### 决策内容

一阶惯性滤波器采用前向欧拉法（Forward Euler）进行离散化。

### 数学推导

连续时间传递函数:
```
H(s) = 1 / (τs + 1)
```

微分方程:
```
τ * dy/dt + y = u
```

前向欧拉离散化（Ts 为采样周期）:
```
τ * (y[k] - y[k-1])/Ts + y[k-1] = u[k-1]
```

整理得递推公式:
```
α = Ts / (τ + Ts)
y[k] = y[k-1] + α * (u[k-1] - y[k-1])
    = (1 - α) * y[k-1] + α * u[k-1]
```

### 理由

1. **计算简单**: 仅需一次乘法和两次加法，执行时间 < 1μs。
2. **数值稳定**: 当 Ts << τ 时（过采样条件），稳定且精确。
3. **嵌入式友好**: 无需复杂的三角函数或指数运算。
4. **内存占用小**: 仅需保存一个前值 y[k-1]。

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| 双线性变换（Tustin） | 更高精度，稳定性更好 | 计算复杂度略高 | 对控制应用精度已够用 |
| 后向欧拉 | 无条件稳定 | 需要求解方程，复杂 | 前向欧拉已满足稳定性 |
| 精确离散化（ZOH） | 理论最优 | 需要指数函数，开销大 | 性能不划算 |

### 稳定性分析

稳定条件: `0 < α < 2`

由 `α = Ts / (τ + Ts)` 可知:
- 当 `Ts > 0` 且 `τ > 0` 时，`0 < α < 1` 恒成立
- 因此系统始终稳定

### 参考资料

- Digital Control Engineering, Fadali & Visioli
- Discrete-Time Signal Processing, Oppenheim

---

## 决策 4: 使用最小有效值 1e-6 进行除零保护

### 决策内容

涉及除法运算的参数（时间常数、采样周期）当小于 `1e-6` 时，强制使用 `1e-6` 替代。

### 实现示例

```c
#define MIN_VALID_VALUE 1e-6f

float safe_divide(float numerator, float denominator) {
    if (fabsf(denominator) < MIN_VALID_VALUE) {
        denominator = (denominator >= 0) ? MIN_VALID_VALUE : -MIN_VALID_VALUE;
    }
    return numerator / denominator;
}
```

### 理由

1. **避免除零**: 彻底消除除零错误。
2. **保持功能块可用**: 即使参数异常，系统仍可运行（虽然结果可能不准确）。
3. **合理的下限**: `1e-6` 秒（1微秒）对于 Cortex-M4 @168MHz 是合理的最小时间尺度。
4. **可预测行为**: 相比返回错误，替代值提供确定性行为。

### 最小值选择依据

- **时间常数**: 1微秒对应 1MHz 带宽，远超传感器和执行机构的响应速度
- **采样周期**: 168MHz CPU 的一个时钟周期约 6ns，1μs = 168 个时钟周期，足够执行功能块
- **浮点精度**: 单精度 float 的精度约 7 位有效数字，`1e-6` 仍在有效精度范围内

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| 返回错误并停止 | 明确指示错误 | 系统停止工作，影响可用性 | 工业系统需高可用性 |
| 使用默认值（如 1.0） | 简单 | 默认值可能不合理 | 可能导致意外行为 |
| 使用更小的值（1e-9） | 更接近真实零 | 接近浮点精度极限，不稳定 | 数值稳定性问题 |

### 参考资料

- IEEE 754 浮点标准
- Numerical Recipes in C, Press et al.

---

## 决策 5: NaN/Inf 检测后输出零并设置错误标志

### 决策内容

功能块执行时检测输入是否为 NaN 或 Inf，若是则输出零值并设置 `FB_STATUS_ERROR_NAN` 或 `FB_STATUS_ERROR_INF` 标志。

### 实现方法

```c
#include <math.h>

float FB_PID_Execute(FB_PID_t* fb, float setpoint, float measurement) {
    // 检测 NaN/Inf
    if (isnan(setpoint) || isnan(measurement)) {
        fb->state.status = FB_STATUS_ERROR_NAN;
        return 0.0f;  // 安全的中性输出
    }
    if (isinf(setpoint) || isinf(measurement)) {
        fb->state.status = FB_STATUS_ERROR_INF;
        return 0.0f;
    }

    // 正常执行...
}
```

### 理由

1. **安全的默认输出**: 零值是最安全的中性输出，不会导致执行机构过度动作。
2. **提供诊断信息**: 错误标志通知上层应用异常发生，便于故障排查。
3. **系统可继续运行**: 不会因单次异常数据导致系统崩溃。
4. **符合工业实践**: 传感器故障时输出安全值是工业控制的标准做法。

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| 保持最后有效值 | 继续使用历史数据 | 可能使用陈旧数据，隐藏故障 | 安全性较低 |
| 将 NaN 视为零处理 | 静默处理 | 无诊断信息，难以调试 | 缺乏可观测性 |
| 停止功能块工作 | 最保守 | 系统完全停止，可用性差 | 过于激进 |

### 参考资料

- ISO 26262 功能安全标准
- IEC 61508 安全相关系统

---

## 决策 6: 首次执行使用输入作为初始输出（无跳变启动）

### 决策内容

所有动态功能块（PT1、积分器）在首次执行时，使用首次输入值作为初始输出，实现平滑启动。

### 实现示例：PT1 滤波器

```c
float FB_PT1_Execute(FB_PT1_t* fb, float input) {
    if (fb->state.first_run) {
        fb->state.output = input;  // 首次调用：输出 = 输入
        fb->state.first_run = false;
        return input;
    }

    // 正常滤波
    float alpha = fb->config.sample_time /
                  (fb->config.time_constant + fb->config.sample_time);
    fb->state.output += alpha * (input - fb->state.output);
    return fb->state.output;
}
```

### 实现示例：PID 控制器

```c
float FB_PID_Execute(FB_PID_t* fb, float setpoint, float measurement) {
    if (fb->state.first_run) {
        fb->state.prev_measurement = measurement;
        fb->state.integral = 0.0f;  // 或根据初始输出反推
        fb->state.first_run = false;
    }
    // ...
}
```

### 理由

1. **避免冷启动冲击**: 输出从零跳变到实际值会对执行机构产生冲击。
2. **平滑过渡**: 系统启动时的瞬态响应更平滑。
3. **符合物理直觉**: 滤波器初始状态应反映当前测量值，而非零。
4. **工业标准**: 商业PLC中的标准做法。

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| 输出从零开始 | 实现简单 | 启动时输出跳变 | 对执行机构不友好 |
| 用户提供初始值 | 灵活 | 增加使用复杂度 | 大多数情况不需要 |
| 首次仅初始化 | 延迟一个周期 | 响应延迟 | 不必要 |

### 参考资料

- Industrial Control Handbook, Kilian
- PLC Programming using RSLogix 5000

---

## 决策 7: 初始化时验证配置参数

### 决策内容

所有功能块的 `_Init()` 函数必须验证配置参数的有效性，无效配置返回错误码并阻止功能块启动。

### 验证规则

#### PID 控制器
```c
int FB_PID_ValidateConfig(const FB_PID_Config_t* config) {
    if (config->sample_time <= 0.0f || config->sample_time > 1000.0f) {
        return -1;  // 采样周期无效
    }
    if (config->out_max <= config->out_min) {
        return -2;  // 输出限幅无效
    }
    if (config->int_max <= config->int_min) {
        return -3;  // 积分限幅无效
    }
    return 0;  // 配置有效
}
```

#### PT1 滤波器
```c
int FB_PT1_ValidateConfig(const FB_PT1_Config_t* config) {
    if (config->time_constant < 1e-6f) {
        return -1;  // 时间常数过小
    }
    if (config->sample_time <= 0.0f || config->sample_time > 1000.0f) {
        return -2;  // 采样周期无效
    }
    return 0;
}
```

### 理由

1. **早期错误检测**: 在配置阶段发现问题，而非运行时。
2. **明确的错误信息**: 返回码指示具体的配置错误。
3. **防止系统以错误状态运行**: 避免不可预测的行为。
4. **符合防御性编程**: 不信任用户输入。

### 参考资料

- Code Complete, Steve McConnell
- Embedded C Coding Standard, BARR Group

---

## 决策 8: Unity 测试框架用于单元测试

### 决策内容

使用 Unity 嵌入式 C 测试框架进行所有功能块的单元测试。

### Unity 特性

- **轻量级**: 核心代码 < 10KB
- **无依赖**: 不需要操作系统或标准库（可选）
- **嵌入式友好**: 可在目标硬件上运行
- **丰富的断言**: 支持整数、浮点、字符串、数组比较
- **MIT 许可证**: 商业友好

### 集成方式

```cmake
# CMakeLists.txt
add_subdirectory(.toolchain/unity)

add_executable(test_fb_pid
    tests/plcopen/test_fb_pid.c
    src/plcopen/fb_pid.c
)
target_link_libraries(test_fb_pid unity)
```

### 测试示例

```c
#include "unity.h"
#include "plcopen/fb_pid.h"

void setUp(void) {
    // 每个测试前执行
}

void tearDown(void) {
    // 每个测试后执行
}

void test_PID_Init_ValidConfig(void) {
    FB_PID_t pid;
    FB_PID_Config_t config = {
        .kp = 1.0f,
        .ki = 0.1f,
        .kd = 0.05f,
        .sample_time = 0.01f,
        .out_min = 0.0f,
        .out_max = 100.0f,
        .int_min = -50.0f,
        .int_max = 50.0f
    };

    int result = FB_PID_Init(&pid, &config);
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_PID_Execute_ProportionalAction(void) {
    // ... 测试比例作用
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_PID_Init_ValidConfig);
    RUN_TEST(test_PID_Execute_ProportionalAction);
    return UNITY_END();
}
```

### 理由

1. **专为嵌入式设计**: 可在资源受限环境运行。
2. **广泛使用**: 嵌入式社区事实标准。
3. **简单易用**: 学习曲线平缓。
4. **可在目标硬件运行**: 真实环境测试。

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| Check | 功能完整 | 需要 POSIX，不适合嵌入式 | 依赖过重 |
| CppUTest | C++ 友好 | 需要 C++ 编译器 | 项目使用 C11 |
| 自定义测试宏 | 极简 | 缺乏标准报告格式 | 缺少工具支持 |

### 参考资料

- Unity GitHub: https://github.com/ThrowTheSwitch/Unity
- Test-Driven Development for Embedded C, James Grenning

---

## 决策 9: 文件组织 - 每个功能块独立文件

### 决策内容

每个功能块使用独立的 `.c` 和 `.h` 文件，通用函数放在 `common.c` 和 `common.h`。

### 文件映射

| 功能块 | 源文件 | 头文件 |
|--------|--------|--------|
| PID | `src/plcopen/fb_pid.c` | `include/plcopen/fb_pid.h` |
| PT1 | `src/plcopen/fb_pt1.c` | `include/plcopen/fb_pt1.h` |
| 通用 | `src/plcopen/common.c` | `include/plcopen/common.h` |

### 主头文件 `plcopen.h`

```c
#ifndef PLCOPEN_H
#define PLCOPEN_H

// 包含所有功能块
#include "plcopen/fb_pid.h"
#include "plcopen/fb_pt1.h"
#include "plcopen/fb_ramp.h"
#include "plcopen/fb_limit.h"
#include "plcopen/fb_deadband.h"
#include "plcopen/fb_integrator.h"
#include "plcopen/fb_derivative.h"
#include "plcopen/common.h"

#endif // PLCOPEN_H
```

用户只需 `#include "plcopen.h"` 即可使用所有功能块。

### 理由

1. **模块化**: 每个功能块独立编译和测试。
2. **选择性链接**: 用户可仅链接需要的功能块。
3. **可维护性**: 修改一个功能块不影响其他。
4. **并行开发**: 团队成员可同时开发不同功能块。

### 考虑的替代方案

| 方案 | 优点 | 缺点 | 不选择原因 |
|------|------|------|-----------|
| 单一大文件 | 简单 | 难以维护，编译慢 | 不符合模块化原则 |
| 每个功能一个目录 | 极度模块化 | 目录过多，复杂 | 过度设计 |

### 参考资料

- Embedded C Coding Standard, BARR Group
- Large-Scale C++ Software Design, John Lakos

---

## 决策总结表

| # | 决策内容 | 关键理由 | 影响模块 |
|---|----------|----------|----------|
| 1 | 微分项先行滤波 | 避免设定值突变尖峰 | PID |
| 2 | 条件积分法 | 简单高效的抗饱和 | PID |
| 3 | 前向欧拉离散化 | 计算简单，嵌入式友好 | PT1 |
| 4 | 最小有效值 1e-6 | 除零保护，保持可用性 | 所有 |
| 5 | NaN/Inf 输出零 | 安全输出 + 诊断能力 | 所有 |
| 6 | 首次输入初始化 | 平滑启动，无跳变 | PT1, 积分器 |
| 7 | 初始化时验证 | 早期错误检测 | 所有 |
| 8 | Unity 测试框架 | 嵌入式专用，轻量级 | 测试系统 |
| 9 | 独立文件组织 | 模块化，可维护性 | 项目结构 |

---

## 未解决问题（推迟到实现阶段）

以下低优先级问题将在实现过程中决定：

1. **Doxygen 配置**: 文档生成工具的具体配置（推迟到文档阶段）
2. **性能优化**: 具体的编译器优化标志（-O2 vs -O3）
3. **代码风格细节**: 缩进、命名等（遵循现有代码风格）

---

**研究状态**: ✅ 完成
**下一步**: 生成 data-model.md 和 contracts/
**最后更新**: 2026-01-18
