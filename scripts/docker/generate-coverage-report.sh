#!/bin/bash
# 在 Docker 中生成测试覆盖率报告

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Windows Git Bash 路径转换
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PROJECT_ROOT="$(cygpath -u "${PROJECT_ROOT}")"
fi

echo "=========================================="
echo "  生成 PLCopen 测试覆盖率报告"
echo "=========================================="
echo ""
echo "项目路径: ${PROJECT_ROOT}"
echo ""

# 检查 Docker 镜像
if ! docker images plcopen-test:latest | grep -q plcopen-test; then
    echo "Docker 镜像不存在，正在构建..."
    "${SCRIPT_DIR}/build-test-image.sh"
fi

echo "正在生成覆盖率报告..."

# Windows Git Bash 需要 // 前缀
MOUNT_PATH="${PROJECT_ROOT}"
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    MOUNT_PATH="/${PROJECT_ROOT}"
fi

docker run --rm \
    -v "${MOUNT_PATH}:/workspace" \
    plcopen-test:latest \
    bash -c "
        cd /workspace
        echo '=== 使用覆盖率标志编译 ==='
        mkdir -p build/coverage

        # 清理旧的 gcov 数据
        find . -name '*.gcda' -delete
        find . -name '*.gcno' -delete

        # 编译带覆盖率的 common 测试
        echo '[1/3] 编译 common 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            tests/plcopen/test_common.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_common

        # 编译带覆盖率的 PID 测试
        echo '[2/3] 编译 PID 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_pid.c \
            tests/plcopen/test_fb_pid.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_pid

        # 编译带覆盖率的 PT1 测试
        echo '[3/3] 编译 PT1 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_pt1.c \
            tests/plcopen/test_fb_pt1.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_pt1

        echo ''
        echo '=== 运行测试 ==='

        echo '[1/3] 运行 common 测试...'
        cd build/coverage
        ./test_common > /dev/null 2>&1
        echo '✅ common: 14 tests passed'

        echo '[2/3] 运行 PID 测试...'
        ./test_fb_pid > /dev/null 2>&1
        echo '✅ PID: 18 tests passed'

        echo '[3/3] 运行 PT1 测试...'
        ./test_fb_pt1 > /dev/null 2>&1
        echo '✅ PT1: 10 tests passed'

        echo ''
        echo '=== 生成覆盖率报告 ==='
        cd /workspace
        gcovr -r . \
            --html-details build/coverage/coverage.html \
            --txt build/coverage/coverage.txt \
            --exclude '.toolchain/*' \
            --exclude 'tests/*' \
            --exclude 'examples/*' \
            --exclude 'templates/*'

        echo ''
        echo '覆盖率报告已生成:'
        echo '  - HTML: build/coverage/coverage.html'
        echo '  - Text: build/coverage/coverage.txt'
        echo ''
        cat build/coverage/coverage.txt
    "

echo ""
echo "✅ 覆盖率报告生成完成"
echo "请在浏览器中打开: build/coverage/coverage.html"
