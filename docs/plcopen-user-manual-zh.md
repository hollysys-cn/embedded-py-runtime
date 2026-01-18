# PLCopen 基础功能块库使用手册

版本: 1.0.0
日期: 2026-01-18

## 1. 简介

本库提供了符合 PLCopen 标准的 7 个基础控制功能块，专为 ARM Cortex-M4 嵌入式系统设计。所有功能块均采用标准 C11 编写，不依赖特定硬件，易于移植。

### 功能块列表

1. **PID**: 比例-积分-微分控制器
2. **PT1**: 一阶惯性滤波器
3. **RAMP**: 斜坡发生器
4. **LIMIT**: 限幅器
5. **DEADBAND**: 死区处理
6. **INTEGRATOR**: 积分器
7. **DERIVATIVE**: 微分器

### 核心特性

- **标准接口**: 统一使用 Parse-Init-Execute 模式
- **数值保护**: 内置溢出、除零、NaN/Inf 检测
- **高性能**: 针对嵌入式优化，单次执行 < 10μs
- **无动态内存**: 所有状态由用户分配，确定性内存占用

---

## 2. 基础数据类型

所有功能块返回统一的状态码：

```c
typedef enum {
    FB_STATUS_OK = 0,           // 正常
    FB_STATUS_ERROR_PARAM = -1, // 参数错误
    FB_STATUS_ERROR_NAN = -2,   // 输入/输出为 NaN
    FB_STATUS_ERROR_INF = -3,   // 输入/输出为 Inf
    FB_STATUS_LIMIT_HI = 1,     // 输出达到上限
    FB_STATUS_LIMIT_LO = 2,     // 输出达到下限
} FB_Status_t;
```

---

## 3. 功能块详解

### 3.1 PID 控制器 (FB_PID)

通用 PID 控制器，支持抗饱和、微分先行和无扰切换。

**配置参数**:
- `kp`, `ki`, `kd`: 比例/积分/微分增益
- `sample_time`: 采样周期 (s)
- `out_min`/`out_max`: 输出限幅
- `int_min`/`int_max`: 积分限幅

**使用示例**:

```c
#include "plcopen/fb_pid.h"

FB_PID_t pid;
FB_PID_Config_t config = {
    .kp = 1.0f, .ki = 0.1f, .kd = 0.01f,
    .sample_time = 0.01f, // 10ms
    .out_min = 0.0f, .out_max = 100.0f,
    .int_min = -50.0f, .int_max = 50.0f
};

// 初始化
FB_PID_Init(&pid, &config);

// 在控制循环中周期调用
float output = FB_PID_Execute(&pid, setpoint, process_value);
```

### 3.2 一阶滤波器 (FB_PT1)

用于信号平滑和噪声抑制。

**配置参数**:
- `time_constant`: 时间常数 (s)
- `sample_time`: 采样周期 (s)

**使用示例**:

```c
#include "plcopen/fb_pt1.h"

FB_PT1_t pt1;
FB_PT1_Config_t config = { .time_constant = 0.1f, .sample_time = 0.01f };

FB_PT1_Init(&pt1, &config);

// 周期调用
float filtered = FB_PT1_Execute(&pt1, raw_input);
```

### 3.3 斜坡发生器 (FB_RAMP)

限制信号变化率，避免系统冲击。

**配置参数**:
- `rise_rate`: 上升速率 (units/s)
- `fall_rate`: 下降速率 (units/s)
- `sample_time`: 采样周期 (s)

### 3.4 限幅器 (FB_LIMIT)

将信号限制在指定范围内。

**配置参数**:
- `min`: 下限
- `max`: 上限

### 3.5 死区处理 (FB_DEADBAND)

消除零点附近的微小噪声。

**配置参数**:
- `center`: 中心值
- `width`: 死区宽度 (半宽)

### 3.6 积分器 (FB_INTEGRATOR)

对输入信号进行时间积分。

**配置参数**:
- `sample_time`: 采样周期 (s)
- `out_min`/`out_max`: 积分输出限幅 (可选)

### 3.7 微分器 (FB_DERIVATIVE)

计算信号变化率，支持滤波。

**配置参数**:
- `sample_time`: 采样周期 (s)
- `filter_time_constant`: 微分滤波时间常数 (0表示不滤波)

---

## 4. 最佳实践

1. **初始化检查**: 始终检查 `FB_xxx_Init` 的返回值，确保参数有效。
2. **采样一致性**: 确保配置的 `sample_time` 与实际调用周期一致。
3. **状态隔离**: 每个控制回路应使用独立的 FB 实例变量。
4. **数值安全**: 库函数已处理大部分数值异常，但仍建议监控 `status` 字段。
