# Docker 测试环境使用指南

本目录包含用于 PLCopen 功能块库测试的 Docker 环境配置和脚本。

## 文件说明

```
.
├── Dockerfile.test              # Docker 镜像定义
├── docker-compose.test.yml      # Docker Compose 配置
└── scripts/docker/
    ├── build-test-image.sh      # 构建镜像脚本
    ├── run-tests-in-docker.sh   # 运行测试脚本
    └── generate-coverage-report.sh  # 生成覆盖率报告脚本
```

## 快速开始

### 方法 1：使用脚本（推荐）

#### 1. 构建 Docker 镜像

```bash
./scripts/docker/build-test-image.sh
```

#### 2. 运行测试

```bash
./scripts/docker/run-tests-in-docker.sh
```

此脚本会自动：
- 编译 PLCopen 库（x86 版本）
- 编译 Unity 测试框架
- 运行所有已实现的测试
- 显示测试结果

#### 3. 生成覆盖率报告（可选）

```bash
./scripts/docker/generate-coverage-report.sh
```

报告生成位置：`build/coverage/coverage.html`

### 方法 2：使用 Docker Compose

#### 1. 构建并启动容器

```bash
docker-compose -f docker-compose.test.yml up -d
```

#### 2. 进入容器

```bash
docker-compose -f docker-compose.test.yml exec plcopen-test bash
```

#### 3. 在容器内手动运行测试

```bash
# 编译库
gcc -std=c11 -Wall -Wextra -O2 -I./include -c src/plcopen/common.c -o build/common.o
gcc -std=c11 -Wall -Wextra -O2 -I./include -c src/plcopen/fb_pid.c -o build/fb_pid.o

# 编译测试
gcc -std=c11 -Wall -Wextra \
    -I./include -I./.toolchain/unity/src \
    tests/plcopen/test_common.c \
    build/common.o \
    .toolchain/unity/src/unity.c \
    -lm -o test_common

# 运行测试
./test_common
```

#### 4. 停止容器

```bash
docker-compose -f docker-compose.test.yml down
```

### 方法 3：直接使用 Docker

```bash
# 构建镜像
docker build -f Dockerfile.test -t plcopen-test:latest .

# 运行容器
docker run -it -v $(pwd):/workspace plcopen-test:latest

# 在容器内执行命令
```

## Docker 镜像说明

### 基础镜像
- **Ubuntu 22.04**

### 包含的工具
- **GCC/G++**: C/C++ 编译器
- **ARM 工具链**: `arm-none-eabi-gcc`（交叉编译）
- **构建工具**: make, cmake
- **测试工具**: Unity 框架
- **静态分析**: cppcheck
- **覆盖率工具**: gcovr, lcov
- **开发工具**: git, vim, curl

### 环境变量
- `CC=gcc`
- `CXX=g++`
- `WORKSPACE=/workspace`

## 测试脚本详解

### run-tests-in-docker.sh

自动化测试流程：

1. **检查镜像**: 如果不存在则自动构建
2. **编译库**: 编译所有 PLCopen 源文件（x86）
3. **创建静态库**: `libplcopen.a`
4. **编译测试**:
   - `test_common` (基础功能层)
   - `test_fb_pid` (PID 控制器)
   - `test_fb_pt1` (PT1 滤波器)
5. **运行测试**: 执行所有测试并显示结果

### generate-coverage-report.sh

覆盖率报告生成流程：

1. **编译覆盖率版本**: 使用 `--coverage` 标志
2. **运行测试**: 生成 `.gcda` 数据文件
3. **生成报告**: 使用 `gcovr` 生成 HTML 和 TXT 报告
4. **排除目录**: 排除 `.toolchain/`, `tests/`, `examples/`

## 预期输出

### 成功的测试输出示例

```
=== 编译 PLCopen 库 (x86) ===
编译 common...
编译 fb_pid...
编译 fb_pt1...
✅ 库编译完成: build/x86/libplcopen.a

=== 编译并运行测试 ===

[1/3] 测试基础功能层 (common)...
tests/plcopen/test_common.c:35:test_check_overflow_normal_values:PASS
tests/plcopen/test_common.c:43:test_check_overflow_nan:PASS
...
18 Tests 0 Failures 0 Ignored
OK

[2/3] 测试 PID 控制器...
tests/plcopen/test_fb_pid.c:45:test_pid_init_valid_config:PASS
...
20 Tests 0 Failures 0 Ignored
OK

[3/3] 测试 PT1 滤波器...
tests/plcopen/test_fb_pt1.c:38:test_pt1_init_valid_config:PASS
...
12 Tests 0 Failures 0 Ignored
OK

========================================
  测试完成
========================================
```

### 覆盖率报告示例

```
------------------------------------------------------------------------------
GCC Code Coverage Report
Directory: .
------------------------------------------------------------------------------
File                       Lines    Exec  Cover
------------------------------------------------------------------------------
src/plcopen/common.c         45      43   95.6%
src/plcopen/fb_pid.c        120     115   95.8%
src/plcopen/fb_pt1.c         35      34   97.1%
------------------------------------------------------------------------------
TOTAL                       200     192   96.0%
------------------------------------------------------------------------------
```

## 故障排查

### 问题：Docker 镜像构建失败

**解决方案**:
```bash
# 清理 Docker 缓存
docker system prune -a

# 重新构建
./scripts/docker/build-test-image.sh
```

### 问题：测试编译失败

**可能原因**:
- Unity 框架未下载
- 源文件路径错误

**解决方案**:
```bash
# 确保 Unity 框架存在
ls .toolchain/unity/src/unity.c

# 手动下载（如果需要）
mkdir -p .toolchain/unity
cd .toolchain/unity
git clone https://github.com/ThrowTheSwitch/Unity.git .
```

### 问题：测试运行失败

**检查步骤**:
1. 查看测试输出，确认具体失败的测试用例
2. 检查源代码实现是否正确
3. 验证测试用例的预期值是否合理

## 性能说明

### 镜像大小
- 约 500MB（包含所有开发工具）

### 构建时间
- 首次构建：2-5 分钟（取决于网络速度）
- 后续构建：< 1 分钟（使用缓存）

### 测试执行时间
- 完整测试套件：< 10 秒

## 扩展用途

### 静态分析

```bash
docker run --rm -v $(pwd):/workspace plcopen-test:latest \
    cppcheck --enable=all --std=c11 src/plcopen/
```

### 交叉编译（ARM）

```bash
docker run --rm -v $(pwd):/workspace plcopen-test:latest \
    arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb \
    -I./include -c src/plcopen/common.c -o build/common.o
```

### 持续集成

可以将 Docker 测试集成到 CI/CD 流程中：

```yaml
# .github/workflows/test.yml (示例)
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Run tests in Docker
        run: ./scripts/docker/run-tests-in-docker.sh
```

## 最佳实践

1. **定期更新镜像**: 每月重新构建一次以获取最新的安全更新
2. **使用 .dockerignore**: 避免不必要的文件进入构建上下文
3. **缓存构建产物**: 使用 Docker volumes 缓存 `build/` 目录
4. **清理容器**: 测试完成后及时清理停止的容器

## 相关文档

- [PLCopen 库 README](../README.md)
- [实施报告](../docs/FINAL_IMPLEMENTATION_REPORT_002.md)
- [测试任务列表](../specs/002-plcopen-function-blocks/tasks.md)

---

**创建日期**: 2026-01-18
**版本**: 1.0.0
**维护者**: Hollysys Embedded Team
