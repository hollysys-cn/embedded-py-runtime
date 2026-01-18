# PLCopen 功能块测试 - CMake 配置
# 使用 Unity 测试框架

# Unity 测试库
add_library(unity STATIC
    ${CMAKE_SOURCE_DIR}/.toolchain/unity/src/unity.c
)
target_include_directories(unity PUBLIC
    ${CMAKE_SOURCE_DIR}/.toolchain/unity/src
)

# 通用测试配置宏
function(add_plcopen_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE plcopen unity m)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# 添加各个功能块的单元测试
add_plcopen_test(test_common test_common.c)
add_plcopen_test(test_fb_pid test_fb_pid.c)
add_plcopen_test(test_fb_pt1 test_fb_pt1.c)
add_plcopen_test(test_fb_ramp test_fb_ramp.c)
add_plcopen_test(test_fb_limit test_fb_limit.c)
add_plcopen_test(test_fb_deadband test_fb_deadband.c)
add_plcopen_test(test_fb_integrator test_fb_integrator.c)
add_plcopen_test(test_fb_derivative test_fb_derivative.c)
add_plcopen_test(test_performance test_performance.c)
