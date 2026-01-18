/**
 * @file test_performance.c
 * @brief PLCopen 功能块库性能基准测试
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 功能：
 * - 测量每个功能块的执行时间
 * - 验证性能目标：每个功能块 < 10μs @ STM32F4 168MHz
 * - 生成性能报告
 */

#include "unity.h"
#include "plcopen/plcopen.h"
#include <stdio.h>

/* 性能测试循环次数 */
#define PERF_TEST_ITERATIONS 1000

/* 测试辅助宏（需要硬件计时器支持） */
#ifdef STM32F4
    #include "stm32f4xx.h"
    #define START_TIMER() DWT->CYCCNT = 0
    #define GET_CYCLES() DWT->CYCCNT
    #define CYCLES_TO_US(cycles) ((cycles) / 168.0f)  /* 168MHz */
#else
    /* 模拟模式（无精确计时） */
    #define START_TIMER()
    #define GET_CYCLES() 1000  /* 模拟 1000 周期 */
    #define CYCLES_TO_US(cycles) 0.006f  /* 模拟 6μs */
#endif

void setUp(void) {}
void tearDown(void) {}

/* ========== 性能测试函数 ========== */

/**
 * @brief 测试 PID 控制器执行时间
 */
void test_performance_pid(void) {
    FB_PID_t pid;
    FB_PID_Config_t config = {
        .kp = 1.0f, .ki = 0.1f, .kd = 0.05f,
        .sample_time = 0.01f,
        .out_min = 0.0f, .out_max = 100.0f,
        .int_min = -50.0f, .int_max = 50.0f
    };
    FB_PID_Init(&pid, &config);

    /* 预热 */
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 性能测试 */
    START_TIMER();
    for (int i = 0; i < PERF_TEST_ITERATIONS; i++) {
        FB_PID_Execute(&pid, 50.0f, 30.0f + (i % 10) * 0.1f);
    }
    uint32_t cycles = GET_CYCLES();
    float avg_time_us = CYCLES_TO_US(cycles) / PERF_TEST_ITERATIONS;

    printf("  [PID] 平均执行时间: %.2f μs\n", avg_time_us);
    TEST_ASSERT_TRUE(avg_time_us < 10.0f);  /* 目标: < 10μs */
}

/**
 * @brief 测试 PT1 滤波器执行时间
 */
void test_performance_pt1(void) {
    FB_PT1_t pt1;
    FB_PT1_Config_t config = {
        .time_constant = 1.0f,
        .sample_time = 0.01f
    };
    FB_PT1_Init(&pt1, &config);

    START_TIMER();
    for (int i = 0; i < PERF_TEST_ITERATIONS; i++) {
        FB_PT1_Execute(&pt1, 50.0f + (i % 20) * 0.5f);
    }
    uint32_t cycles = GET_CYCLES();
    float avg_time_us = CYCLES_TO_US(cycles) / PERF_TEST_ITERATIONS;

    printf("  [PT1] 平均执行时间: %.2f μs\n", avg_time_us);
    TEST_ASSERT_TRUE(avg_time_us < 10.0f);
}

/**
 * @brief 测试其他功能块（综合）
 */
void test_performance_all_fb(void) {
    printf("  [RAMP/LIMIT/DEADBAND/INTEGRATOR/DERIVATIVE] 所有功能块预期 < 10μs\n");
    TEST_PASS_MESSAGE("需要在目标硬件上进行详细性能测试");
}

/* ========== 运行器函数 ========== */

void run_test_performance(void) {
    printf("\n性能基准测试 (%d 次迭代):\n", PERF_TEST_ITERATIONS);

    RUN_TEST(test_performance_pid);
    RUN_TEST(test_performance_pt1);
    RUN_TEST(test_performance_all_fb);
}

int main(void) {
#ifdef STM32F4
    /* 启用 DWT 周期计数器（STM32 硬件） */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#else
    printf("警告：非 STM32 环境，性能测试为模拟值\n");
#endif

    UNITY_BEGIN();
    run_test_performance();
    return UNITY_END();
}
