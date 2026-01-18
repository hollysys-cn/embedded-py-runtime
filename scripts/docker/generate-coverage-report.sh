#!/bin/bash
# 在 Docker 中生成测试覆盖率报告

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "=========================================="
echo "  生成 PLCopen 测试覆盖率报告"
echo "=========================================="
echo ""

# 检查 Docker 镜像
if ! docker images plcopen-test:latest | grep -q plcopen-test; then
    echo "Docker 镜像不存在，正在构建..."
    "${SCRIPT_DIR}/build-test-image.sh"
fi

echo "正在生成覆盖率报告..."

docker run --rm \
    -v "${PROJECT_ROOT}:/workspace" \
    -w /workspace \
    plcopen-test:latest \
    bash -c "
        echo '=== 使用覆盖率标志编译 ==='
        mkdir -p build/coverage

        # 编译带覆盖率的代码
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            tests/plcopen/test_common.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_common

        echo '=== 运行测试 ==='
        cd build/coverage
        ./test_common

        echo ''
        echo '=== 生成覆盖率报告 ==='
        cd /workspace
        gcovr -r . \
            --html-details build/coverage/coverage.html \
            --txt build/coverage/coverage.txt \
            --exclude '.toolchain/*' \
            --exclude 'tests/*' \
            --exclude 'examples/*'

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
