# 研究报告: 嵌入式 PLCOpen 控制应用程序

**Branch**: `003-embedded-plcopen-app` | **Date**: 2026-01-18

## 研究概述

本文档记录了为 003 迭代开发嵌入式 PLCOpen 演示程序所进行的技术研究。

---

## 研究任务 1: PLCOpen 功能块库 API 接口

### 问题描述

需要了解 002 迭代开发的功能块库的完整 API 接口，以便在演示程序中正确使用。

### 研究结论

**Decision**: 直接使用现有的 `#include <plcopen/plcopen.h>` 统一入口

**Rationale**:
- 库提供统一的头文件入口 `plcopen.h`，包含所有功能块
- 每个功能块遵循一致的 Init/Execute 模式
- 所有功能块使用相同的状态码枚举 `FB_Status_t`

**API 概览**:

| 功能块 | 初始化函数 | 执行函数 | 其他函数 |
|--------|-----------|----------|---------|
| FB_PID | `FB_PID_Init(&fb, &config)` | `FB_PID_Execute(&fb, sp, pv)` | `FB_PID_SetManual()`, `FB_PID_SetAuto()` |
| FB_PT1 | `FB_PT1_Init(&fb, &config)` | `FB_PT1_Execute(&fb, input)` | - |
| FB_RAMP | `FB_RAMP_Init(&fb, &config)` | `FB_RAMP_Execute(&fb, target)` | - |
| FB_LIMIT | `FB_LIMIT_Init(&fb, &config)` | `FB_LIMIT_Execute(&fb, input)` | - |
| FB_DEADBAND | `FB_DEADBAND_Init(&fb, &config)` | `FB_DEADBAND_Execute(&fb, input)` | - |
| FB_INTEGRATOR | `FB_INTEGRATOR_Init(&fb, &config)` | `FB_INTEGRATOR_Execute(&fb, input)` | `FB_INTEGRATOR_Reset(&fb)` |
| FB_DERIVATIVE | `FB_DERIVATIVE_Init(&fb, &config)` | `FB_DERIVATIVE_Execute(&fb, input)` | - |

**Alternatives Considered**:
- 自定义封装层：增加不必要的复杂性，拒绝
- 直接包含各个头文件：不符合库设计意图，拒绝

---

## 研究任务 2: 一阶惯性被控对象模型

### 问题描述

PID 闭环控制演示需要模拟被控对象。需要确定模型的数学形式和离散化方法。

### 研究结论

**Decision**: 使用一阶惯性模型，前向欧拉法离散化

**Rationale**:
- 一阶惯性模型 `G(s) = K/(τs+1)` 是最常见的过程模型
- 前向欧拉法简单高效，适合嵌入式系统
- 与功能块库中 PT1 使用相同的离散化方法，保持一致性

**数学模型**:

连续域传递函数：
```
G(s) = K / (τs + 1)
```

离散化（前向欧拉）：
```
y[k] = y[k-1] + (Ts/τ) * (K * u[k-1] - y[k-1])
```

其中：
- K = 1.0（增益）
- τ = 5.0s（时间常数，模拟热系统惰性）
- Ts = 0.1s（采样周期）

**Alternatives Considered**:
- 后向欧拉法：更稳定但计算复杂度略高，不必要
- 双线性变换（Tustin）：精度更高但对本演示过于复杂
- 二阶系统：超出演示需求范围

---

## 研究任务 3: 伪随机噪声发生器

### 问题描述

传感器噪声仿真需要伪随机数生成器。需要不依赖标准库 `rand()` 的实现。

### 研究结论

**Decision**: 使用线性同余生成器（LCG）

**Rationale**:
- LCG 算法简单，无外部依赖
- 可重复性好（相同种子产生相同序列）
- 适合嵌入式环境，内存占用极小

**算法**:
```c
// LCG 参数（来自 Numerical Recipes）
#define LCG_A 1664525UL
#define LCG_C 1013904223UL
#define LCG_M 4294967296UL  // 2^32

static uint32_t lcg_state = 12345;  // 默认种子

uint32_t lcg_rand(void) {
    lcg_state = (LCG_A * lcg_state + LCG_C) % LCG_M;
    return lcg_state;
}

// 生成 [-amplitude, +amplitude] 范围的浮点噪声
float noise_generate(float amplitude) {
    float normalized = (float)lcg_rand() / (float)LCG_M;  // [0, 1)
    return amplitude * (2.0f * normalized - 1.0f);  // [-amp, +amp)
}
```

**Alternatives Considered**:
- 标准库 rand()：在某些嵌入式环境不可用或行为不一致
- Mersenne Twister：内存占用大（约 2.5KB），过于复杂
- LFSR：需要处理周期性问题

---

## 研究任务 4: CSV 输出格式设计

### 问题描述

需要设计统一的 CSV 输出格式，便于数据分析和可视化。

### 研究结论

**Decision**: 统一 CSV 格式，首行列名，逗号分隔

**Rationale**:
- CSV 是通用数据交换格式
- 支持 Excel、Python pandas、MATLAB 等工具直接导入
- 首行列名便于自动解析

**格式规范**:

1. **PID 控制演示**:
```csv
time,setpoint,process_value,filtered_pv,control_output,status
0.000,80.00,25.00,25.00,100.00,0
0.100,80.00,26.50,26.45,100.00,1
...
```

2. **斜坡限幅演示**:
```csv
time,target,ramp_output,limited_output,limit_status
0.000,100.00,0.00,0.00,0
0.100,100.00,1.00,1.00,0
...
```

3. **信号处理链演示**:
```csv
time,raw_input,filtered,deadband_out,limited_out,status
0.000,50.50,50.00,50.00,50.00,0
...
```

4. **积分器/微分器演示**:
```csv
time,input,integral,derivative,int_status,deriv_status
0.000,10.00,0.00,0.00,0,0
...
```

**实现方式**:
```c
// CSV 输出辅助宏
#define CSV_HEADER_PID "time,setpoint,process_value,filtered_pv,control_output,status\n"
#define CSV_FORMAT_PID "%.3f,%.2f,%.2f,%.2f,%.2f,%d\n"

// 使用示例
printf(CSV_HEADER_PID);
printf(CSV_FORMAT_PID, time, sp, pv, filtered_pv, output, status);
```

**Alternatives Considered**:
- JSON 格式：解析复杂，不适合嵌入式
- 人类可读格式：不便于自动化分析

---

## 研究任务 5: CMake 构建配置最佳实践

### 问题描述

需要设计支持 x86 和 ARM 交叉编译的 CMake 配置。

### 研究结论

**Decision**: 使用现有项目 CMake 结构，扩展 examples/plcopen/CMakeLists.txt

**Rationale**:
- 保持与现有构建系统一致
- 复用已有的工具链配置
- 最小化改动范围

**CMake 结构**:

1. **顶层 CMakeLists.txt 更新**（已存在，无需修改）:
```cmake
# 已有: add_subdirectory(examples)
```

2. **examples/plcopen/CMakeLists.txt 更新**:
```cmake
# PLCopen 功能块示例程序 - CMake 配置

# 共享辅助模块库
add_subdirectory(common)

# 演示程序
add_subdirectory(pid_control_demo)
add_subdirectory(filter_demo)
add_subdirectory(full_system_demo)
add_subdirectory(ramp_limit_demo)      # 新增
add_subdirectory(integrator_demo)      # 新增
```

3. **common/CMakeLists.txt**（新建）:
```cmake
add_library(plcopen_demo_common STATIC
    plant_model.c
    noise_generator.c
)
target_include_directories(plcopen_demo_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(plcopen_demo_common PRIVATE plcopen m)
```

4. **各演示程序 CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.20)
add_executable(pid_control_demo main.c)
target_link_libraries(pid_control_demo PRIVATE plcopen_demo_common plcopen m)
```

**Alternatives Considered**:
- 单独的构建配置：与项目风格不一致
- 头文件 only 库：复杂模型需要实现文件

---

## 研究任务 6: 仿真配置宏设计

### 问题描述

需要设计编译时配置的仿真参数宏。

### 研究结论

**Decision**: 在 common/sim_config.h 中集中定义所有仿真参数

**Rationale**:
- 集中管理便于维护
- 编译时配置适合嵌入式环境
- 用户可覆盖默认值

**配置文件设计**:
```c
// sim_config.h - 仿真配置参数

#ifndef SIM_CONFIG_H
#define SIM_CONFIG_H

/* 仿真时间参数 */
#ifndef SIM_SAMPLE_TIME
#define SIM_SAMPLE_TIME     0.1f    /* 采样周期（秒） */
#endif

#ifndef SIM_DURATION
#define SIM_DURATION        20.0f   /* 仿真时长（秒） */
#endif

#ifndef SIM_OUTPUT_INTERVAL
#define SIM_OUTPUT_INTERVAL 1       /* 输出间隔（采样周期数） */
#endif

/* 被控对象模型参数 */
#ifndef PLANT_GAIN
#define PLANT_GAIN          1.0f    /* 系统增益 */
#endif

#ifndef PLANT_TIME_CONST
#define PLANT_TIME_CONST    5.0f    /* 时间常数（秒） */
#endif

/* 噪声参数 */
#ifndef NOISE_AMPLITUDE
#define NOISE_AMPLITUDE     1.0f    /* 噪声振幅 */
#endif

#ifndef NOISE_SEED
#define NOISE_SEED          12345   /* 随机种子 */
#endif

/* PID 控制器默认参数 */
#ifndef PID_KP
#define PID_KP              2.0f
#endif

#ifndef PID_KI
#define PID_KI              0.5f
#endif

#ifndef PID_KD
#define PID_KD              0.1f
#endif

#endif /* SIM_CONFIG_H */
```

**Alternatives Considered**:
- 运行时配置文件：需要文件系统，不适合裸机
- 命令行参数：嵌入式环境不实用

---

## 总结

所有研究任务已完成，无需进一步澄清。技术方案已确定：

| 研究任务 | 决策 |
|---------|------|
| 功能块 API | 使用统一入口 `plcopen.h` |
| 被控对象模型 | 一阶惯性模型，前向欧拉离散化 |
| 噪声发生器 | 线性同余生成器（LCG） |
| CSV 输出 | 统一格式，首行列名 |
| CMake 配置 | 扩展现有结构，添加共享库 |
| 仿真配置 | 头文件宏定义 |
