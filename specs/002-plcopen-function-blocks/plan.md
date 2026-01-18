# 实现计划：PLCopen 基础功能块库

**分支**: `002-plcopen-function-blocks` | **日期**: 2026-01-18 | **规格**: [spec.md](spec.md)
**输入**: 功能规格来自 `/specs/002-plcopen-function-blocks/spec.md`

## 概要

实现符合 PLCopen 标准的基础功能块库，用于 ARM Cortex-M4 嵌入式控制系统。核心包括 PID 控制器和一阶惯性滤波器（MVP），扩展支持斜坡发生器、限幅器、死区处理、积分器和微分器。所有功能块使用 C11 标准实现，支持多实例、完整的数值保护和错误处理机制。采用 Unity 测试框架确保代码质量，目标是实现高性能（<10μs 执行时间）和小内存占用（<2KB RAM）的工业级控制算法库。

## 技术上下文

**语言/版本**: C11 标准
**主要依赖**:
- ARM GCC Embedded 10.3-2021.10（工具链，已在 001 分支配置）
- Unity 测试框架（单元测试）
- CMake 3.20+（构建系统）
- 数学库：libm（浮点数学函数）

**存储**: 不适用（无需持久化存储，纯计算库）
**测试**: Unity 嵌入式 C 单元测试框架
**目标平台**: ARM Cortex-M4 with FPU（STM32F4 系列）
**项目类型**: 嵌入式库（library）
**性能目标**:
- 单次功能块执行时间 < 10μs（@168MHz）
- PID 控制器稳态误差 < 1%
- 测试覆盖率 > 90%

**约束**:
- Flash 占用 < 50KB
- 静态 RAM 占用 < 2KB
- 无动态内存分配
- 单精度浮点运算（float）
- 非线程安全（用户负责线程保护）

**规模/范围**:
- 7 个基础功能块
- 约 2000-3000 行代码
- 每个功能块 3-5 个 API 函数
- 完整的单元测试套件和示例程序

## Constitution Check

### 本土化与语言 ✅

- ✅ 所有文档使用简体中文
- ✅ 代码注释使用中文
- ✅ 函数和变量命名遵循 PLCopen 英文约定（行业标准）
- ✅ 错误消息和状态描述使用中文

**决策**: 代码接口保持英文（符合 PLCopen 国际标准），注释和文档中文化。

### 架构与设计 ✅

- ✅ 模块化设计：每个功能块独立实现
- ✅ 清晰的接口：结构体 + Init/Execute 函数模式
- ✅ 避免过度设计：不使用函数指针、虚表等复杂机制
- ✅ 人类可读：简洁的算法实现，详细注释

**设计模式**: 使用状态封装模式（每个功能块用结构体保存状态）

### 架构约束 ✅

- ✅ 利用现有 C11 环境（001 分支已完成）
- ✅ 遵循现有目录结构：
  - `src/plcopen/` - 功能块源代码
  - `include/plcopen/` - 公共头文件
  - `tests/plcopen/` - 单元测试
  - `examples/plcopen/` - 示例程序

**决策**: 不修改根目录结构，在现有框架内扩展。

### 环境与配置 ✅

- ✅ 复用现有工具链（ARM GCC 已自动安装）
- ✅ Unity 框架将自动下载到 `.toolchain/unity/`
- ✅ 所有配置限定在项目工作空间
- ✅ CMake 配置文件管理构建

**工具安装策略**:
- Unity: 通过 git submodule 或脚本自动下载
- 其他工具: 复用 001 分支的环境

### 工程实践与开发体验 ✅

- ✅ 跨平台构建支持（Windows/Linux/macOS）
- ✅ 统一的构建命令：`./scripts/build/build.sh --config Release`
- ✅ 统一的测试命令：`./scripts/test/run-tests.sh plcopen`

**工具选择**: 使用 001 分支已建立的工具链和脚本框架。

### 依赖管理 ✅

- ✅ Unity 测试框架：MIT 许可证，从 GitHub 镜像获取
- ✅ 数学库 libm：GCC 内置，无需额外依赖
- ✅ 无其他第三方依赖

**镜像策略**: Unity 可选从 Gitee 镜像获取（如果 GitHub 访问慢）

### 许可合规 ✅

- ✅ Unity: MIT 许可证（商业友好）
- ✅ 项目代码: MIT 许可证
- ✅ ARM GCC: GPL（工具链，不影响生成代码）

**合规性**: 所有依赖对商业应用友好。

### 代码质量 ✅

- ✅ UTF-8 编码，LF 换行符
- ✅ 完整的中文注释
- ✅ 圈复杂度限制：单函数 < 10
- ✅ 单元测试覆盖率 > 90%
- ✅ 代码复用：通用数值保护函数

**质量工具**: 使用 cppcheck 进行静态分析（可选）

### 修改范围 ✅

- ✅ 变更限定在 `src/plcopen/`、`include/plcopen/`、`tests/plcopen/` 目录
- ✅ 不修改现有 C11 环境配置
- ✅ 仅添加新的构建目标到 CMakeLists.txt

**最小化原则**: 新增代码，不修改现有模块。

## Constitution 评估结果

**状态**: ✅ **通过** - 所有 Constitution 原则均已满足或有明确的合规策略

**需要监控的项目**:
1. 依赖管理：Unity 框架下载速度（如慢则切换到 Gitee 镜像）
2. 代码质量：定期检查圈复杂度和测试覆盖率

## Phase 0: 研究与技术决策

### 研究任务

以下研究任务将生成 `research.md` 文档：

1. **PID 控制算法最佳实践**
   - 微分项先行滤波（Derivative on Measurement）实现细节
   - 条件积分法（Conditional Integration）的数学推导
   - 手自动切换的无扰动策略

2. **PLCopen 标准规范**
   - IEC 61131-3 功能块接口约定
   - 命名规则和数据类型映射
   - 状态码和错误处理标准

3. **一阶惯性滤波器设计**
   - 离散化方法选择（前向欧拉 vs 双线性变换）
   - 时间常数与采样周期的关系
   - 数值稳定性分析

4. **数值保护策略**
   - 浮点溢出检测方法（isinf, isnan）
   - 除零保护的最小有效值选择（1e-6）
   - 误差累积控制

5. **Unity 测试框架集成**
   - Unity 项目结构和配置
   - CMake 集成方法
   - 嵌入式目标上的测试运行策略

### 待解决的技术问题

研究将解决以下问题：

| 问题 | 决策依据 |
|------|----------|
| PT1 滤波器离散化方法 | 精度 vs 计算复杂度权衡 |
| PID 微分项滤波时间常数 | 噪声抑制 vs 响应速度 |
| Unity 测试在目标硬件上的执行方式 | 串口输出 vs 半主机模式 |
| 功能块文件组织 | 单文件 vs 多文件模块化 |

### 研究输出

生成 `research.md` 文档，包含：
- 决策记录（Decision）
- 理由（Rationale）
- 考虑的替代方案（Alternatives Considered）
- 参考资料（References）

## Phase 1: 设计与合约

### 数据模型设计

生成 `data-model.md` 文档，定义：

#### FB_PID（PID 控制器）

```c
// 配置参数（用户设置）
typedef struct {
    float kp;              // 比例增益
    float ki;              // 积分增益
    float kd;              // 微分增益
    float sample_time;     // 采样周期（秒）
    float out_min;         // 输出下限
    float out_max;         // 输出上限
    float int_min;         // 积分限幅下限
    float int_max;         // 积分限幅上限
} FB_PID_Config_t;

// 运行时状态（内部）
typedef struct {
    float integral;        // 积分累加值
    float prev_measurement;// 上次测量值（用于微分）
    float prev_output;     // 上次输出值
    bool manual_mode;      // 手动/自动模式
    bool first_run;        // 首次运行标志
    FB_Status_t status;    // 状态码
} FB_PID_State_t;

// 完整功能块实例
typedef struct {
    FB_PID_Config_t config;
    FB_PID_State_t state;
} FB_PID_t;
```

#### FB_PT1（一阶惯性滤波器）

```c
typedef struct {
    float time_constant;   // 时间常数 τ（秒）
    float sample_time;     // 采样周期（秒）
} FB_PT1_Config_t;

typedef struct {
    float output;          // 当前输出值
    bool first_run;        // 首次运行标志
    FB_Status_t status;    // 状态码
} FB_PT1_State_t;

typedef struct {
    FB_PT1_Config_t config;
    FB_PT1_State_t state;
} FB_PT1_t;
```

#### 通用状态码

```c
typedef enum {
    FB_STATUS_OK = 0,          // 正常运行
    FB_STATUS_LIMIT_HI = 1,    // 输出达到上限
    FB_STATUS_LIMIT_LO = 2,    // 输出达到下限
    FB_STATUS_ERROR_NAN = -1,  // 输入为 NaN
    FB_STATUS_ERROR_INF = -2,  // 输入为 Inf
    FB_STATUS_ERROR_CONFIG = -3 // 配置错误
} FB_Status_t;
```

### API 合约设计

生成 `contracts/` 目录，包含：

#### `contracts/fb_pid.md`

**FB_PID_Init()**
- 输入：FB_PID_t* 实例指针，FB_PID_Config_t* 配置
- 输出：int 返回码（0=成功，-1=配置无效）
- 前置条件：配置参数已设置且有效
- 后置条件：实例已初始化，状态重置
- 验证规则：
  - sample_time > 0 && < 1000
  - out_max > out_min
  - int_max > int_min

**FB_PID_Execute()**
- 输入：FB_PID_t* 实例，float setpoint（设定值），float measurement（测量值）
- 输出：float 控制输出
- 前置条件：实例已初始化
- 后置条件：内部状态已更新
- 行为：
  - 计算误差 = setpoint - measurement
  - P项 = kp * error
  - I项累加（条件积分法）
  - D项 = -kd * (measurement - prev_measurement) / sample_time
  - 输出限幅
  - 更新状态

#### `contracts/fb_pt1.md`

**FB_PT1_Init()**
- 输入：FB_PT1_t* 实例，FB_PT1_Config_t* 配置
- 输出：int 返回码
- 验证规则：
  - time_constant > 1e-6
  - sample_time > 0

**FB_PT1_Execute()**
- 输入：FB_PT1_t* 实例，float input
- 输出：float 滤波后输出
- 行为：
  - alpha = sample_time / (time_constant + sample_time)
  - output = output + alpha * (input - output)
  - 首次运行：output = input

### 快速入门指南

生成 `quickstart.md` 文档，包含：

#### 安装

```bash
# 确保 C11 环境已配置（001 分支）
source ./scripts/setup/configure-environment.sh

# 安装 Unity 测试框架
./scripts/setup/install-unity.sh

# 构建功能块库
./scripts/build/build.sh --config Release
```

#### 使用示例：PID 控制

```c
#include "plcopen/fb_pid.h"

// 1. 定义实例
FB_PID_t pid_controller;

// 2. 配置参数
FB_PID_Config_t pid_config = {
    .kp = 1.0f,
    .ki = 0.1f,
    .kd = 0.05f,
    .sample_time = 0.01f,  // 10ms
    .out_min = 0.0f,
    .out_max = 100.0f,
    .int_min = -50.0f,
    .int_max = 50.0f
};

// 3. 初始化
if (FB_PID_Init(&pid_controller, &pid_config) != 0) {
    // 处理配置错误
}

// 4. 周期执行（10ms 定时任务）
while (1) {
    float setpoint = 50.0f;       // 设定值
    float measurement = read_sensor(); // 读取传感器

    float output = FB_PID_Execute(&pid_controller, setpoint, measurement);

    set_actuator(output);         // 输出到执行器

    delay_ms(10);                 // 等待下一个周期
}
```

#### 使用示例：PT1 滤波

```c
#include "plcopen/fb_pt1.h"

FB_PT1_t filter;
FB_PT1_Config_t filter_config = {
    .time_constant = 1.0f,   // 1秒时间常数
    .sample_time = 0.01f     // 10ms采样
};

FB_PT1_Init(&filter, &filter_config);

// 周期执行
float raw_signal = read_noisy_sensor();
float filtered_signal = FB_PT1_Execute(&filter, raw_signal);
```

## Phase 1 输出总结

完成后将生成以下文档：

- ✅ `data-model.md` - 完整的数据结构定义
- ✅ `contracts/fb_pid.md` - PID 功能块 API 合约
- ✅ `contracts/fb_pt1.md` - PT1 功能块 API 合约
- ✅ `contracts/fb_ramp.md` - RAMP 功能块 API 合约
- ✅ `contracts/fb_limit.md` - LIMIT 功能块 API 合约
- ✅ `contracts/fb_deadband.md` - DEADBAND 功能块 API 合约
- ✅ `contracts/fb_integrator.md` - INTEGRATOR 功能块 API 合约
- ✅ `contracts/fb_derivative.md` - DERIVATIVE 功能块 API 合约
- ✅ `contracts/common.md` - 通用数值保护函数合约
- ✅ `quickstart.md` - 快速入门指南

## 项目结构

### 文档结构（本功能）

```
specs/002-plcopen-function-blocks/
├── spec.md              # 功能规格（已完成）
├── plan.md              # 本文件（当前）
├── research.md          # Phase 0 输出
├── data-model.md        # Phase 1 输出
├── quickstart.md        # Phase 1 输出
├── contracts/           # Phase 1 输出
│   ├── fb_pid.md
│   ├── fb_pt1.md
│   ├── fb_ramp.md
│   ├── fb_limit.md
│   ├── fb_deadband.md
│   ├── fb_integrator.md
│   ├── fb_derivative.md
│   └── common.md
├── checklists/
│   └── requirements.md  # 质量检查清单（已完成）
└── tasks.md             # Phase 2 输出（/speckit.tasks 命令生成）
```

### 源代码结构（仓库根目录）

```
embedded-py-runtime/
├── src/
│   └── plcopen/
│       ├── fb_pid.c
│       ├── fb_pt1.c
│       ├── fb_ramp.c
│       ├── fb_limit.c
│       ├── fb_deadband.c
│       ├── fb_integrator.c
│       ├── fb_derivative.c
│       └── common.c          # 通用数值保护函数
│
├── include/
│   └── plcopen/
│       ├── fb_pid.h
│       ├── fb_pt1.h
│       ├── fb_ramp.h
│       ├── fb_limit.h
│       ├── fb_deadband.h
│       ├── fb_integrator.h
│       ├── fb_derivative.h
│       ├── common.h
│       └── plcopen.h         # 主头文件（包含所有功能块）
│
├── tests/
│   └── plcopen/
│       ├── test_fb_pid.c
│       ├── test_fb_pt1.c
│       ├── test_fb_ramp.c
│       ├── test_fb_limit.c
│       ├── test_fb_deadband.c
│       ├── test_fb_integrator.c
│       ├── test_fb_derivative.c
│       ├── test_common.c
│       └── test_runner.c     # Unity 测试运行器
│
├── examples/
│   └── plcopen/
│       ├── pid_control_demo/
│       │   ├── main.c
│       │   └── CMakeLists.txt
│       ├── filter_demo/
│       │   ├── main.c
│       │   └── CMakeLists.txt
│       └── full_system_demo/
│           ├── main.c
│           └── CMakeLists.txt
│
├── .toolchain/
│   └── unity/                # Unity 测试框架（自动下载）
│
└── CMakeLists.txt            # 根 CMake 配置（添加 plcopen 目标）
```

## 技术架构

### 模块依赖关系

```
┌─────────────────────────────────────┐
│   用户应用代码（main.c）            │
└──────────────┬──────────────────────┘
               │ 调用
               ▼
┌─────────────────────────────────────┐
│   PLCopen 功能块层                  │
│  ┌──────┐ ┌──────┐ ┌──────┐        │
│  │ PID  │ │ PT1  │ │ RAMP │ ...    │
│  └───┬──┘ └───┬──┘ └───┬──┘        │
│      └────────┼────────┘            │
│               ▼                      │
│      ┌────────────────┐             │
│      │  Common Utils  │             │
│      │ (数值保护函数)  │             │
│      └────────────────┘             │
└──────────────┬──────────────────────┘
               │ 依赖
               ▼
┌─────────────────────────────────────┐
│   ARM GCC 标准库                    │
│   libm (math.h)                     │
│   float.h, stdbool.h, stdint.h     │
└─────────────────────────────────────┘
```

### 构建流程

```
CMake 配置
    ↓
生成 Makefile
    ↓
编译源文件 (.c → .o)
    ↓
链接静态库 (libplcopen.a)
    ↓
编译测试 (Unity + 测试代码)
    ↓
链接测试可执行文件
    ↓
运行测试（可选：在目标硬件）
    ↓
编译示例程序
    ↓
生成文档 (Doxygen, 可选)
```

## 里程碑与交付物

### Milestone 1: MVP - 核心功能块（2周）

**目标**: 实现 PID 和 PT1 功能块，提供基本控制能力

**交付物**:
- ✅ `research.md` - 技术决策文档
- ✅ `data-model.md` - PID 和 PT1 数据模型
- ✅ `contracts/fb_pid.md` 和 `contracts/fb_pt1.md`
- ✅ `src/plcopen/fb_pid.c` 和 `fb_pt1.c`
- ✅ `include/plcopen/fb_pid.h` 和 `fb_pt1.h`
- ✅ `tests/plcopen/test_fb_pid.c` 和 `test_fb_pt1.c`
- ✅ `examples/plcopen/pid_control_demo/`
- ✅ Unity 测试框架集成
- ✅ CMake 构建配置
- ✅ `quickstart.md` 初版

**验收标准**:
- PID 控制器通过仿真测试（稳态误差 < 1%）
- PT1 滤波器响应曲线符合一阶惯性特性
- 单元测试覆盖率 > 90%
- 所有测试通过
- 示例程序可编译并运行

### Milestone 2: 扩展功能块（1.5周）

**目标**: 完成 RAMP、LIMIT、DEADBAND 功能块

**交付物**:
- ✅ `contracts/fb_ramp.md`、`fb_limit.md`、`fb_deadband.md`
- ✅ 对应源文件和测试
- ✅ 更新 `data-model.md`
- ✅ 更新 `quickstart.md`

**验收标准**:
- 所有功能块通过单元测试
- 测试覆盖率保持 > 90%
- 性能满足 < 10μs 执行时间要求

### Milestone 3: 高级功能块（1.5周）

**目标**: 完成 INTEGRATOR 和 DERIVATIVE 功能块

**交付物**:
- ✅ `contracts/fb_integrator.md`、`fb_derivative.md`
- ✅ 对应源文件和测试
- ✅ 完整的 `examples/plcopen/full_system_demo/`
- ✅ 性能基准测试报告

**验收标准**:
- 所有 7 个功能块实现完成
- 综合示例程序展示多功能块协同工作
- 内存占用 < 2KB RAM
- 代码大小 < 50KB Flash

### Milestone 4: 文档与打磨（0.5周）

**目标**: 完善文档、优化性能、代码审查

**交付物**:
- ✅ 完整的 API 文档（Doxygen 生成）
- ✅ 用户手册（中文）
- ✅ 性能优化报告
- ✅ 代码审查报告
- ✅ 发布版本 v1.0.0

**验收标准**:
- 所有文档完整且经过审阅
- 代码通过静态分析（cppcheck）
- 无已知 Bug
- 准备合并到主分支

## 风险与缓解策略

| 风险 | 影响 | 缓解策略 |
|------|------|----------|
| Unity 框架集成复杂 | 中 | 预先验证 Unity 在 Cortex-M4 上的运行，准备简化的测试方案 |
| PID 算法调试困难 | 中 | 使用 MATLAB/Python 仿真验证算法正确性后再实现 |
| 浮点精度问题 | 低 | 使用已知测试用例验证数值稳定性 |
| 性能不达标 | 低 | 早期进行性能基准测试，必要时优化关键路径 |
| 依赖下载慢 | 低 | 提供 Gitee 镜像作为备份 |

## 下一步行动

1. **立即**: 运行 `/speckit.plan` 完成 Phase 0 和 Phase 1（生成 research.md, data-model.md, contracts/, quickstart.md）
2. **然后**: 运行 `/speckit.tasks` 生成详细任务分解（tasks.md）
3. **开始实现**: 按照 tasks.md 中的任务顺序开始编码

---

**计划状态**: ✅ Phase 0-1 设计完成
**下一步**: 运行 agent 脚本生成 research.md, data-model.md, contracts/, quickstart.md
**最后更新**: 2026-01-18
