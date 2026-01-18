# PLCopen 测试环境 - 快速开始

## ✅ 已创建的 Docker 测试环境

### 文件清单

1. **Dockerfile.test** - Docker 镜像定义
   - 基于 Ubuntu 22.04
   - 包含 GCC、ARM 工具链、测试工具
   - 使用阿里云镜像源（加速）

2. **docker-compose.test.yml** - Docker Compose 配置
   - 简化容器管理
   - 自动挂载项目目录

3. **.dockerignore** - 排除不必要文件

4. **scripts/docker/** - 自动化脚本
   - `build-test-image.sh` - 构建镜像
   - `run-tests-in-docker.sh` - 自动化测试
   - `generate-coverage-report.sh` - 覆盖率报告

5. **docs/DOCKER_TEST_GUIDE.md** - 详细使用指南

## 🚀 快速使用

### 方法 1：一键运行测试（推荐）

```bash
# 1. 构建镜像（首次运行）
./scripts/docker/build-test-image.sh

# 2. 运行测试
./scripts/docker/run-tests-in-docker.sh
```

### 方法 2：进入容器手动操作

```bash
# 启动容器
docker run -it -v $(pwd):/workspace plcopen-test:latest bash

# 在容器内编译
gcc -std=c11 -Wall -Wextra -I./include -c src/plcopen/common.c

# 运行测试
gcc -I./include -I./.toolchain/unity/src \
    tests/plcopen/test_common.c \
    build/common.o \
    .toolchain/unity/src/unity.c \
    -lm -o test_common && ./test_common
```

### 方法 3：使用 Docker Compose

```bash
# 启动
docker-compose -f docker-compose.test.yml up -d

# 进入
docker-compose -f docker-compose.test.yml exec plcopen-test bash

# 停止
docker-compose -f docker-compose.test.yml down
```

## 📊 预期输出

成功运行测试后，您将看到：

```
=== 编译 PLCopen 库 (x86) ===
✅ 库编译完成: build/x86/libplcopen.a

=== 编译并运行测试 ===

[1/3] 测试基础功能层 (common)...
18 Tests 0 Failures 0 Ignored
OK

[2/3] 测试 PID 控制器...
20 Tests 0 Failures 0 Ignored
OK

[3/3] 测试 PT1 滤波器...
12 Tests 0 Failures 0 Ignored
OK
```

## 🔧 当前状态

### ✅ 完成的工作
- Docker 环境配置完成
- 自动化测试脚本就绪
- 镜像正在构建中（使用阿里云镜像源）

### ⏳ 下一步
1. 等待 Docker 镜像构建完成（预计 5-10 分钟）
2. 运行自动化测试验证所有功能
3. 查看测试报告
4. 生成覆盖率报告（可选）

## 📚 详细文档

- [Docker 测试环境详细指南](DOCKER_TEST_GUIDE.md)
- [最终实施报告](FINAL_IMPLEMENTATION_REPORT_002.md)
- [任务列表](../specs/002-plcopen-function-blocks/tasks.md)

## 💡 提示

### 网络问题
如果构建失败，Dockerfile 已配置为使用阿里云镜像源以加速国内用户访问。

### Windows 路径问题
在 Git Bash 中使用 `$(pwd)` 挂载目录：
```bash
docker run -it -v $(pwd):/workspace plcopen-test:latest
```

在 PowerShell 中使用 `${PWD}`：
```powershell
docker run -it -v ${PWD}:/workspace plcopen-test:latest
```

---

**更新时间**: 2026-01-18
**状态**: Docker 环境构建中
**维护者**: Hollysys Embedded Team
