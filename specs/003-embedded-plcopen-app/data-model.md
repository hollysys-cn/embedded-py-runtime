# 数据模型: 嵌入式 PLCOpen 控制应用程序

**Branch**: `003-embedded-plcopen-app` | **Date**: 2026-01-18

## 概述

本文档定义了演示程序中使用的数据结构和辅助模块。这些模块用于仿真环境搭建，与 PLCOpen 功能块库配合使用。

---

## 实体定义

### 1. PlantModel（被控对象模型）

**用途**: 模拟一阶惯性物理系统（如温度系统），响应控制器输出并产生过程反馈。

**数学模型**: `G(s) = K / (τs + 1)`

**结构定义**:

```c
/**
 * @brief 一阶惯性被控对象模型配置
 */
typedef struct {
    float gain;              /**< 系统增益 K（通常为 1.0） */
    float time_constant;     /**< 时间常数 τ（秒，> 0） */
    float sample_time;       /**< 采样周期 Ts（秒，> 0） */
} PlantModel_Config_t;

/**
 * @brief 一阶惯性被控对象模型状态
 */
typedef struct {
    float output;            /**< 当前输出值（过程变量） */
    bool first_run;          /**< 首次运行标志 */
} PlantModel_State_t;

/**
 * @brief 一阶惯性被控对象模型实例
 */
typedef struct {
    PlantModel_Config_t config;  /**< 配置参数 */
    PlantModel_State_t state;    /**< 运行时状态 */
} PlantModel_t;
```

**API**:

| 函数 | 描述 | 参数 | 返回值 |
|------|------|------|--------|
| `PlantModel_Init` | 初始化模型 | `fb`, `config`, `initial_output` | `int` (0=成功) |
| `PlantModel_Execute` | 执行模型更新 | `fb`, `control_input` | `float` (过程输出) |
| `PlantModel_Reset` | 重置模型状态 | `fb`, `initial_output` | `void` |

**验证规则**:
- `time_constant > 0` 且 `< MAX_SAMPLE_TIME`
- `sample_time > 0` 且 `< MAX_SAMPLE_TIME`
- `gain` 可为任意有限浮点数

---

### 2. NoiseGenerator（噪声发生器）

**用途**: 生成可配置幅度的伪随机噪声，模拟传感器测量误差。

**算法**: 线性同余生成器（LCG）

**结构定义**:

```c
/**
 * @brief 噪声发生器配置
 */
typedef struct {
    float amplitude;         /**< 噪声振幅（生成 [-amp, +amp] 范围） */
    uint32_t seed;           /**< 随机数种子 */
} NoiseGenerator_Config_t;

/**
 * @brief 噪声发生器状态
 */
typedef struct {
    uint32_t state;          /**< LCG 内部状态 */
} NoiseGenerator_State_t;

/**
 * @brief 噪声发生器实例
 */
typedef struct {
    NoiseGenerator_Config_t config;  /**< 配置参数 */
    NoiseGenerator_State_t state;    /**< 运行时状态 */
} NoiseGenerator_t;
```

**API**:

| 函数 | 描述 | 参数 | 返回值 |
|------|------|------|--------|
| `NoiseGenerator_Init` | 初始化噪声发生器 | `ng`, `config` | `void` |
| `NoiseGenerator_Generate` | 生成一个噪声样本 | `ng` | `float` (噪声值) |
| `NoiseGenerator_Reset` | 重置随机状态 | `ng` | `void` |

**验证规则**:
- `amplitude >= 0`
- `seed` 可为任意 uint32_t 值（0 使用默认种子）

---

### 3. SimulationConfig（仿真配置）

**用途**: 封装仿真运行参数，通过宏定义在编译时配置。

**结构定义**（宏定义形式）:

```c
/**
 * @file sim_config.h
 * @brief 仿真参数配置（编译时定义）
 */

/* 时间参数 */
#define SIM_SAMPLE_TIME     0.1f    /**< 采样周期（秒） */
#define SIM_DURATION        20.0f   /**< 仿真时长（秒） */
#define SIM_OUTPUT_INTERVAL 1       /**< 输出间隔（采样周期数） */

/* 被控对象参数 */
#define PLANT_GAIN          1.0f    /**< 系统增益 */
#define PLANT_TIME_CONST    5.0f    /**< 时间常数（秒） */
#define PLANT_INITIAL_TEMP  25.0f   /**< 初始温度（°C） */

/* 设定值 */
#define SETPOINT_INITIAL    80.0f   /**< 初始设定值 */
#define SETPOINT_CHANGE_TIME 10.0f  /**< 设定值改变时间（秒） */
#define SETPOINT_FINAL      60.0f   /**< 最终设定值 */

/* 噪声参数 */
#define NOISE_AMPLITUDE     1.0f    /**< 噪声振幅 */
#define NOISE_SEED          12345   /**< 随机种子 */

/* PID 参数 */
#define PID_KP              2.0f    /**< 比例增益 */
#define PID_KI              0.5f    /**< 积分增益 */
#define PID_KD              0.1f    /**< 微分增益 */
#define PID_OUT_MIN         0.0f    /**< 输出下限 */
#define PID_OUT_MAX         100.0f  /**< 输出上限 */
#define PID_INT_MIN         -50.0f  /**< 积分下限 */
#define PID_INT_MAX         50.0f   /**< 积分上限 */

/* PT1 滤波参数 */
#define PT1_TIME_CONST      0.5f    /**< 滤波时间常数（秒） */

/* 手动/自动模式切换时机（周期数） */
#define MODE_SWITCH_TO_MANUAL  150  /**< 第 150 周期切换到手动模式 */
#define MODE_SWITCH_TO_AUTO    170  /**< 第 170 周期切回自动模式 */
#define MANUAL_OUTPUT          50.0f /**< 手动模式输出值（%） */
```

---

### 4. CSV 输出辅助宏

**用途**: 提供统一的 CSV 输出格式宏定义。

**编码规范**: 所有 CSV 输出使用 **UTF-8 无 BOM** 编码，直接通过 printf 输出到 stdout。

**错误处理约定**:
- 初始化函数返回 `int` 类型：`0` 表示成功，`-1` 表示配置无效
- 配置无效时通过 `fprintf(stderr, "...")` 输出警告信息
- 配置无效时自动使用安全默认值继续运行

**结构定义**:

```c
/**
 * @file csv_output.h
 * @brief CSV 输出格式定义（UTF-8 无 BOM 编码）
 */

/* PID 控制演示 CSV 格式 */
#define CSV_HEADER_PID \
    "time,setpoint,process_value,filtered_pv,control_output,status\n"
#define CSV_FORMAT_PID \
    "%.3f,%.2f,%.2f,%.2f,%.2f,%d\n"

/* 斜坡限幅演示 CSV 格式 */
#define CSV_HEADER_RAMP_LIMIT \
    "time,target,ramp_output,limited_output,limit_status\n"
#define CSV_FORMAT_RAMP_LIMIT \
    "%.3f,%.2f,%.2f,%.2f,%d\n"

/* 信号处理链演示 CSV 格式 */
#define CSV_HEADER_FILTER \
    "time,raw_input,filtered,deadband_out,limited_out,status\n"
#define CSV_FORMAT_FILTER \
    "%.3f,%.2f,%.2f,%.2f,%.2f,%d\n"

/* 积分器/微分器演示 CSV 格式 */
#define CSV_HEADER_INT_DERIV \
    "time,input,integral,derivative,int_status,deriv_status\n"
#define CSV_FORMAT_INT_DERIV \
    "%.3f,%.2f,%.4f,%.4f,%d,%d\n"

/* 综合系统演示 CSV 格式 */
#define CSV_HEADER_FULL_SYSTEM \
    "time,setpoint,ramp_sp,process_value,filtered_pv,control_output,status\n"
#define CSV_FORMAT_FULL_SYSTEM \
    "%.3f,%.2f,%.2f,%.2f,%.2f,%.2f,%d\n"
```

---

## 实体关系图

```
┌─────────────────┐     ┌─────────────────┐
│ NoiseGenerator  │     │  SimulationConfig│
│   (噪声发生器)   │     │   (仿真配置宏)   │
└────────┬────────┘     └────────┬────────┘
         │                       │
         │ 添加噪声               │ 参数来源
         ▼                       ▼
┌─────────────────┐     ┌─────────────────┐
│   PlantModel    │◄────│ PLCOpen 功能块  │
│  (被控对象模型)  │     │  (PID, PT1 等)  │
└────────┬────────┘     └────────┬────────┘
         │                       │
         │ 过程输出               │ 控制输出
         │                       │
         ▼                       ▼
┌─────────────────────────────────────────┐
│           CSV 输出 (stdio.h printf)      │
└─────────────────────────────────────────┘
```

---

## 状态转换

### PlantModel 状态转换

```
                    ┌──────────────┐
         Init()    │  first_run   │
        ──────────►│   = true     │
                    └──────┬───────┘
                           │
                           │ Execute() 首次调用
                           ▼
                    ┌──────────────┐
                    │  first_run   │◄─────┐
                    │   = false    │      │
                    └──────┬───────┘      │
                           │              │
                           │ Execute()    │
                           └──────────────┘
```

### NoiseGenerator 状态转换

```
                    ┌──────────────┐
         Init()    │  state = seed│
        ──────────►│              │
                    └──────┬───────┘
                           │
                           │ Generate()
                           ▼
                    ┌──────────────┐
                    │ state更新    │◄─────┐
                    │ (LCG 迭代)   │      │
                    └──────┬───────┘      │
                           │              │
                           │ Generate()   │
                           └──────────────┘
```

---

## 文件结构

```
examples/plcopen/common/
├── sim_config.h        # 仿真参数宏定义
├── csv_output.h        # CSV 输出格式宏
├── plant_model.h       # 被控对象模型头文件
├── plant_model.c       # 被控对象模型实现
├── noise_generator.h   # 噪声发生器头文件
├── noise_generator.c   # 噪声发生器实现
└── CMakeLists.txt      # 共享库构建配置
```
