#!/usr/bin/env bash
#
# OpenOCD 调试服务器启动脚本
# 功能: 启动 OpenOCD 连接调试器（ST-Link, J-Link, CMSIS-DAP）
#
# 使用方式:
#   ./start-debug-server.sh [选项]
#
# 选项:
#   --interface <type>   调试接口类型（stlink-v2|jlink|cmsis-dap），默认: stlink-v2
#   --target <type>      目标 MCU 类型（stm32f4x|stm32f7x），默认: stm32f4x
#   --port <num>         GDB 端口，默认: 3333
#   --daemon             后台运行模式
#   --help               显示帮助信息
#
# 编码: UTF-8
# 换行符: LF

set -euo pipefail

# 获取脚本目录和项目根目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# 引入公共函数
# shellcheck source=../common.sh
source "$PROJECT_ROOT/scripts/common.sh"

#######################################
# 配置变量
#######################################
CONFIG_FILE="${PROJECT_ROOT}/config/debug-openocd.json"

# 默认参数
INTERFACE="stlink-v2"
TARGET="stm32f4x"
GDB_PORT=3333
TELNET_PORT=4444
TCL_PORT=6666
DAEMON_MODE=0

# OpenOCD 可执行文件
OPENOCD_BIN="openocd"

#######################################
# 显示帮助信息
#######################################
show_help() {
    cat << EOF
OpenOCD 调试服务器启动脚本

使用方式:
    $0 [选项]

选项:
    --interface <type>   调试接口类型，默认: stlink-v2
                        支持: stlink-v2, jlink, cmsis-dap
    --target <type>      目标 MCU 类型，默认: stm32f4x
                        支持: stm32f4x, stm32f7x
    --port <num>         GDB 端口，默认: 3333
    --daemon             后台运行模式（守护进程）
    --help               显示帮助信息

示例:
    $0                                  # 使用默认配置启动
    $0 --interface jlink                # 使用 J-Link 接口
    $0 --target stm32f7x --daemon       # 后台运行，目标为 STM32F7
    $0 --port 3334                      # 使用自定义端口

环境变量:
    OPENOCD_BIN         OpenOCD 可执行文件路径（默认: openocd）

配置文件:
    $CONFIG_FILE

EOF
}

#######################################
# 验证接口类型
#######################################
validate_interface() {
    case "$INTERFACE" in
        stlink-v2|jlink|cmsis-dap)
            return 0
            ;;
        *)
            log_error "不支持的接口类型: $INTERFACE"
            log_error "支持的类型: stlink-v2, jlink, cmsis-dap"
            return 1
            ;;
    esac
}

#######################################
# 验证目标类型
#######################################
validate_target() {
    case "$TARGET" in
        stm32f4x|stm32f7x)
            return 0
            ;;
        *)
            log_error "不支持的目标类型: $TARGET"
            log_error "支持的类型: stm32f4x, stm32f7x"
            return 1
            ;;
    esac
}

#######################################
# 检查 OpenOCD 是否安装
#######################################
check_openocd() {
    if [[ -n "${OPENOCD_BIN:-}" ]]; then
        OPENOCD_BIN="${OPENOCD_BIN:-openocd}"
    fi

    if ! command_exists "$OPENOCD_BIN"; then
        log_error "未找到 OpenOCD"
        log_error "请安装 OpenOCD:"
        log_error "  Ubuntu/Debian: sudo apt-get install openocd"
        log_error "  macOS: brew install openocd"
        log_error "  Windows: 下载 https://github.com/xpack-dev-tools/openocd-xpack/releases"
        return 1
    fi

    local version
    version=$("$OPENOCD_BIN" --version 2>&1 | head -n1 || true)
    log_info "OpenOCD 版本: $version"
}

#######################################
# 检查端口是否被占用
#######################################
check_port() {
    local port=$1

    if command_exists netstat; then
        if netstat -an 2>/dev/null | grep -q ":$port.*LISTEN"; then
            log_warn "端口 $port 已被占用"
            return 1
        fi
    elif command_exists lsof; then
        if lsof -i ":$port" 2>/dev/null | grep -q LISTEN; then
            log_warn "端口 $port 已被占用"
            return 1
        fi
    fi

    return 0
}

#######################################
# 启动 OpenOCD
#######################################
start_openocd() {
    log_info "启动 OpenOCD 调试服务器..."
    log_info "  接口: $INTERFACE"
    log_info "  目标: $TARGET"
    log_info "  GDB 端口: $GDB_PORT"
    log_info "  Telnet 端口: $TELNET_PORT"

    # 检查端口
    if ! check_port "$GDB_PORT"; then
        log_error "GDB 端口 $GDB_PORT 被占用"
        log_error "请使用 --port 指定其他端口，或停止占用该端口的进程"
        return 1
    fi

    # 构建 OpenOCD 命令
    local openocd_args=(
        -f "interface/${INTERFACE}.cfg"
        -f "target/${TARGET}.cfg"
        -c "gdb_port $GDB_PORT"
        -c "telnet_port $TELNET_PORT"
        -c "tcl_port $TCL_PORT"
    )

    # 后台运行
    if [[ $DAEMON_MODE -eq 1 ]]; then
        log_info "以守护进程模式启动..."
        "$OPENOCD_BIN" "${openocd_args[@]}" > "$PROJECT_ROOT/build/openocd.log" 2>&1 &
        local pid=$!
        echo "$pid" > "$PROJECT_ROOT/build/openocd.pid"
        log_success "OpenOCD 已启动（PID: $pid）"
        log_info "日志文件: $PROJECT_ROOT/build/openocd.log"
        log_info ""
        log_info "停止 OpenOCD:"
        log_info "  kill $pid"
        log_info "或"
        log_info "  kill \$(cat $PROJECT_ROOT/build/openocd.pid)"
    else
        log_info "按 Ctrl+C 停止 OpenOCD"
        log_info ""
        "$OPENOCD_BIN" "${openocd_args[@]}"
    fi
}

#######################################
# 主函数
#######################################
main() {
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            --interface)
                INTERFACE="$2"
                shift 2
                ;;
            --target)
                TARGET="$2"
                shift 2
                ;;
            --port)
                GDB_PORT="$2"
                shift 2
                ;;
            --daemon)
                DAEMON_MODE=1
                shift
                ;;
            --help)
                show_help
                exit 0
                ;;
            *)
                log_error "未知选项: $1"
                echo ""
                show_help
                exit 1
                ;;
        esac
    done

    # 验证参数
    validate_interface || exit 1
    validate_target || exit 1

    # 检查 OpenOCD
    check_openocd || exit 1

    # 启动服务器
    start_openocd
}

# 运行主函数
main "$@"
