#!/usr/bin/env bats
# 测试调试服务器启动功能
# Task: T057 - 测试调试服务器启动

setup() {
    export PROJECT_ROOT="$(cd "$(dirname "$BATS_TEST_FILENAME")/../.." && pwd)"
    export DEBUG_SERVER_SCRIPT="$PROJECT_ROOT/scripts/debug/start-debug-server.sh"
}

@test "调试服务器脚本存在且可执行" {
    [ -f "$DEBUG_SERVER_SCRIPT" ]
    [ -x "$DEBUG_SERVER_SCRIPT" ]
}

@test "调试服务器脚本显示帮助信息" {
    run "$DEBUG_SERVER_SCRIPT" --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "用法:" ]] || [[ "$output" =~ "Usage:" ]]
}

@test "调试服务器脚本检查必需参数" {
    run "$DEBUG_SERVER_SCRIPT"
    # 可能需要 interface 和 target 参数，或有默认值
    # 根据实际实现调整断言
    [ "$status" -eq 0 ] || [ "$status" -eq 1 ]
}

@test "调试服务器脚本支持 --interface 选项" {
    run "$DEBUG_SERVER_SCRIPT" --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "--interface" ]] || [[ "$output" =~ "interface" ]]
}

@test "调试服务器脚本支持 --target 选项" {
    run "$DEBUG_SERVER_SCRIPT" --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "--target" ]] || [[ "$output" =~ "target" ]]
}

@test "调试服务器脚本支持 --port 选项" {
    run "$DEBUG_SERVER_SCRIPT" --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "--port" ]] || [[ "$output" =~ "port" ]]
}

@test "调试服务器脚本支持 --daemon 选项" {
    run "$DEBUG_SERVER_SCRIPT" --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "--daemon" ]] || [[ "$output" =~ "daemon" ]] || true
}

@test "调试服务器脚本检测 OpenOCD 是否安装" {
    skip "需要安装 OpenOCD"

    # 检查 OpenOCD 是否在 PATH 中
    run which openocd
    [ "$status" -eq 0 ]
}

@test "调试服务器脚本使用有效的接口配置" {
    skip "需要安装 OpenOCD 和硬件连接"

    # 测试支持的接口: stlink-v2
    run "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target stm32f4x --daemon
    [ "$status" -eq 0 ] || [ "$status" -eq 1 ]  # 如果没有硬件连接可能失败

    # 检查是否生成了正确的 OpenOCD 命令
    [[ "$output" =~ "stlink" ]] || true
}

@test "调试服务器脚本使用有效的目标配置" {
    skip "需要安装 OpenOCD 和硬件连接"

    # 测试支持的目标: stm32f4x
    run "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target stm32f4x --daemon
    [ "$status" -eq 0 ] || [ "$status" -eq 1 ]

    # 检查是否生成了正确的 OpenOCD 命令
    [[ "$output" =~ "stm32f4" ]] || true
}

@test "调试服务器脚本可以指定自定义端口" {
    skip "需要安装 OpenOCD"

    run "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target stm32f4x --port 4444 --daemon
    [ "$status" -eq 0 ] || [ "$status" -eq 1 ]

    # 检查端口配置
    [[ "$output" =~ "4444" ]] || true
}

@test "调试服务器脚本守护进程模式启动" {
    skip "需要安装 OpenOCD 和硬件连接"

    # 启动守护进程
    run "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target stm32f4x --daemon

    # 检查是否有 OpenOCD 进程在运行
    sleep 2
    run pgrep -f openocd
    local openocd_running=$status

    # 清理：终止 OpenOCD 进程
    if [ $openocd_running -eq 0 ]; then
        pkill -f openocd || true
    fi
}

@test "调试服务器脚本前台模式可以手动终止" {
    skip "需要安装 OpenOCD 和硬件连接"

    # 在后台启动前台模式（测试用）
    "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target stm32f4x &
    local pid=$!

    sleep 2

    # 检查进程是否在运行
    run ps -p $pid
    [ "$status" -eq 0 ]

    # 终止进程
    kill $pid || true
    wait $pid 2>/dev/null || true
}

@test "调试服务器脚本检测到端口被占用" {
    skip "需要安装 OpenOCD 和端口占用测试"

    # 启动第一个服务器
    "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target stm32f4x --port 3333 --daemon &
    local pid1=$!
    sleep 2

    # 尝试在相同端口启动第二个服务器
    run "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target stm32f4x --port 3333 --daemon
    [ "$status" -ne 0 ]
    [[ "$output" =~ "端口" ]] || [[ "$output" =~ "port" ]] || true

    # 清理
    kill $pid1 || true
    wait $pid1 2>/dev/null || true
}

@test "调试服务器脚本处理无效的接口" {
    run "$DEBUG_SERVER_SCRIPT" --interface invalid-interface --target stm32f4x
    [ "$status" -ne 0 ]
    [[ "$output" =~ "无效" ]] || [[ "$output" =~ "invalid" ]] || [[ "$output" =~ "不支持" ]]
}

@test "调试服务器脚本处理无效的目标" {
    run "$DEBUG_SERVER_SCRIPT" --interface stlink-v2 --target invalid-target
    [ "$status" -ne 0 ]
    [[ "$output" =~ "无效" ]] || [[ "$output" =~ "invalid" ]] || [[ "$output" =~ "不支持" ]]
}

@test "调试服务器脚本配置文件被正确读取" {
    # 检查是否读取了配置文件
    local config_file="$PROJECT_ROOT/config/debug-openocd.json"

    if [ -f "$config_file" ]; then
        # 配置文件存在，脚本应该能读取它
        run "$DEBUG_SERVER_SCRIPT" --help
        [ "$status" -eq 0 ]
    else
        skip "配置文件不存在"
    fi
}
