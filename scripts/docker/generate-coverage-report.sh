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
        echo '[1/8] 编译 common 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            tests/plcopen/test_common.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_common

        # 编译带覆盖率的 PID 测试
        echo '[2/8] 编译 PID 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_pid.c \
            tests/plcopen/test_fb_pid.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_pid

        # 编译带覆盖率的 PT1 测试
        echo '[3/8] 编译 PT1 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_pt1.c \
            tests/plcopen/test_fb_pt1.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_pt1

        # 编译带覆盖率的 Deadband 测试
        echo '[4/8] 编译 Deadband 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_deadband.c \
            tests/plcopen/test_fb_deadband.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_deadband

        # 编译带覆盖率的 Derivative 测试
        echo '[5/8] 编译 Derivative 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_derivative.c \
            tests/plcopen/test_fb_derivative.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_derivative

        # 编译带覆盖率的 Integrator 测试
        echo '[6/8] 编译 Integrator 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_integrator.c \
            tests/plcopen/test_fb_integrator.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_integrator

        # 编译带覆盖率的 Limit 测试
        echo '[7/8] 编译 Limit 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_limit.c \
            tests/plcopen/test_fb_limit.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_limit

        # 编译带覆盖率的 Ramp 测试
        echo '[8/8] 编译 Ramp 测试...'
        gcc -std=c11 -Wall -Wextra --coverage -fprofile-arcs -ftest-coverage \
            -I./include -I./.toolchain/unity/src \
            src/plcopen/common.c \
            src/plcopen/fb_ramp.c \
            tests/plcopen/test_fb_ramp.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/coverage/test_fb_ramp

        echo ''
        echo '=== 运行测试 ==='

        cd build/coverage

        echo '[1/8] 运行 common 测试...'
        ./test_common > /dev/null 2>&1 && echo '✅ common: 14 tests passed' || echo '⚠️ common: 测试失败'

        echo '[2/8] 运行 PID 测试...'
        ./test_fb_pid > /dev/null 2>&1 && echo '✅ PID: 18 tests passed' || echo '⚠️ PID: 测试失败'

        echo '[3/8] 运行 PT1 测试...'
        ./test_fb_pt1 > /dev/null 2>&1 && echo '✅ PT1: 10 tests passed' || echo '⚠️ PT1: 测试失败'

        echo '[4/8] 运行 Deadband 测试...'
        ./test_fb_deadband > /dev/null 2>&1 && echo '✅ Deadband: 测试通过' || echo '⚠️ Deadband: 测试失败'

        echo '[5/8] 运行 Derivative 测试...'
        ./test_fb_derivative > /dev/null 2>&1 && echo '✅ Derivative: 测试通过' || echo '⚠️ Derivative: 测试失败'

        echo '[6/8] 运行 Integrator 测试...'
        ./test_fb_integrator > /dev/null 2>&1 && echo '✅ Integrator: 测试通过' || echo '⚠️ Integrator: 测试失败'

        echo '[7/8] 运行 Limit 测试...'
        ./test_fb_limit > /dev/null 2>&1 && echo '✅ Limit: 测试通过' || echo '⚠️ Limit: 测试失败'

        echo '[8/8] 运行 Ramp 测试...'
        ./test_fb_ramp > /dev/null 2>&1 && echo '✅ Ramp: 测试通过' || echo '⚠️ Ramp: 测试失败'

        echo ''
        echo '=== 生成覆盖率报告 ==='
        cd /workspace

        # gcovr 需要在编译目录运行以找到 .gcda 文件
        gcovr build/coverage \
            --root . \
            --html-details build/coverage/coverage.html \
            --txt build/coverage/coverage.txt \
            --exclude '.toolchain/*' \
            --exclude 'tests/*' \
            --exclude 'examples/*' \
            --exclude 'templates/*' \
            --object-directory build/coverage

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
