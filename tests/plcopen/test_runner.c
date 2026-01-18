/**
 * @file test_runner.c
 * @brief PLCopen 功能块库 Unity 测试运行器主程序
 *
 * @details
 * 本文件是 PLCopen 功能块库的主测试运行器，整合所有模块的单元测试。
 * 使用 Unity 测试框架运行测试套件。
 *
 * 功能:
 * - 依次运行所有测试组
 * - 统计测试结果
 * - 生成测试报告
 *
 * 使用方法:
 * 1. 编译: arm-none-eabi-gcc test_runner.c test_*.c -I../../include -L../../build -lplcopen
 * 2. 运行: ./test_runner.elf (需要 ARM 环境或 QEMU)
 *
 * @note 本测试套件设计用于嵌入式 ARM Cortex-M4 平台
 *
 * @version 1.0.0
 * @date 2026-01-18
 * @author PLCopen Library Team
 */

#include <stdio.h>
#include "unity.h"

/* 前向声明 - 各测试模块的运行器函数 */
extern void run_test_common(void);        /* 基础功能层测试 */
extern void run_test_fb_pid(void);        /* PID 控制器测试 */
extern void run_test_fb_pt1(void);        /* PT1 滤波器测试 */
extern void run_test_fb_ramp(void);       /* RAMP 斜坡发生器测试 */
extern void run_test_fb_limit(void);      /* LIMIT 限幅器测试 */
extern void run_test_fb_deadband(void);   /* DEADBAND 死区处理测试 */
extern void run_test_fb_integrator(void); /* INTEGRATOR 积分器测试 */
extern void run_test_fb_derivative(void); /* DERIVATIVE 微分器测试 */
extern void run_test_performance(void);   /* 性能基准测试 */

/**
 * @brief 主测试运行器入口
 *
 * @details
 * 依次调用各模块的测试运行器，执行完整的测试套件。
 * 测试顺序：
 * 1. 基础功能层（common）- 最基础的依赖
 * 2. 各功能块单元测试（按优先级：PID, PT1, RAMP, LIMIT, DEADBAND, INTEGRATOR, DERIVATIVE）
 * 3. 性能基准测试（最后执行，不影响功能正确性）
 *
 * @return int 测试结果：0=全部通过，非0=有失败
 */
int main(void) {
    printf("===========================================\n");
    printf("  PLCopen 功能块库 - 单元测试套件\n");
    printf("  Unity 测试框架 v%d.%d.%d\n",
           UNITY_VERSION_MAJOR, UNITY_VERSION_MINOR, UNITY_VERSION_BUILD);
    printf("===========================================\n\n");

    /* 初始化 Unity 测试框架 */
    UNITY_BEGIN();

    /* ===== Phase 1: 基础功能层测试 ===== */
    printf("\n[Phase 1] 基础功能层测试...\n");
    printf("-------------------------------------------\n");
    run_test_common();

    /* ===== Phase 2: P1 优先级功能块测试（MVP）===== */
    printf("\n[Phase 2] P1 优先级功能块测试（MVP）...\n");
    printf("-------------------------------------------\n");

    printf("\n[US1] PID 控制器测试...\n");
    run_test_fb_pid();

    printf("\n[US2] PT1 滤波器测试...\n");
    run_test_fb_pt1();

    /* ===== Phase 3: P2 优先级功能块测试 ===== */
    printf("\n[Phase 3] P2 优先级功能块测试...\n");
    printf("-------------------------------------------\n");

    printf("\n[US3] RAMP 斜坡发生器测试...\n");
    run_test_fb_ramp();

    printf("\n[US4] LIMIT 限幅器测试...\n");
    run_test_fb_limit();

    /* ===== Phase 4: P3 优先级功能块测试 ===== */
    printf("\n[Phase 4] P3 优先级功能块测试...\n");
    printf("-------------------------------------------\n");

    printf("\n[US5] DEADBAND 死区处理测试...\n");
    run_test_fb_deadband();

    printf("\n[US6] INTEGRATOR 积分器测试...\n");
    run_test_fb_integrator();

    printf("\n[US7] DERIVATIVE 微分器测试...\n");
    run_test_fb_derivative();

    /* ===== Phase 5: 性能基准测试 ===== */
    printf("\n[Phase 5] 性能基准测试...\n");
    printf("-------------------------------------------\n");
    run_test_performance();

    /* 结束测试并生成报告 */
    printf("\n===========================================\n");
    printf("  测试完成\n");
    printf("===========================================\n");

    return UNITY_END();
}
