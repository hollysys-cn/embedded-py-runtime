/**
 * @file test_fb_pt1.c
 * @brief PT1 一阶惯性滤波器功能块单元测试
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 测试范围：
 * - 配置验证（time_constant, sample_time）
 * - 阶跃响应（1τ → 63.2%, 3τ → 95%）
 * - 高频噪声抑制
 * - 首次调用行为（无跳变）
 * - 数值保护（NaN/Inf）
 * - 状态码输出
 */

#include "unity.h"
#include "plcopen/fb_pt1.h"
#include <math.h>
#include <string.h>

#define FLOAT_TOLERANCE 0.02f  /* 2% 容差 */

static FB_PT1_t pt1;
static FB_PT1_Config_t config;

void setUp(void) {
    memset(&pt1, 0, sizeof(FB_PT1_t));
    memset(&config, 0, sizeof(FB_PT1_Config_t));

    /* 默认配置：1秒时间常数，10ms 采样 */
    config.time_constant = 1.0f;
    config.sample_time = 0.01f;
}

void tearDown(void) {}

/* ========== 配置验证测试 ========== */

void test_pt1_init_valid_config(void) {
    FB_Status_t status = FB_PT1_Init(&pt1, &config);
    TEST_ASSERT_EQUAL(FB_STATUS_OK, status);
    TEST_ASSERT_TRUE(pt1.state.is_initialized);
}

void test_pt1_init_invalid_time_constant(void) {
    config.time_constant = -1.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PT1_Init(&pt1, &config));

    config.time_constant = 1e-7f;  /* < 1e-6 */
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PT1_Init(&pt1, &config));
}

void test_pt1_init_invalid_sample_time(void) {
    config.sample_time = 0.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PT1_Init(&pt1, &config));

    config.sample_time = 1001.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PT1_Init(&pt1, &config));
}

/* ========== 首次调用测试 ========== */

void test_pt1_first_call_no_jump(void) {
    FB_PT1_Init(&pt1, &config);

    float input = 50.0f;
    float output = FB_PT1_Execute(&pt1, input);

    /* 首次调用输出等于输入（无跳变） */
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_TOLERANCE, input, output);
}

/* ========== 阶跃响应测试 ========== */

void test_pt1_step_response_one_tau(void) {
    config.time_constant = 1.0f;
    config.sample_time = 0.01f;
    FB_PT1_Init(&pt1, &config);

    /* 初始值 0，阶跃到 100 */
    FB_PT1_Execute(&pt1, 0.0f);

    /* 运行 1τ 时间（100 个采样） */
    float output = 0.0f;
    for (int i = 0; i < 100; i++) {
        output = FB_PT1_Execute(&pt1, 100.0f);
    }

    /* 1τ 后应达到 63.2% */
    float expected = 100.0f * (1.0f - expf(-1.0f));  /* ≈ 63.2 */
    TEST_ASSERT_FLOAT_WITHIN(2.0f, expected, output);
}

void test_pt1_step_response_three_tau(void) {
    config.time_constant = 1.0f;
    config.sample_time = 0.01f;
    FB_PT1_Init(&pt1, &config);

    FB_PT1_Execute(&pt1, 0.0f);

    /* 运行 3τ 时间（300 个采样） */
    float output = 0.0f;
    for (int i = 0; i < 300; i++) {
        output = FB_PT1_Execute(&pt1, 100.0f);
    }

    /* 3τ 后应达到 95% */
    float expected = 100.0f * (1.0f - expf(-3.0f));  /* ≈ 95.0 */
    TEST_ASSERT_FLOAT_WITHIN(2.0f, expected, output);
}

/* ========== 噪声抑制测试 ========== */

void test_pt1_noise_rejection(void) {
    config.time_constant = 0.1f;  /* 较短时间常数 */
    config.sample_time = 0.01f;
    FB_PT1_Init(&pt1, &config);

    /* 稳定在 50.0 */
    FB_PT1_Execute(&pt1, 50.0f);
    for (int i = 0; i < 20; i++) {
        FB_PT1_Execute(&pt1, 50.0f);
    }

    /* 添加尖峰噪声 */
    float output_before = FB_PT1_Execute(&pt1, 50.0f);
    float output_spike = FB_PT1_Execute(&pt1, 90.0f);  /* 40 单位尖峰 */

    /* 滤波器应抑制尖峰 */
    TEST_ASSERT_TRUE(output_spike < 60.0f);  /* 远小于输入尖峰 */
    TEST_ASSERT_TRUE(output_spike > output_before);  /* 有响应但被抑制 */
}

/* ========== 数值保护测试 ========== */

void test_pt1_nan_input(void) {
    FB_PT1_Init(&pt1, &config);

    float nan_value = 0.0f / 0.0f;
    FB_Status_t status = FB_PT1_Execute(&pt1, nan_value);

    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_NAN, status);
}

void test_pt1_inf_input(void) {
    FB_PT1_Init(&pt1, &config);

    float inf_value = 1.0f / 0.0f;
    FB_Status_t status = FB_PT1_Execute(&pt1, inf_value);

    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_INF, status);
}

/* ========== 状态码测试 ========== */

void test_pt1_status_ok(void) {
    FB_PT1_Init(&pt1, &config);

    FB_Status_t status = FB_PT1_Execute(&pt1, 50.0f);
    TEST_ASSERT_EQUAL(FB_STATUS_OK, status);
}

/* ========== 运行器函数 ========== */

void run_test_fb_pt1(void) {
    /* 配置验证 */
    RUN_TEST(test_pt1_init_valid_config);
    RUN_TEST(test_pt1_init_invalid_time_constant);
    RUN_TEST(test_pt1_init_invalid_sample_time);

    /* 首次调用 */
    RUN_TEST(test_pt1_first_call_no_jump);

    /* 阶跃响应 */
    RUN_TEST(test_pt1_step_response_one_tau);
    RUN_TEST(test_pt1_step_response_three_tau);

    /* 噪声抑制 */
    RUN_TEST(test_pt1_noise_rejection);

    /* 数值保护 */
    RUN_TEST(test_pt1_nan_input);
    RUN_TEST(test_pt1_inf_input);

    /* 状态码 */
    RUN_TEST(test_pt1_status_ok);
}

int main(void) {
    UNITY_BEGIN();
    run_test_fb_pt1();
    return UNITY_END();
}
