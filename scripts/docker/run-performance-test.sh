#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Windows Git Bash path conversion
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PROJECT_ROOT="$(cygpath -u "${PROJECT_ROOT}")"
fi

echo "=========================================="
echo "  Running PLCopen Performance Tests"
echo "=========================================="

MOUNT_PATH="${PROJECT_ROOT}"
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    MOUNT_PATH="/${PROJECT_ROOT}"
fi

docker run --rm \
    -v "${MOUNT_PATH}:/workspace" \
    plcopen-test:latest \
    bash -c "
        cd /workspace
        mkdir -p build/x86

        echo 'Compiling Performance Tests...'
        gcc -std=c11 -Wall -Wextra -O2 \
            -I./include -I./.toolchain/unity/src \
            tests/plcopen/test_performance.c \
            src/plcopen/common.c \
            src/plcopen/fb_pid.c \
            src/plcopen/fb_pt1.c \
            src/plcopen/fb_ramp.c \
            src/plcopen/fb_limit.c \
            src/plcopen/fb_deadband.c \
            src/plcopen/fb_integrator.c \
            src/plcopen/fb_derivative.c \
            .toolchain/unity/src/unity.c \
            -lm -o build/x86/test_performance

        echo 'Running Performance Tests...'
        ./build/x86/test_performance
    "
