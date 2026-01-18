#!/bin/bash
# 在 Docker 容器中运行 PLCopen 测试套件

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Windows Git Bash 路径转换
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    # 转换 Windows 路径到 Docker 可用格式
    # C:/Users/... -> /c/Users/...
    PROJECT_ROOT="$(cygpath -u "${PROJECT_ROOT}")"
fi

echo "=========================================="
echo "  在 Docker 中运行 PLCopen 测试套件"
echo "=========================================="
echo ""
echo "项目路径: ${PROJECT_ROOT}"
echo ""

# 检查 Docker 镜像是否存在
if ! docker images plcopen-test:latest | grep -q plcopen-test; then
    echo "Docker 镜像不存在，正在构建..."
    "${SCRIPT_DIR}/build-test-image.sh"
fi

echo "正在启动 Docker 容器并运行测试..."
echo ""

# Windows Git Bash 需要 // 前缀来避免路径转换问题
MOUNT_PATH="${PROJECT_ROOT}"
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    MOUNT_PATH="/${PROJECT_ROOT}"
fi

# 运行测试（挂载当前目录到容器）
docker run --rm \
    -v "${MOUNT_PATH}:/workspace" \
    plcopen-test:latest \
    bash -c "
        cd /workspace
        echo '=== 编译 PLCopen 库 (x86) ==='
        mkdir -p build/x86

        # 编译 common 模块
        gcc -std=c11 -Wall -Wextra -O2 \
            -I./include \
            -c src/plcopen/common.c \
            -o build/x86/common.o

        # 编译所有功能块
        for src in src/plcopen/fb_*.c; do
            name=\$(basename \"\${src}\" .c)
            echo \"编译 \${name}...\"
            gcc -std=c11 -Wall -Wextra -O2 \
                -I./include \
                -c \"\${src}\" \
                -o build/x86/\${name}.o
        done

        # 创建静态库
        ar rcs build/x86/libplcopen.a build/x86/*.o
        echo \"✅ 库编译完成: build/x86/libplcopen.a\"
        echo \"\"

        echo '=== 编译并运行测试 ==='

        # 编译 Unity 框架
        gcc -std=c11 -Wall -Wextra -O2 \
            -I./.toolchain/unity/src \
            -c .toolchain/unity/src/unity.c \
            -o build/x86/unity.o

        # 编译并运行 test_common
        echo \"\"
        echo \"[1/3] 测试基础功能层 (common)...\"
        gcc -std=c11 -Wall -Wextra \
            -I./include -I./.toolchain/unity/src \
            tests/plcopen/test_common.c \
            build/x86/common.o \
            build/x86/unity.o \
            -lm -o build/x86/test_common

        ./build/x86/test_common || echo \"⚠️ test_common 有失败\"

        # 编译并运行 test_fb_pid
        echo \"\"
        echo \"[2/3] 测试 PID 控制器...\"
        gcc -std=c11 -Wall -Wextra \
            -I./include -I./.toolchain/unity/src \
            tests/plcopen/test_fb_pid.c \
            build/x86/fb_pid.o \
            build/x86/common.o \
            build/x86/unity.o \
            -lm -o build/x86/test_fb_pid

        ./build/x86/test_fb_pid || echo \"⚠️ test_fb_pid 有失败\"

        # 编译并运行 test_fb_pt1
        echo \"\"
        echo \"[3/3] 测试 PT1 滤波器...\"
        gcc -std=c11 -Wall -Wextra \
            -I./include -I./.toolchain/unity/src \
            tests/plcopen/test_fb_pt1.c \
            build/x86/fb_pt1.o \
            build/x86/common.o \
            build/x86/unity.o \
            -lm -o build/x86/test_fb_pt1

        ./build/x86/test_fb_pt1 || echo \"⚠️ test_fb_pt1 有失败\"

        echo \"\"
        echo \"========================================\"
        echo \"  测试完成\"
        echo \"========================================\"
    "

echo ""
echo "✅ Docker 测试执行完成"
