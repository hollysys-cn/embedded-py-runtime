# 快速入门: 嵌入式 PLCOpen 控制应用程序

**Branch**: `003-embedded-plcopen-app` | **Date**: 2026-01-18

## 概述

本文档提供演示程序的快速构建和运行指南。

---

## 前置条件

- C11 编译器（GCC 4.9+ 或 Clang 3.4+）
- CMake 3.20+
- PLCOpen 功能块库已构建（002 迭代）

---

## 快速构建

### x86 主机构建（开发测试）

```bash
# 进入项目根目录
cd /path/to/embedded-py-runtime

# 创建构建目录
mkdir -p build/x86 && cd build/x86

# 配置 CMake
cmake ../.. -DCMAKE_BUILD_TYPE=Release

# 构建所有演示程序
cmake --build . --target pid_control_demo
cmake --build . --target filter_demo
cmake --build . --target ramp_limit_demo
cmake --build . --target integrator_demo
cmake --build . --target full_system_demo
```

### ARM Cortex-M4 交叉编译

```bash
# 进入项目根目录
cd /path/to/embedded-py-runtime

# 创建构建目录
mkdir -p build/arm && cd build/arm

# 配置 CMake（使用交叉编译工具链）
cmake ../.. \
    -DCMAKE_TOOLCHAIN_FILE=../../config/toolchain.json \
    -DCMAKE_BUILD_TYPE=Release

# 构建演示程序
cmake --build . --target pid_control_demo
```

---

## 运行演示程序

### PID 闭环控制演示

```bash
# 运行程序
./pid_control_demo > pid_output.csv

# 查看结果
head -20 pid_output.csv
```

输出示例：
```csv
time,setpoint,process_value,filtered_pv,control_output,status
0.000,80.00,25.00,25.00,100.00,0
0.100,80.00,26.50,26.45,100.00,1
0.200,80.00,27.95,27.85,100.00,1
...
```

### 斜坡限幅演示

```bash
./ramp_limit_demo > ramp_output.csv
```

### 信号处理链演示

```bash
./filter_demo > filter_output.csv
```

### 积分器/微分器演示

```bash
./integrator_demo > integrator_output.csv
```

### 综合系统演示

```bash
./full_system_demo > full_system_output.csv
```

---

## 数据可视化

### 使用 Python 绘制控制曲线

```python
import pandas as pd
import matplotlib.pyplot as plt

# 读取 CSV 数据
df = pd.read_csv('pid_output.csv')

# 绘制图表
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

# 过程值跟踪设定值
ax1.plot(df['time'], df['setpoint'], 'r--', label='设定值')
ax1.plot(df['time'], df['process_value'], 'b-', label='过程值')
ax1.plot(df['time'], df['filtered_pv'], 'g-', label='滤波值')
ax1.set_xlabel('时间 (s)')
ax1.set_ylabel('温度 (°C)')
ax1.legend()
ax1.set_title('PID 温度控制响应')
ax1.grid(True)

# 控制输出
ax2.plot(df['time'], df['control_output'], 'b-', label='控制输出')
ax2.set_xlabel('时间 (s)')
ax2.set_ylabel('输出 (%)')
ax2.legend()
ax2.set_title('控制器输出')
ax2.grid(True)

plt.tight_layout()
plt.savefig('pid_response.png')
plt.show()
```

### 使用 Excel

1. 打开 Excel
2. 数据 → 从文本/CSV
3. 选择输出的 CSV 文件
4. 插入图表进行可视化

---

## 自定义配置

### 修改仿真参数

编辑 `examples/plcopen/common/sim_config.h`：

```c
// 修改仿真时长
#define SIM_DURATION        30.0f   // 从 20 秒改为 30 秒

// 修改 PID 参数
#define PID_KP              3.0f    // 增加比例增益
#define PID_KI              0.8f    // 增加积分增益

// 修改设定值
#define SETPOINT_INITIAL    100.0f  // 目标温度 100°C
```

然后重新编译：

```bash
cmake --build . --clean-first
```

### 使用 CMake 覆盖参数

```bash
cmake ../.. \
    -DPID_KP=3.0 \
    -DPID_KI=0.8 \
    -DSIM_DURATION=30.0

cmake --build .
```

---

## 项目结构

```
examples/plcopen/
├── CMakeLists.txt              # 顶层 CMake 配置
├── common/                     # 共享辅助模块
│   ├── sim_config.h            # 仿真参数配置
│   ├── csv_output.h            # CSV 输出宏
│   ├── plant_model.h/.c        # 被控对象模型
│   ├── noise_generator.h/.c    # 噪声发生器
│   └── CMakeLists.txt
├── pid_control_demo/           # PID 闭环控制
├── filter_demo/                # 信号处理链
├── ramp_limit_demo/            # 斜坡限幅
├── integrator_demo/            # 积分器/微分器
└── full_system_demo/           # 综合系统
```

---

## 常见问题

### Q: 编译报错找不到 plcopen.h

确保 PLCOpen 功能块库已构建。运行：

```bash
cmake --build . --target plcopen
```

### Q: 如何修改采样周期？

编辑 `sim_config.h` 中的 `SIM_SAMPLE_TIME` 宏，然后重新编译。

### Q: 如何关闭噪声？

将 `NOISE_AMPLITUDE` 设置为 `0.0f`。

---

## 下一步

- 阅读 [data-model.md](data-model.md) 了解数据结构设计
- 阅读 [research.md](research.md) 了解技术决策背景
- 查看 [spec.md](spec.md) 了解完整功能规格
