/**
 * @file test_fb_derivative.c
 * @brief DERIVATIVE 功能块单元测试
 */

#include "unity.h"
#include "plcopen/fb_derivative.h"
#include <math.h>
#include <string.h>

static FB_DERIVATIVE_t fb;

void setUp(void) {
    memset(&fb, 0, sizeof(FB_DERIVATIVE_t));
}

void tearDown(void) {}

// ============ 配置验证测试 ============

void test_derivative_init_valid_config(void) {
    FB_DERIVATIVE_Config_t config = {
        .sample_time = 0.1f,
        .filter_time_constant = 0.5f
    };

    int result = FB_DERIVATIVE_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_TRUE(fb.state.first_run);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, fb.state.filtered_output);
}

void test_derivative_init_invalid_sample_time(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.0f, .filter_time_constant = 0.0f};

    int result = FB_DERIVATIVE_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_derivative_init_excessive_sample_time(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 1001.0f, .filter_time_constant = 0.0f};

    int result = FB_DERIVATIVE_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_derivative_init_negative_filter_constant(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = -1.0f};

    int result = FB_DERIVATIVE_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_derivative_init_no_filter(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};

    int result = FB_DERIVATIVE_Init(&fb, &config);

    // 零滤波时间常数应该有效（无滤波模式）
    TEST_ASSERT_EQUAL_INT(0, result);
}

// ============ 首次调用测试 ============

void test_derivative_first_call_zero(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    float output = FB_DERIVATIVE_Execute(&fb, 10.0f);

    // 首次调用应该返回 0（无历史数据）
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
    TEST_ASSERT_FALSE(fb.state.first_run);
}

// ============ 恒定输入测试 ============

void test_derivative_constant_input(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 10.0f);  // 首次调用
    float output = FB_DERIVATIVE_Execute(&fb, 10.0f);  // 恒定输入

    // 恒定输入的导数应该为 0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
}

// ============ 线性变化测试 ============

void test_derivative_linear_change(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 0.0f);   // 首次调用
    float output = FB_DERIVATIVE_Execute(&fb, 10.0f);  // 变化 10.0

    // 导数 = (10.0 - 0.0) / 0.1 = 100.0
    TEST_ASSERT_EQUAL_FLOAT(100.0f, output);
}

void test_derivative_negative_slope(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 10.0f);
    float output = FB_DERIVATIVE_Execute(&fb, 5.0f);  // 变化 -5.0

    // 导数 = (5.0 - 10.0) / 0.1 = -50.0
    TEST_ASSERT_EQUAL_FLOAT(-50.0f, output);
}

void test_derivative_continuous_linear(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 0.0f);
    FB_DERIVATIVE_Execute(&fb, 10.0f);  // 导数 100
    float output = FB_DERIVATIVE_Execute(&fb, 20.0f);  // 继续以相同速率变化

    // 导数应该保持 100.0
    TEST_ASSERT_EQUAL_FLOAT(100.0f, output);
}

// ============ 突变测试 ============

void test_derivative_step_change(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.01f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 0.0f);
    float output = FB_DERIVATIVE_Execute(&fb, 100.0f);  // 大幅突变

    // 导数 = 100.0 / 0.01 = 10000.0
    TEST_ASSERT_EQUAL_FLOAT(10000.0f, output);
}

// ============ 滤波测试 ============

void test_derivative_with_filter(void) {
    FB_DERIVATIVE_Config_t config = {
        .sample_time = 0.1f,
        .filter_time_constant = 0.5f  // 加入滤波
    };
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 0.0f);
    float output = FB_DERIVATIVE_Execute(&fb, 10.0f);

    // 有滤波时，输出应该小于无滤波的 100.0
    // alpha = 0.1 / (0.5 + 0.1) = 0.1667
    // filtered = 0 + 0.1667 * (100 - 0) = 16.67
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 16.67f, output);
}

void test_derivative_filter_convergence(void) {
    FB_DERIVATIVE_Config_t config = {
        .sample_time = 0.1f,
        .filter_time_constant = 0.1f
    };
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 0.0f);

    // 多次相同输入，滤波器应该收敛到实际导数
    for (int i = 0; i < 10; i++) {
        FB_DERIVATIVE_Execute(&fb, 10.0f);
    }

    // 应该接近 0（恒定输入的导数）
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, fb.state.filtered_output);
}

// ============ 数值保护测试 ============

void test_derivative_nan_input(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 10.0f);
    float output = FB_DERIVATIVE_Execute(&fb, NAN);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_NAN, fb.state.status);
}

void test_derivative_inf_input(void) {
    FB_DERIVATIVE_Config_t config = {.sample_time = 0.1f, .filter_time_constant = 0.0f};
    FB_DERIVATIVE_Init(&fb, &config);

    FB_DERIVATIVE_Execute(&fb, 10.0f);
    float output = FB_DERIVATIVE_Execute(&fb, INFINITY);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_INF, fb.state.status);
}

// ============ 测试套件 ============

void run_test_fb_derivative(void) {
    RUN_TEST(test_derivative_init_valid_config);
    RUN_TEST(test_derivative_init_invalid_sample_time);
    RUN_TEST(test_derivative_init_excessive_sample_time);
    RUN_TEST(test_derivative_init_negative_filter_constant);
    RUN_TEST(test_derivative_init_no_filter);
    RUN_TEST(test_derivative_first_call_zero);
    RUN_TEST(test_derivative_constant_input);
    RUN_TEST(test_derivative_linear_change);
    RUN_TEST(test_derivative_negative_slope);
    RUN_TEST(test_derivative_continuous_linear);
    RUN_TEST(test_derivative_step_change);
    RUN_TEST(test_derivative_with_filter);
    RUN_TEST(test_derivative_filter_convergence);
    RUN_TEST(test_derivative_nan_input);
    RUN_TEST(test_derivative_inf_input);
}

int main(void) {
    UNITY_BEGIN();
    run_test_fb_derivative();
    return UNITY_END();
}
