#!/bin/bash
# 构建 PLCopen 测试 Docker 镜像脚本

set -e  # 遇到错误立即退出

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "=========================================="
echo "  PLCopen 测试环境 Docker 镜像构建"
echo "=========================================="
echo ""

cd "${PROJECT_ROOT}"

# 检查 Docker 是否安装
if ! command -v docker &> /dev/null; then
    echo "错误：Docker 未安装或不在 PATH 中"
    exit 1
fi

echo "正在构建 Docker 镜像..."
docker build -f Dockerfile.test -t plcopen-test:latest .

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Docker 镜像构建成功！"
    echo ""
    echo "镜像信息："
    docker images plcopen-test:latest
    echo ""
    echo "使用方法："
    echo "  1. 启动容器：docker run -it -v \$(pwd):/workspace plcopen-test:latest"
    echo "  2. 或使用 docker-compose："
    echo "     docker-compose -f docker-compose.test.yml up -d"
    echo "     docker-compose -f docker-compose.test.yml exec plcopen-test bash"
else
    echo ""
    echo "❌ Docker 镜像构建失败"
    exit 1
fi
