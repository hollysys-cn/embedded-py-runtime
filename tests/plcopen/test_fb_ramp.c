/**
 * @file test_fb_ramp.c
 * @brief RAMP 功能块单元测试
 */

#include "unity.h"
#include "plcopen/fb_ramp.h"
#include <math.h>
#include <string.h>

static FB_RAMP_t fb;

void setUp(void) {
    memset(&fb, 0, sizeof(FB_RAMP_t));
}

void tearDown(void) {}

// ============ 配置验证测试 ============

void test_ramp_init_valid_config(void) {
    FB_RAMP_Config_t config = {
        .rise_rate = 10.0f,
        .fall_rate = 5.0f,
        .sample_time = 0.1f
    };

    int result = FB_RAMP_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, fb.config.rise_rate);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, fb.config.fall_rate);
    TEST_ASSERT_EQUAL_FLOAT(0.1f, fb.config.sample_time);
    TEST_ASSERT_TRUE(fb.state.first_run);
}

void test_ramp_init_zero_rise_rate(void) {
    FB_RAMP_Config_t config = {.rise_rate = 0.0f, .fall_rate = 5.0f, .sample_time = 0.1f};

    int result = FB_RAMP_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_ramp_init_zero_fall_rate(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 0.0f, .sample_time = 0.1f};

    int result = FB_RAMP_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_ramp_init_invalid_sample_time(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.0f};

    int result = FB_RAMP_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_ramp_init_excessive_sample_time(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 1001.0f};

    int result = FB_RAMP_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

// ============ 首次调用测试 ============

void test_ramp_first_call_immediate_set(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    float output = FB_RAMP_Execute(&fb, 100.0f);

    // 首次调用应该立即设置到目标值
    TEST_ASSERT_EQUAL_FLOAT(100.0f, output);
    TEST_ASSERT_FALSE(fb.state.first_run);
}

// ============ 上升斜坡测试 ============

void test_ramp_rising_slope(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    // 首次调用设置到 0
    FB_RAMP_Execute(&fb, 0.0f);

    // 设置目标为 5.0，速率 10 单位/秒，采样时间 0.1s，每次变化 1.0
    float output = FB_RAMP_Execute(&fb, 5.0f);

    TEST_ASSERT_EQUAL_FLOAT(1.0f, output);
}

void test_ramp_rising_reach_target(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    FB_RAMP_Execute(&fb, 0.0f);  // 首次调用

    // 多次调用直到到达目标 5.0
    for (int i = 0; i < 6; i++) {
        FB_RAMP_Execute(&fb, 5.0f);
    }

    float output = FB_RAMP_Execute(&fb, 5.0f);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, output);
}

// ============ 下降斜坡测试 ============

void test_ramp_falling_slope(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    FB_RAMP_Execute(&fb, 10.0f);  // 首次调用设置到 10

    // 设置目标为 8.0，下降速率 5 单位/秒，采样时间 0.1s，每次变化 0.5
    float output = FB_RAMP_Execute(&fb, 8.0f);

    TEST_ASSERT_EQUAL_FLOAT(9.5f, output);
}

// ============ 不对称速率测试 ============

void test_ramp_asymmetric_rates(void) {
    FB_RAMP_Config_t config = {.rise_rate = 20.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    FB_RAMP_Execute(&fb, 0.0f);  // 首次调用

    // 上升：速率 20，每次变化 2.0
    float rise_output = FB_RAMP_Execute(&fb, 10.0f);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, rise_output);

    // 设置到高值
    FB_RAMP_Execute(&fb, 10.0f);
    FB_RAMP_Execute(&fb, 10.0f);
    FB_RAMP_Execute(&fb, 10.0f);
    FB_RAMP_Execute(&fb, 10.0f);
    FB_RAMP_Execute(&fb, 10.0f);

    // 下降：速率 5，每次变化 0.5
    float fall_output = FB_RAMP_Execute(&fb, 0.0f);
    TEST_ASSERT_EQUAL_FLOAT(9.5f, fall_output);
}

// ============ 目标值突变测试 ============

void test_ramp_target_change(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    FB_RAMP_Execute(&fb, 0.0f);
    FB_RAMP_Execute(&fb, 5.0f);  // 开始上升

    // 中途改变目标
    float output = FB_RAMP_Execute(&fb, 10.0f);

    // 应该从当前位置(1.0)继续上升
    TEST_ASSERT_EQUAL_FLOAT(2.0f, output);
}

// ============ 数值保护测试 ============

void test_ramp_nan_input(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    FB_RAMP_Execute(&fb, 5.0f);  // 首次调用
    float prev_output = fb.state.output;

    float output = FB_RAMP_Execute(&fb, NAN);

    // NaN 输入应该保持上次输出
    TEST_ASSERT_EQUAL_FLOAT(prev_output, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_NAN, fb.state.status);
}

void test_ramp_inf_input(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    FB_RAMP_Execute(&fb, 5.0f);
    float prev_output = fb.state.output;

    float output = FB_RAMP_Execute(&fb, INFINITY);

    TEST_ASSERT_EQUAL_FLOAT(prev_output, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_INF, fb.state.status);
}

// ============ 小变化测试 ============

void test_ramp_small_change(void) {
    FB_RAMP_Config_t config = {.rise_rate = 10.0f, .fall_rate = 5.0f, .sample_time = 0.1f};
    FB_RAMP_Init(&fb, &config);

    FB_RAMP_Execute(&fb, 0.0f);

    // 目标变化小于最大变化量，应该一步到位
    float output = FB_RAMP_Execute(&fb, 0.5f);

    TEST_ASSERT_EQUAL_FLOAT(0.5f, output);
}

// ============ 测试套件 ============

void run_test_fb_ramp(void) {
    RUN_TEST(test_ramp_init_valid_config);
    RUN_TEST(test_ramp_init_zero_rise_rate);
    RUN_TEST(test_ramp_init_zero_fall_rate);
    RUN_TEST(test_ramp_init_invalid_sample_time);
    RUN_TEST(test_ramp_init_excessive_sample_time);
    RUN_TEST(test_ramp_first_call_immediate_set);
    RUN_TEST(test_ramp_rising_slope);
    RUN_TEST(test_ramp_rising_reach_target);
    RUN_TEST(test_ramp_falling_slope);
    RUN_TEST(test_ramp_asymmetric_rates);
    RUN_TEST(test_ramp_target_change);
    RUN_TEST(test_ramp_nan_input);
    RUN_TEST(test_ramp_inf_input);
    RUN_TEST(test_ramp_small_change);
}

int main(void) {
    UNITY_BEGIN();
    run_test_fb_ramp();
    return UNITY_END();
}
