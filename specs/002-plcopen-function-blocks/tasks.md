# Tasks: PLCopen 基础功能块库

**输入**: 设计文档来自 `/specs/002-plcopen-function-blocks/`
**前置条件**: plan.md（技术架构）、spec.md（用户故事）、research.md（技术决策）

**测试策略**: 本项目采用 Unity 嵌入式 C 单元测试框架，所有功能块需要完整的单元测试覆盖率（目标 >90%）

**组织方式**: 任务按用户故事分组，使每个故事可以独立实现和测试

## 格式: `[ID] [P?] [Story] 描述`

- **[P]**: 可并行执行（不同文件，无依赖）
- **[Story]**: 任务所属用户故事（如 US1, US2, US3）
- 描述中包含准确的文件路径

## 路径约定

本项目结构：
- 源代码: `src/plcopen/`
- 头文件: `include/plcopen/`
- 测试: `tests/plcopen/`
- 示例: `examples/plcopen/`
- 工具: `.toolchain/unity/`

---

## Phase 1: Setup（项目初始化）

**目的**: 搭建项目基础设施和构建环境

- [ ] T001 创建 PLCopen 功能块目录结构（src/plcopen/, include/plcopen/, tests/plcopen/, examples/plcopen/）
- [ ] T002 下载和配置 Unity 测试框架到 .toolchain/unity/ 目录
- [ ] T003 [P] 创建 include/plcopen/plcopen.h 主聚合头文件
- [ ] T004 [P] 更新根目录 CMakeLists.txt，添加 plcopen 库构建目标
- [ ] T005 [P] 创建 tests/plcopen/CMakeLists.txt 测试构建配置
- [ ] T006 [P] 创建 examples/plcopen/CMakeLists.txt 示例构建配置

---

## Phase 2: Foundational（基础功能层）

**目的**: 实现所有功能块依赖的公共模块，必须在任何用户故事之前完成

**⚠️ 关键**: 此阶段未完成前，任何用户故事实现都无法开始

- [ ] T007 定义通用状态码枚举 FB_Status_t 在 include/plcopen/common.h
- [ ] T008 [P] 实现数值溢出检测函数 check_overflow() 在 src/plcopen/common.c
- [ ] T009 [P] 实现除零保护函数 safe_divide() 在 src/plcopen/common.c（使用 1e-6 最小值）
- [ ] T010 [P] 实现 NaN/Inf 检测函数 check_nan_inf() 在 src/plcopen/common.c
- [ ] T011 [P] 实现输出限幅函数 clamp_output() 在 src/plcopen/common.c
- [ ] T012 为 common.c 创建单元测试 tests/plcopen/test_common.c：测试溢出检测（输入 FLT_MAX * 2）、除零保护（分母 0 和 1e-6）、NaN 检测（0.0/0.0）、Inf 检测（1.0/0.0）、限幅函数
- [ ] T013 创建 Unity 测试运行器 tests/plcopen/test_runner.c
- [ ] T014 验证基础功能层：编译并运行 test_common.c，确保所有数值保护函数测试通过（覆盖率 > 90%）

**检查点**: 基础功能层就绪 - 用户故事实现现在可以并行开始

---

## Phase 3: User Story 1 - PID 控制器功能块 (Priority: P1) 🎯 MVP

**目标**: 实现符合 PLCopen 标准的 PID 控制器，支持微分项先行滤波、条件积分法抗饱和、手自动模式无扰切换

**独立测试**: 输入阶跃设定值，验证输出收敛到设定值且稳态误差 < 1%

### User Story 1 的实现任务

- [ ] T015 [P] [US1] 定义 FB_PID_Config_t 配置结构体在 include/plcopen/fb_pid.h
- [ ] T016 [P] [US1] 定义 FB_PID_State_t 状态结构体在 include/plcopen/fb_pid.h
- [ ] T017 [P] [US1] 定义 FB_PID_t 完整实例结构体在 include/plcopen/fb_pid.h
- [ ] T018 [US1] 实现 FB_PID_Init() 初始化函数在 src/plcopen/fb_pid.c（内部验证配置参数有效性：sample_time > 0 且 < 1000s, out_max > out_min, int_max > int_min, Kp/Ki/Kd >= 0（允许全零配置用于手动模式或特殊场景），无效参数返回错误码 -1）
- [ ] T019 [US1] 实现 FB_PID_Execute() 核心执行函数在 src/plcopen/fb_pid.c：
  - 计算比例项 P = Kp * error
  - 计算积分项（条件积分法：输出限幅时停止累加）
  - 计算微分项（微分项先行：D = -Kd * d(PV)/dt）
  - 输出限幅和状态更新
- [ ] T020 [US1] 实现 FB_PID_SetManual() 手动模式切换函数在 src/plcopen/fb_pid.c
- [ ] T021 [US1] 实现 FB_PID_SetAuto() 自动模式切换函数（无扰切换）在 src/plcopen/fb_pid.c
- [ ] T022 [US1] 实现首次调用处理：使用首次测量值作为初始输出在 FB_PID_Execute() 中
- [ ] T023 [US1] 创建 PID 功能块单元测试 tests/plcopen/test_fb_pid.c：
  - 测试配置验证：无效 sample_time（<=0 或 >1000s）、out_max <= out_min、int_max <= int_min、负增益参数
  - 测试阶跃响应（收敛性和稳态误差）
  - 测试积分抗饱和（双向条件积分法：上限停止正积分，下限停止负积分）
  - 测试手自动切换（无扰切换）
  - 测试数值保护（溢出、NaN/Inf 输入）
  - 测试首次调用行为（使用首次测量值作为初始输出）
- [ ] T024 [US1] 创建 PID 控制演示程序 examples/plcopen/pid_control_demo/main.c（模拟一阶惯性系统的闭环控制）
- [ ] T025 [US1] 创建 PID 演示程序构建配置 examples/plcopen/pid_control_demo/CMakeLists.txt
- [ ] T026 [US1] 创建 PID 状态码测试 tests/plcopen/test_fb_pid.c：验证所有状态码输出（FB_STATUS_OK, FB_STATUS_LIMIT_HI, FB_STATUS_LIMIT_LO, FB_STATUS_ERROR_NAN, FB_STATUS_ERROR_INF）
- [ ] T027 [US1] 验证 User Story 1：编译并运行所有 PID 测试，执行演示程序，确认控制性能达标

**检查点**: 此时 User Story 1 应完全功能正常且可独立测试

---

## Phase 4: User Story 2 - 一阶惯性（PT1）滤波器功能块 (Priority: P1) 🎯 MVP

**目标**: 实现一阶惯性滤波器，用于信号平滑和噪声抑制

**独立测试**: 输入阶跃信号，验证输出在 1τ 后达到 63.2%，3τ 后达到 95%

### User Story 2 的实现任务

- [ ] T027 [P] [US2] 定义 FB_PT1_Config_t 配置结构体在 include/plcopen/fb_pt1.h
- [ ] T028 [P] [US2] 定义 FB_PT1_State_t 状态结构体在 include/plcopen/fb_pt1.h
- [ ] T029 [P] [US2] 定义 FB_PT1_t 完整实例结构体在 include/plcopen/fb_pt1.h
- [ ] T030 [US2] 实现 FB_PT1_Init() 初始化函数在 src/plcopen/fb_pt1.c（内部验证：time_constant >= 1e-6, sample_time > 0 且 < 1000s，无效参数返回错误码 -1）
- [ ] T031 [US2] 实现 FB_PT1_Execute() 执行函数在 src/plcopen/fb_pt1.c：
  - 使用前向欧拉离散化：alpha = Ts/(τ+Ts)
  - 更新公式：output = output + alpha * (input - output)
  - 首次运行：output = input（无跳变启动）
- [ ] T032 [US2] 创建 PT1 功能块单元测试 tests/plcopen/test_fb_pt1.c：
  - 测试配置验证：time_constant < 1e-6、sample_time <= 0 或 > 1000s
  - 测试阶跃响应（1τ 达到 63.2%，3τ 达到 95%）
  - 测试高频噪声抑制
  - 测试首次调用行为（输出 = 首次输入，无跳变）
  - 测试数值保护（NaN/Inf 输入）
  - 测试状态码输出（FB_STATUS_OK, FB_STATUS_ERROR_NAN, FB_STATUS_ERROR_INF）
- [ ] T033 [US2] 创建 PT1 滤波演示程序 examples/plcopen/filter_demo/main.c（输入带噪声信号，展示滤波效果）
- [ ] T034 [US2] 创建滤波演示程序构建配置 examples/plcopen/filter_demo/CMakeLists.txt
- [ ] T035 [US2] 验证 User Story 2：编译并运行所有 PT1 测试，执行演示程序，确认滤波特性正确

**检查点**: 此时 User Stories 1 和 2 都应独立工作正常（MVP 核心功能完成）

---

## Phase 5: User Story 3 - 斜坡发生器（RAMP）功能块 (Priority: P2)

**目标**: 实现斜坡发生器，用于平滑设定值变化，避免执行机构冲击

**独立测试**: 设置目标值和斜坡速率，验证输出按设定速率线性变化

### User Story 3 的实现任务

- [ ] T037 [P] [US3] 定义 FB_RAMP_Config_t 配置结构体在 include/plcopen/fb_ramp.h（上升速率、下降速率、采样周期）
- [ ] T038 [P] [US3] 定义 FB_RAMP_State_t 状态结构体在 include/plcopen/fb_ramp.h
- [ ] T039 [P] [US3] 定义 FB_RAMP_t 完整实例结构体在 include/plcopen/fb_ramp.h
- [ ] T040 [US3] 实现 FB_RAMP_Init() 初始化函数在 src/plcopen/fb_ramp.c（内部验证：rise_rate > 0, fall_rate > 0, sample_time > 0 且 < 1000s，无效参数返回错误码 -1）
- [ ] T040 [US3] 实现 FB_RAMP_Execute() 执行函数在 src/plcopen/fb_ramp.c：
  - 判断目标值方向（上升/下降）
  - 选择对应速率
  - 线性逼近目标：output += rate * sample_time
  - 到达目标时停止变化
- [ ] T041 [US3] 创建 RAMP 功能块单元测试 tests/plcopen/test_fb_ramp.c：
  - 测试配置验证：rate <= 0、sample_time <= 0 或 > 1000s
  - 测试上升斜坡（速率和时间）
  - 测试下降斜坡
  - 测试目标值突变（从当前值开始新斜坡）
  - 测试不对称速率（上升快下降慢）
  - 测试首次调用行为
  - 测试状态码输出
- [ ] T042 [US3] 验证 User Story 3：编译并运行所有 RAMP 测试，确认斜坡特性正确

**检查点**: 此时 User Stories 1、2 和 3 都应独立工作正常

---

## Phase 6: User Story 4 - 限幅器（LIMIT）功能块 (Priority: P2)

**目标**: 实现限幅器，确保输出信号在安全范围内

**独立测试**: 输入超出范围的信号，验证输出被正确限制

### User Story 4 的实现任务

- [ ] T043 [P] [US4] 定义 FB_LIMIT_Config_t 配置结构体在 include/plcopen/fb_limit.h（上限、下限）
- [ ] T044 [P] [US4] 定义 FB_LIMIT_State_t 状态结构体在 include/plcopen/fb_limit.h（限幅状态标志）
- [ ] T045 [P] [US4] 定义 FB_LIMIT_t 完整实例结构体在 include/plcopen/fb_limit.h
- [ ] T046 [US4] 实现 FB_LIMIT_Init() 初始化函数在 src/plcopen/fb_limit.c（内部验证：max > min，无效参数返回错误码 -1）
- [ ] T047 [US4] 实现 FB_LIMIT_Execute() 执行函数在 src/plcopen/fb_limit.c：
  - 检查输入是否超出范围
  - 限幅到 [min, max]
  - 设置状态标志（HI_LIM/LO_LIM/OK）
- [ ] T048 [US4] 创建 LIMIT 功能块单元测试 tests/plcopen/test_fb_limit.c：
  - 测试配置验证：max <= min
  - 测试上限限幅（状态 FB_STATUS_LIMIT_HI）
  - 测试下限限幅（状态 FB_STATUS_LIMIT_LO）
  - 测试正常范围（状态 FB_STATUS_OK，无限幅）
  - 测试边界值（精确等于 min 或 max）
  - 测试首次调用行为
- [ ] T049 [US4] 验证 User Story 4：编译并运行所有 LIMIT 测试，确认限幅逻辑正确

**检查点**: 此时 User Stories 1-4 都应独立工作正常

---

## Phase 7: User Story 5 - 死区处理（DEADBAND）功能块 (Priority: P3)

**目标**: 实现死区处理，消除微小信号波动

**独立测试**: 输入在死区内变化的信号，验证输出保持不变

### User Story 5 的实现任务

- [ ] T050 [P] [US5] 定义 FB_DEADBAND_Config_t 配置结构体在 include/plcopen/fb_deadband.h（死区宽度、中心值）
- [ ] T051 [P] [US5] 定义 FB_DEADBAND_State_t 状态结构体在 include/plcopen/fb_deadband.h
- [ ] T052 [P] [US5] 定义 FB_DEADBAND_t 完整实例结构体在 include/plcopen/fb_deadband.h
- [ ] T053 [US5] 实现 FB_DEADBAND_Init() 初始化函数在 src/plcopen/fb_deadband.c（内部验证：width >= 0，无效参数返回错误码 -1）
- [ ] T054 [US5] 实现 FB_DEADBAND_Execute() 执行函数在 src/plcopen/fb_deadband.c：
  - 计算输入与中心值的偏差
  - 如果偏差 <= width，输出保持中心值
  - 如果偏差 > width，输出跟随输入
- [ ] T055 [US5] 创建 DEADBAND 功能块单元测试 tests/plcopen/test_fb_deadband.c：
  - 测试配置验证：width < 0
  - 测试死区内信号（输出不变）
  - 测试死区外信号（输出跟随）
  - 测试死区边界值
  - 测试零死区宽度（直通模式）
  - 测试首次调用行为
  - 测试状态码输出
- [ ] T056 [US5] 验证 User Story 5：编译并运行所有 DEADBAND 测试，确认死区逻辑正确

**检查点**: 此时 User Stories 1-5 都应独立工作正常

---

## Phase 8: User Story 6 - 积分器（INTEGRATOR）功能块 (Priority: P3)

**目标**: 实现积分器，用于累计量计算（流量累计、能量累计）

**独立测试**: 输入恒定信号，验证输出随时间线性增加

### User Story 6 的实现任务

- [ ] T057 [P] [US6] 定义 FB_INTEGRATOR_Config_t 配置结构体在 include/plcopen/fb_integrator.h（采样周期、输出限幅）
- [ ] T058 [P] [US6] 定义 FB_INTEGRATOR_State_t 状态结构体在 include/plcopen/fb_integrator.h（积分值、限幅标志）
- [ ] T059 [P] [US6] 定义 FB_INTEGRATOR_t 完整实例结构体在 include/plcopen/fb_integrator.h
- [ ] T060 [US6] 实现 FB_INTEGRATOR_Init() 初始化函数在 src/plcopen/fb_integrator.c（内部验证：sample_time > 0 且 < 1000s, 如配置限幅则 out_max > out_min，无效参数返回错误码 -1）
- [ ] T061 [US6] 实现 FB_INTEGRATOR_Execute() 执行函数在 src/plcopen/fb_integrator.c：
  - 积分计算：integral += input * sample_time
  - 输出限幅（如果配置）
  - 设置限幅状态标志
- [ ] T062 [US6] 实现 FB_INTEGRATOR_Reset() 复位函数在 src/plcopen/fb_integrator.c（归零积分值）
- [ ] T063 [US6] 创建 INTEGRATOR 功能块单元测试 tests/plcopen/test_fb_integrator.c：
  - 测试配置验证：sample_time <= 0 或 > 1000s, out_max <= out_min（如启用限幅）
  - 测试恒定输入累积（线性增长）
  - 测试复位功能
  - 测试输出限幅（状态 FB_STATUS_LIMIT_HI/LO）
  - 测试首次调用行为（积分值从0开始）
  - 测试数值保护（溢出、NaN/Inf 输入）
  - 测试状态码输出
- [ ] T064 [US6] 验证 User Story 6：编译并运行所有 INTEGRATOR 测试，确认积分逻辑正确

**检查点**: 此时 User Stories 1-6 都应独立工作正常

---

## Phase 9: User Story 7 - 微分器（DERIVATIVE）功能块 (Priority: P3)

**目标**: 实现微分器，计算信号变化率（速度、加速度）

**独立测试**: 输入斜坡信号，验证输出为恒定的斜率值

### User Story 7 的实现任务

- [ ] T065 [P] [US7] 定义 FB_DERIVATIVE_Config_t 配置结构体在 include/plcopen/fb_derivative.h（采样周期、滤波时间常数）
- [ ] T066 [P] [US7] 定义 FB_DERIVATIVE_State_t 状态结构体在 include/plcopen/fb_derivative.h（前一采样值、滤波后输出）
- [ ] T067 [P] [US7] 定义 FB_DERIVATIVE_t 完整实例结构体在 include/plcopen/fb_derivative.h
- [ ] T068 [US7] 实现 FB_DERIVATIVE_Init() 初始化函数在 src/plcopen/fb_derivative.c（内部验证：sample_time > 0 且 < 1000s, filter_time_constant >= 0，无效参数返回错误码 -1）
- [ ] T069 [US7] 实现 FB_DERIVATIVE_Execute() 执行函数在 src/plcopen/fb_derivative.c：
  - 计算原始微分：d = (input - prev_input) / sample_time
  - 一阶滤波（如果配置）：output = output + alpha * (d - output)
  - 更新前一采样值
  - 首次运行：输出 0（避免跳变）
- [ ] T070 [US7] 创建 DERIVATIVE 功能块单元测试 tests/plcopen/test_fb_derivative.c：
  - 测试配置验证：sample_time <= 0 或 > 1000s, filter_time_constant < 0
  - 测试斜坡输入（恒定输出）
  - 测试阶跃输入（平滑脉冲而非无穷大尖峰）
  - 测试滤波效果（高 vs 低滤波时间常数）
  - 测试首次调用行为（输出0，避免初始尖峰）
  - 测试数值保护（NaN/Inf 输入）
  - 测试状态码输出
- [ ] T071 [US7] 验证 User Story 7：编译并运行所有 DERIVATIVE 测试，确认微分逻辑正确

**检查点**: 所有用户故事（1-7）现在应该独立功能正常

---

## Phase 10: Polish & Cross-Cutting Concerns（打磨和交叉关注点）

**目的**: 完善文档、优化性能、综合测试

- [ ] T072 [P] 更新 include/plcopen/plcopen.h 主头文件，包含所有 7 个功能块头文件
- [ ] T073 [P] 创建综合系统演示程序 examples/plcopen/full_system_demo/main.c（展示 PID + PT1 + RAMP 协同工作）
- [ ] T074 [P] 创建综合演示程序构建配置 examples/plcopen/full_system_demo/CMakeLists.txt
- [ ] T075 创建性能基准测试 tests/plcopen/test_performance.c（测量每个功能块执行时间，目标 < 10μs）
- [ ] T076 运行性能基准测试，生成性能报告 docs/002-performance-report.md
- [ ] T077 验证内存占用：检查编译后的库大小（目标 Flash < 50KB, RAM < 2KB）
- [ ] T078 [P] 创建用户手册 docs/plcopen-user-manual-zh.md（中文，包含所有功能块使用说明）
- [ ] T079 [P] 为所有源文件添加 Doxygen 格式的 API 文档注释
- [ ] T080 [P] 生成 Doxygen API 文档（如果配置了 Doxygen）
- [ ] T081 运行代码静态分析工具 cppcheck（如果可用），修复发现的问题
- [ ] T082 执行完整测试套件，确认所有测试通过且覆盖率 > 90%
- [ ] T082a 使用 gcov 或类似工具生成测试覆盖率报告，输出到 docs/002-coverage-report.md（确保 SC-005 达标）
- [ ] T083 代码审查：检查代码质量、注释完整性、圈复杂度（目标 < 10）
- [ ] T084 运行 specs/002-plcopen-function-blocks/checklists/requirements.md 中的质量检查清单
- [ ] T085 准备发布：更新 CHANGELOG.md，标记版本 v1.0.0，准备合并到主分支

---

## 依赖关系与执行顺序

### 阶段依赖

- **Setup (Phase 1)**: 无依赖 - 可立即开始
- **Foundational (Phase 2)**: 依赖 Setup 完成 - **阻塞所有用户故事**
- **User Stories (Phase 3-9)**: 全部依赖 Foundational 阶段完成
  - 用户故事之间可以并行（如果有团队资源）
  - 或按优先级顺序（P1 → P2 → P3）
- **Polish (Phase 10)**: 依赖所有期望的用户故事完成

### 用户故事依赖

- **User Story 1 (P1)**: 可在 Foundational (Phase 2) 后开始 - 不依赖其他故事
- **User Story 2 (P1)**: 可在 Foundational (Phase 2) 后开始 - 不依赖其他故事
- **User Story 3 (P2)**: 可在 Foundational (Phase 2) 后开始 - 不依赖其他故事
- **User Story 4 (P2)**: 可在 Foundational (Phase 2) 后开始 - 不依赖其他故事
- **User Story 5 (P3)**: 可在 Foundational (Phase 2) 后开始 - 不依赖其他故事
- **User Story 6 (P3)**: 可在 Foundational (Phase 2) 后开始 - 不依赖其他故事
- **User Story 7 (P3)**: 可在 Foundational (Phase 2) 后开始 - 不依赖其他故事

**关键点**: 所有用户故事设计为独立可测试，完成 Foundational 阶段后可并行开发

### 每个用户故事内部

- 头文件定义（结构体）在实现文件之前
- 核心实现在单元测试之前（但推荐 TDD：先写失败的测试）
- 示例程序在核心功能验证后
- 故事完成后再进入下一优先级

### 并行机会

- Phase 1 所有标记 [P] 的任务可并行
- Phase 2 所有标记 [P] 的任务可并行（T008-T011）
- Phase 2 完成后，所有 7 个用户故事可并行开始（如果团队容量允许）
- 每个用户故事内部，标记 [P] 的任务（通常是头文件定义）可并行
- Phase 10 标记 [P] 的文档任务可并行

---

## 并行示例: MVP 实现（User Stories 1 & 2）

```bash
# 假设 Phase 1 和 Phase 2 已完成

# 开发者 A: 实现 PID 控制器（US1）
# 并行执行头文件定义
task T015 & task T016 & task T017 &
wait

# 串行实现核心逻辑
task T018  # Init
task T019  # Execute
task T020  # SetManual
task T021  # SetAuto
task T022  # 首次调用处理
task T023  # 单元测试
task T024  # 演示程序
task T025  # 构建配置
task T026  # 验证

# 开发者 B: 实现 PT1 滤波器（US2）- 可与 US1 同时进行
# 并行执行头文件定义
task T027 & task T028 & task T029 &
wait

# 串行实现核心逻辑
task T030  # Init
task T031  # Execute
task T032  # 单元测试
task T033  # 演示程序
task T034  # 构建配置
task T035  # 验证

# 两个用户故事都完成后，MVP 就绪
```

---

## 并行示例: P2 优先级功能块（User Stories 3 & 4）

```bash
# 假设 MVP（US1 & US2）已完成

# 开发者 A: 实现 RAMP（US3）
task T036 & task T037 & task T038 &  # 并行头文件
wait
task T039  # Init
task T040  # Execute
task T041  # 单元测试
task T042  # 验证

# 开发者 B: 实现 LIMIT（US4）- 可与 US3 同时进行
task T043 & task T044 & task T045 &  # 并行头文件
wait
task T046  # Init
task T047  # Execute
task T048  # 单元测试
task T049  # 验证

# 两个用户故事都完成后，P2 功能块就绪
```

---

## 建议的实现策略

### MVP 优先方法（推荐）

1. **第 1-2 周**: 完成 Phase 1-2（Setup + Foundational）
2. **第 3-4 周**: 实现 User Stories 1 & 2（PID + PT1）- 这是 MVP
3. **测试 MVP**: 确保核心控制功能可用，获取早期反馈
4. **第 5-6 周**: 实现 User Stories 3 & 4（RAMP + LIMIT）
5. **第 7-8 周**: 实现 User Stories 5-7（DEADBAND + INTEGRATOR + DERIVATIVE）
6. **第 9 周**: Phase 10（打磨、文档、发布）

### 关键里程碑

- **Milestone 1 (2周)**: Setup + Foundational + MVP (US1 + US2)
  - 交付: 可工作的 PID 控制器和 PT1 滤波器
  - 验收: 演示闭环控制，测试覆盖率 > 90%

- **Milestone 2 (1.5周)**: User Stories 3 & 4
  - 交付: RAMP + LIMIT 功能块
  - 验收: 所有测试通过，性能 < 10μs

- **Milestone 3 (1.5周)**: User Stories 5-7
  - 交付: DEADBAND + INTEGRATOR + DERIVATIVE
  - 验收: 7 个功能块全部完成

- **Milestone 4 (0.5周)**: Polish
  - 交付: 完整文档、性能报告、v1.0.0 发布
  - 验收: 通过质量检查清单，准备合并

### MVP 范围

仅 User Stories 1 和 2（PID + PT1），包括：
- T001-T014: Setup + Foundational（基础设施）
- T015-T026: PID 控制器（15-20 个任务）
- T027-T035: PT1 滤波器（约 10 个任务）

**总计**: 约 35 个任务实现 MVP 核心功能

---

## 任务统计摘要

- **总任务数**: 87 个任务（包含 T082a 覆盖率测量任务）
- **Setup 阶段**: 6 个任务
- **Foundational 阶段**: 8 个任务（关键阻塞点）
- **User Story 1 (PID - P1)**: 13 个任务
- **User Story 2 (PT1 - P1)**: 9 个任务
- **User Story 3 (RAMP - P2)**: 7 个任务
- **User Story 4 (LIMIT - P2)**: 7 个任务
- **User Story 5 (DEADBAND - P3)**: 7 个任务
- **User Story 6 (INTEGRATOR - P3)**: 8 个任务
- **User Story 7 (DERIVATIVE - P3)**: 7 个任务
- **Polish 阶段**: 15 个任务（新增覆盖率报告任务 T082a）

### 并行机会识别

- **Setup 阶段**: 4 个任务可并行（T003-T006）
- **Foundational 阶段**: 4 个任务可并行（T008-T011）
- **每个用户故事**: 头文件定义任务可并行（通常 3 个任务）
- **用户故事之间**: 完成 Foundational 后，7 个故事可并行开发

### MVP 范围建议

仅实现 User Stories 1 和 2（PID + PT1）:
- Setup + Foundational: 14 个任务
- User Story 1: 13 个任务
- User Story 2: 9 个任务
- **MVP 总计**: 36 个任务（约 2-3 周工作量）

---

## 格式验证 ✅

**确认**: 所有任务都遵循以下格式：
- ✅ 以 `- [ ]` 开头（Markdown 复选框）
- ✅ 包含任务 ID（T001, T002, ...）
- ✅ P1 优先级用户故事任务标记 [US1] 或 [US2]
- ✅ P2 优先级用户故事任务标记 [US3] 或 [US4]
- ✅ P3 优先级用户故事任务标记 [US5], [US6], [US7]
- ✅ Setup 和 Foundational 阶段任务无故事标签
- ✅ Polish 阶段任务无故事标签
- ✅ 可并行任务标记 [P]
- ✅ 描述包含准确的文件路径

---

**任务列表状态**: ✅ 已生成并修订（修复任务编号、覆盖率测量、测试规范）
**下一步**: 开始 Phase 1（Setup），创建项目目录结构
**最后更新**: 2026-01-18（分析和修复）
