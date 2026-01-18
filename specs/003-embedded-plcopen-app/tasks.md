# Tasks: 嵌入式 PLCOpen 控制应用程序

**Input**: Design documents from `/specs/003-embedded-plcopen-app/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, quickstart.md ✅

**Tests**: 本功能规格未明确要求单元测试，通过运行演示程序并验证 CSV 输出进行功能验证。

**Organization**: 任务按用户故事分组，支持独立实现和测试。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: 可并行执行（不同文件，无依赖）
- **[Story]**: 所属用户故事（US1, US2, US3 等）
- 描述包含精确文件路径

---

## Phase 1: Setup（项目初始化）

**Purpose**: 创建共享基础设施和目录结构

- [ ] T001 创建 `examples/plcopen/common/` 目录结构
- [ ] T002 [P] 创建仿真配置头文件 `examples/plcopen/common/sim_config.h`
- [ ] T003 [P] 创建 CSV 输出宏定义文件 `examples/plcopen/common/csv_output.h`
- [ ] T004 创建共享模块 CMake 配置 `examples/plcopen/common/CMakeLists.txt`

---

## Phase 2: Foundational（基础模块）

**Purpose**: 实现所有演示程序依赖的共享辅助模块

**⚠️ CRITICAL**: 用户故事实现必须等待本阶段完成

- [ ] T005 [P] 实现被控对象模型头文件 `examples/plcopen/common/plant_model.h`
- [ ] T006 [P] 实现噪声发生器头文件 `examples/plcopen/common/noise_generator.h`
- [ ] T007 实现被控对象模型源文件 `examples/plcopen/common/plant_model.c`
- [ ] T008 实现噪声发生器源文件 `examples/plcopen/common/noise_generator.c`
- [ ] T009 更新顶层 CMake 配置 `examples/plcopen/CMakeLists.txt`（添加 common 和新目录）

**Checkpoint**: 基础模块就绪，用户故事可以开始并行实现

---

## Phase 3: User Story 1 - PID 闭环温度控制演示 (Priority: P1) 🎯 MVP

**Goal**: 实现完整的 PID 闭环控制演示，模拟温度系统，展示 PID + PT1 滤波器应用

**Independent Test**: 运行 `./pid_control_demo > output.csv`，验证温度从 25°C 收敛到 80°C，稳态误差 < 1°C，超调量 < 10%

### Implementation for User Story 1

- [ ] T010 [US1] 更新 CMake 配置 `examples/plcopen/pid_control_demo/CMakeLists.txt`（链接 common 库）
- [ ] T011 [US1] 实现 PID 演示主程序 `examples/plcopen/pid_control_demo/main.c`
  - 初始化 PID、PT1、PlantModel、NoiseGenerator
  - 实现闭环控制循环
  - 展示设定值跟踪（80°C → 60°C 切换）
  - 展示手动/自动模式切换
  - 输出 CSV 格式数据
- [ ] T012 [US1] 验证编译无警告（-Wall -Wextra）
- [ ] T013 [US1] 运行程序验证控制效果

**Checkpoint**: User Story 1 完成，PID 闭环演示可独立运行和验证

---

## Phase 4: User Story 2 - 斜坡信号发生与限幅联合演示 (Priority: P2)

**Goal**: 实现 RAMP + LIMIT 联合演示，模拟电机软启动

**Independent Test**: 运行 `./ramp_limit_demo > output.csv`，验证输出按 10 单位/秒线性上升，被限制在 80

### Implementation for User Story 2

- [ ] T014 [US2] 创建目录 `examples/plcopen/ramp_limit_demo/`
- [ ] T015 [P] [US2] 创建 CMake 配置 `examples/plcopen/ramp_limit_demo/CMakeLists.txt`
- [ ] T016 [US2] 实现斜坡限幅演示主程序 `examples/plcopen/ramp_limit_demo/main.c`
  - 初始化 RAMP、LIMIT 功能块
  - 展示线性上升斜坡
  - 展示目标值突变时的响应
  - 展示限幅状态标志
  - 输出 CSV 格式数据
- [ ] T017 [US2] 验证编译无警告并运行测试

**Checkpoint**: User Story 2 完成，斜坡限幅演示可独立运行

---

## Phase 5: User Story 3 - 信号处理链演示 (Priority: P2)

**Goal**: 实现 PT1 → DEADBAND → LIMIT 信号处理链演示

**Independent Test**: 运行 `./filter_demo > output.csv`，验证噪声被滤除、死区生效、输出被限幅

### Implementation for User Story 3

- [ ] T018 [US3] 更新 CMake 配置 `examples/plcopen/filter_demo/CMakeLists.txt`（链接 common 库）
- [ ] T019 [US3] 实现信号处理链演示主程序 `examples/plcopen/filter_demo/main.c`
  - 初始化 PT1、DEADBAND、LIMIT 功能块
  - 使用 NoiseGenerator 生成带噪声信号
  - 串联处理：滤波 → 死区 → 限幅
  - 输出 CSV 格式数据
- [ ] T020 [US3] 验证编译无警告并运行测试

**Checkpoint**: User Story 3 完成，信号处理链演示可独立运行

---

## Phase 6: User Story 4 - 积分/微分演示 (Priority: P3)

**Goal**: 实现积分器累计量和微分器变化率演示，模拟流量统计和速度计算

**Independent Test**: 运行 `./integrator_demo > output.csv`，验证积分值线性增加、微分值稳定、复位功能正常

### Implementation for User Story 4

- [ ] T021 [US4] 创建目录 `examples/plcopen/integrator_demo/`
- [ ] T022 [P] [US4] 创建 CMake 配置 `examples/plcopen/integrator_demo/CMakeLists.txt`
- [ ] T023 [US4] 实现积分器/微分器演示主程序 `examples/plcopen/integrator_demo/main.c`
  - 初始化 INTEGRATOR、DERIVATIVE 功能块
  - 展示积分累计量计算
  - 展示微分变化率检测
  - 展示复位功能
  - 展示限幅状态
  - 输出 CSV 格式数据
- [ ] T024 [US4] 验证编译无警告并运行测试

**Checkpoint**: User Story 4 完成，积分器/微分器演示可独立运行

---

## Phase 7: User Story 5 - 综合系统演示 (Priority: P2)

**Goal**: 实现 PID + RAMP + PT1 综合演示，展示完整控制系统

**Independent Test**: 运行 `./full_system_demo > output.csv`，验证设定值斜坡变化 + PID 跟踪 + 滤波全流程

### Implementation for User Story 5

- [ ] T025 [US5] 更新 CMake 配置 `examples/plcopen/full_system_demo/CMakeLists.txt`（链接 common 库）
- [ ] T026 [US5] 实现综合系统演示主程序 `examples/plcopen/full_system_demo/main.c`
  - 初始化 RAMP（设定值斜坡）、PID、PT1、LIMIT、PlantModel
  - 展示设定值斜坡变化
  - 展示 PID 跟踪斜坡设定值
  - 展示输出限幅
  - 输出 CSV 格式数据
- [ ] T027 [US5] 验证编译无警告并运行测试

**Checkpoint**: User Story 5 完成，综合系统演示可独立运行

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: 最终打磨和文档更新

- [ ] T028 [P] 运行所有演示程序，验证 CSV 输出可导入 Excel
- [ ] T029 [P] 检查所有代码中文注释完整性
- [ ] T030 [P] 更新 quickstart.md 中的实际构建命令（如有变化）
- [ ] T031 运行长时间循环测试（100,000 次），验证无崩溃
- [ ] T032 验证 ARM 交叉编译成功（如有工具链）

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: 无依赖，可立即开始
- **Foundational (Phase 2)**: 依赖 Setup 完成 - **阻塞所有用户故事**
- **User Stories (Phase 3-7)**: 依赖 Foundational 完成
  - US1 (PID) 作为 MVP 应首先完成
  - US2-US5 可按优先级顺序或并行实现
- **Polish (Phase 8)**: 依赖所有用户故事完成

### User Story Dependencies

| 用户故事 | 依赖 | 可并行 |
|---------|------|--------|
| US1 (PID 闭环) | Phase 2 完成 | 独立 |
| US2 (斜坡限幅) | Phase 2 完成 | 与 US1 并行 |
| US3 (信号处理链) | Phase 2 完成 | 与 US1/US2 并行 |
| US4 (积分/微分) | Phase 2 完成 | 与 US1-3 并行 |
| US5 (综合系统) | Phase 2 完成 | 与 US1-4 并行 |

### Within Each User Story

1. CMake 配置先于 main.c
2. main.c 实现
3. 编译验证
4. 运行测试

### Parallel Opportunities

```
Phase 1 并行:
  T002 (sim_config.h) || T003 (csv_output.h)

Phase 2 并行:
  T005 (plant_model.h) || T006 (noise_generator.h)

Phase 3-7 用户故事并行:
  US1 || US2 || US3 || US4 || US5  （Phase 2 完成后）

Phase 8 并行:
  T028 || T029 || T030
```

---

## Parallel Example: User Story 2 + User Story 3

```bash
# 在 Foundational 完成后，可以同时开始：

# 开发者 A: User Story 2
Task: T014 创建目录 examples/plcopen/ramp_limit_demo/
Task: T015 创建 CMake 配置
Task: T016 实现 main.c
Task: T017 验证

# 开发者 B: User Story 3
Task: T018 更新 CMake 配置
Task: T019 实现 main.c
Task: T020 验证
```

---

## Implementation Strategy

### MVP First (仅 User Story 1)

1. 完成 Phase 1: Setup
2. 完成 Phase 2: Foundational（关键 - 阻塞所有故事）
3. 完成 Phase 3: User Story 1 (PID)
4. **停止并验证**: 运行 `./pid_control_demo`，检查 CSV 输出
5. 可部署/演示 MVP

### Incremental Delivery

1. Setup + Foundational → 基础就绪
2. 添加 US1 (PID) → 测试 → Demo (MVP!)
3. 添加 US2 (斜坡限幅) → 测试 → Demo
4. 添加 US3 (信号处理链) → 测试 → Demo
5. 添加 US4 (积分器) → 测试 → Demo
6. 添加 US5 (综合系统) → 测试 → Demo
7. 每个故事独立增加价值，不破坏之前的功能

---

## Summary

| 统计项 | 值 |
|--------|-----|
| **总任务数** | 32 |
| **Setup 任务** | 4 (T001-T004) |
| **Foundational 任务** | 5 (T005-T009) |
| **US1 (PID) 任务** | 4 (T010-T013) |
| **US2 (斜坡限幅) 任务** | 4 (T014-T017) |
| **US3 (信号处理链) 任务** | 3 (T018-T020) |
| **US4 (积分器) 任务** | 4 (T021-T024) |
| **US5 (综合系统) 任务** | 3 (T025-T027) |
| **Polish 任务** | 5 (T028-T032) |
| **可并行任务** | 14 (标记 [P]) |

### MVP 范围建议

仅实现 **User Story 1 (PID 闭环控制)**:
- 任务: T001-T013 (13 个任务)
- 文件: common/ 目录 + pid_control_demo/main.c
- 验证: 运行程序，检查温度收敛曲线

### 独立测试标准

| 用户故事 | 独立验证方法 |
|---------|-------------|
| US1 | `./pid_control_demo > out.csv && head -20 out.csv` 检查温度收敛 |
| US2 | `./ramp_limit_demo > out.csv` 检查斜坡线性和限幅 |
| US3 | `./filter_demo > out.csv` 检查噪声衰减 |
| US4 | `./integrator_demo > out.csv` 检查积分线性增长和微分变化率 |
| US5 | `./full_system_demo > out.csv` 检查综合控制效果 |

---

## Notes

- 所有任务文件路径相对于仓库根目录
- [P] 任务可与同阶段其他 [P] 任务并行
- [USn] 标签标识任务所属用户故事
- 每个用户故事完成后应可独立运行和验证
- 每个任务或逻辑组完成后提交代码
- 避免: 模糊任务、同文件冲突、破坏独立性的跨故事依赖
