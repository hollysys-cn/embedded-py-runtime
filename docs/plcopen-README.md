# PLCopen 基础功能块库

> 符合 PLCopen 标准的工业控制功能块库，用于 ARM Cortex-M4 嵌入式系统

## 功能概述

本库实现了 7 个基础控制功能块，适用于工业自动化和过程控制应用：

| 功能块 | 描述 | 优先级 | 典型应用 |
|--------|------|--------|----------|
| **FB_PID** | PID 控制器 | P1 (MVP) | 温度、压力、流量控制 |
| **FB_PT1** | 一阶惯性滤波器 | P1 (MVP) | 信号平滑、噪声抑制 |
| **FB_RAMP** | 斜坡发生器 | P2 | 设定值平滑过渡 |
| **FB_LIMIT** | 限幅器 | P2 | 输出信号限制 |
| **FB_DEADBAND** | 死区处理 | P3 | 消除微小波动 |
| **FB_INTEGRATOR** | 积分器 | P3 | 流量累计、能量累计 |
| **FB_DERIVATIVE** | 微分器 | P3 | 速度、加速度计算 |

## 主要特性

- ✅ **符合 PLCopen 标准** - 遵循 IEC 61131-3 规范
- ✅ **工业级算法** - 微分项先行、条件积分法、手自动切换
- ✅ **完整数值保护** - NaN/Inf 检测、除零保护、输出限幅
- ✅ **C11 标准实现** - 可移植、无动态内存分配
- ✅ **详细中文注释** - 代码和文档完全中文化
- ✅ **单元测试** - 使用 Unity 测试框架
- ✅ **高性能** - 目标执行时间 < 10μs
- ✅ **小内存占用** - Flash < 50KB, RAM < 2KB

## 快速开始

### 1. 安装工具链

```bash
# 安装 ARM GCC 工具链
./scripts/setup/install-toolchain.sh

# 验证安装
./scripts/setup/verify-toolchain.sh
```

### 2. 编译库

```bash
# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake ..

# 编译
make

# 运行测试
make test
```

### 3. 使用示例：PID 控制器

```c
#include <plcopen/plcopen.h>

int main(void) {
    // 1. 定义实例
    FB_PID_t pid_controller;

    // 2. 配置参数
    FB_PID_Config_t config = {
        .kp = 1.0f,           // 比例增益
        .ki = 0.1f,           // 积分增益
        .kd = 0.05f,          // 微分增益
        .sample_time = 0.01f, // 10ms 采样周期
        .out_min = 0.0f,      // 输出下限
        .out_max = 100.0f,    // 输出上限
        .int_min = -50.0f,    // 积分限幅下限
        .int_max = 50.0f      // 积分限幅上限
    };

    // 3. 初始化
    if (FB_PID_Init(&pid_controller, &config) != 0) {
        // 处理配置错误
        return -1;
    }

    // 4. 周期执行（10ms 定时任务）
    while (1) {
        float setpoint = 50.0f;              // 设定值
        float measurement = read_sensor();    // 读取传感器

        // 执行 PID 控制
        float output = FB_PID_Execute(&pid_controller, setpoint, measurement);

        // 检查状态
        if (pid_controller.state.status == FB_STATUS_ERROR_NAN) {
            // 处理异常输入
        }

        // 输出到执行器
        set_actuator(output);

        delay_ms(10);  // 等待下一个周期
    }

    return 0;
}
```

### 4. 使用示例：PT1 滤波器

```c
#include <plcopen/plcopen.h>

int main(void) {
    // 1. 定义实例
    FB_PT1_t filter;

    // 2. 配置参数
    FB_PT1_Config_t config = {
        .time_constant = 1.0f,   // 1秒时间常数
        .sample_time = 0.01f     // 10ms 采样周期
    };

    // 3. 初始化
    FB_PT1_Init(&filter, &config);

    // 4. 周期执行
    while (1) {
        float raw_signal = read_noisy_sensor();
        float filtered_signal = FB_PT1_Execute(&filter, raw_signal);

        // 使用滤波后的信号
        process_signal(filtered_signal);

        delay_ms(10);
    }

    return 0;
}
```

## 项目结构

```
embedded-py-runtime/
├── include/plcopen/          # 公共头文件
│   ├── plcopen.h            # 主头文件
│   ├── common.h             # 通用定义
│   ├── fb_pid.h             # PID 控制器
│   ├── fb_pt1.h             # PT1 滤波器
│   ├── fb_ramp.h            # 斜坡发生器
│   ├── fb_limit.h           # 限幅器
│   ├── fb_deadband.h        # 死区处理
│   ├── fb_integrator.h      # 积分器
│   └── fb_derivative.h      # 微分器
│
├── src/plcopen/              # 功能块实现
│   ├── common.c
│   ├── fb_pid.c
│   ├── fb_pt1.c
│   ├── fb_ramp.c
│   ├── fb_limit.c
│   ├── fb_deadband.c
│   ├── fb_integrator.c
│   └── fb_derivative.c
│
├── tests/plcopen/            # 单元测试
│   ├── test_common.c
│   ├── test_fb_pid.c
│   └── ...
│
├── examples/plcopen/         # 示例程序
│   ├── pid_control_demo/
│   ├── filter_demo/
│   └── full_system_demo/
│
└── docs/                     # 文档
    ├── IMPLEMENTATION_REPORT_002.md
    └── ...
```

## API 文档

### 通用定义

```c
// 状态码枚举
typedef enum {
    FB_STATUS_OK = 0,          // 正常运行
    FB_STATUS_LIMIT_HI = 1,    // 输出达到上限
    FB_STATUS_LIMIT_LO = 2,    // 输出达到下限
    FB_STATUS_ERROR_NAN = -1,  // 输入为 NaN
    FB_STATUS_ERROR_INF = -2,  // 输入为 Inf
    FB_STATUS_ERROR_CONFIG = -3 // 配置错误
} FB_Status_t;

// 通用工具函数
bool check_overflow(float value);
float safe_divide(float numerator, float denominator);
bool check_nan_inf(float value);
float clamp_output(float value, float min_val, float max_val);
```

### PID 控制器 API

```c
// 初始化
int FB_PID_Init(FB_PID_t* fb, const FB_PID_Config_t* config);

// 执行控制算法
float FB_PID_Execute(FB_PID_t* fb, float setpoint, float measurement);

// 手自动切换
void FB_PID_SetManual(FB_PID_t* fb, float manual_output);
void FB_PID_SetAuto(FB_PID_t* fb);

// 状态查询
FB_Status_t FB_PID_GetStatus(const FB_PID_t* fb);
bool FB_PID_IsManual(const FB_PID_t* fb);
```

### PT1 滤波器 API

```c
// 初始化
int FB_PT1_Init(FB_PT1_t* fb, const FB_PT1_Config_t* config);

// 执行滤波
float FB_PT1_Execute(FB_PT1_t* fb, float input);
```

## 技术细节

### PID 控制算法

- **比例项**: `P = Kp * error`
- **积分项**: 条件积分法防止饱和
- **微分项**: 微分项先行滤波（对测量值求微分）
- **输出**: `output = clamp(P + I + D, out_min, out_max)`

### PT1 滤波器

- **传递函数**: `H(s) = 1 / (τs + 1)`
- **离散化**: 前向欧拉法
- **更新公式**: `y[k] = y[k-1] + α * (u[k] - y[k-1])`
- **系数**: `α = Ts / (τ + Ts)`

## 性能指标

| 指标 | 目标 | 说明 |
|------|------|------|
| 执行时间 | < 10μs | 单次功能块调用 @168MHz |
| Flash 占用 | < 50KB | 全部功能块 |
| RAM 占用 | < 2KB | 静态内存 |
| 测试覆盖率 | > 90% | 单元测试 |
| 稳态误差 | < 1% | PID 控制器 |

## 测试

```bash
# 运行所有测试
cd build
make test

# 运行单个测试
./test_common
./test_fb_pid
./test_fb_pt1
```

## 示例程序

```bash
# 运行 PID 控制演示
./pid_control_demo

# 运行滤波演示
./filter_demo

# 运行综合系统演示
./full_system_demo
```

## 贡献指南

1. 遵循 C11 标准
2. 所有代码和注释使用简体中文
3. 使用 Unity 框架编写单元测试
4. 保持函数圈复杂度 < 10
5. 提交前运行全部测试

## 许可证

MIT 许可证

## 联系方式

- 项目主页: [embedded-py-runtime](https://github.com/hollysys-cn/embedded-py-runtime)
- 问题反馈: [Issues](https://github.com/hollysys-cn/embedded-py-runtime/issues)

---

**版本**: 1.0.0
**最后更新**: 2026-01-18
