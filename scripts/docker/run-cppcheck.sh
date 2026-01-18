#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Windows Git Bash path conversion
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PROJECT_ROOT="$(cygpath -u "${PROJECT_ROOT}")"
fi

MOUNT_PATH="${PROJECT_ROOT}"
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    MOUNT_PATH="/${PROJECT_ROOT}"
fi

docker run --rm \
    -v "${MOUNT_PATH}:/workspace" \
    plcopen-test:latest \
    bash -c "cd /workspace && cppcheck --enable=warning,performance,portability,information --std=c11 -I include/ src/plcopen/"
