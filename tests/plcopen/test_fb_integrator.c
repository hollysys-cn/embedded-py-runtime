/**
 * @file test_fb_integrator.c
 * @brief INTEGRATOR 功能块单元测试
 */

#include "unity.h"
#include "plcopen/fb_integrator.h"
#include <math.h>
#include <string.h>

static FB_INTEGRATOR_t fb;

void setUp(void) {
    memset(&fb, 0, sizeof(FB_INTEGRATOR_t));
}

void tearDown(void) {}

// ============ 配置验证测试 ============

void test_integrator_init_valid_config(void) {
    FB_INTEGRATOR_Config_t config = {
        .sample_time = 0.1f,
        .out_min = -100.0f,
        .out_max = 100.0f,
        .enable_limit = true
    };

    int result = FB_INTEGRATOR_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, fb.state.integral);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

void test_integrator_init_invalid_sample_time(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.0f, .enable_limit = false};

    int result = FB_INTEGRATOR_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_integrator_init_excessive_sample_time(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 1001.0f, .enable_limit = false};

    int result = FB_INTEGRATOR_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_integrator_init_invalid_limits(void) {
    FB_INTEGRATOR_Config_t config = {
        .sample_time = 0.1f,
        .out_min = 100.0f,
        .out_max = 50.0f,
        .enable_limit = true
    };

    int result = FB_INTEGRATOR_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_integrator_init_no_limit(void) {
    FB_INTEGRATOR_Config_t config = {
        .sample_time = 0.1f,
        .enable_limit = false
    };

    int result = FB_INTEGRATOR_Init(&fb, &config);

    // 无限幅模式不检查 min/max
    TEST_ASSERT_EQUAL_INT(0, result);
}

// ============ 积分累积测试 ============

void test_integrator_constant_input(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.1f, .enable_limit = false};
    FB_INTEGRATOR_Init(&fb, &config);

    // 输入 10.0，采样时间 0.1s，每次增加 1.0
    float output1 = FB_INTEGRATOR_Execute(&fb, 10.0f);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output1);

    float output2 = FB_INTEGRATOR_Execute(&fb, 10.0f);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, output2);

    float output3 = FB_INTEGRATOR_Execute(&fb, 10.0f);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, output3);
}

void test_integrator_zero_input(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.1f, .enable_limit = false};
    FB_INTEGRATOR_Init(&fb, &config);

    // 零输入，积分值应该保持不变
    FB_INTEGRATOR_Execute(&fb, 10.0f);
    float prev = fb.state.integral;

    FB_INTEGRATOR_Execute(&fb, 0.0f);

    TEST_ASSERT_EQUAL_FLOAT(prev, fb.state.integral);
}

void test_integrator_negative_input(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.1f, .enable_limit = false};
    FB_INTEGRATOR_Init(&fb, &config);

    // 负输入应该减小积分值
    FB_INTEGRATOR_Execute(&fb, 10.0f);  // +1.0
    FB_INTEGRATOR_Execute(&fb, -5.0f);   // -0.5

    TEST_ASSERT_EQUAL_FLOAT(0.5f, fb.state.integral);
}

// ============ 限幅测试 ============

void test_integrator_upper_limit(void) {
    FB_INTEGRATOR_Config_t config = {
        .sample_time = 0.1f,
        .out_min = -10.0f,
        .out_max = 10.0f,
        .enable_limit = true
    };
    FB_INTEGRATOR_Init(&fb, &config);

    // 持续正输入直到超出上限
    for (int i = 0; i < 20; i++) {
        FB_INTEGRATOR_Execute(&fb, 10.0f);
    }

    TEST_ASSERT_EQUAL_FLOAT(10.0f, fb.state.integral);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_LIMIT_HI, fb.state.status);
}

void test_integrator_lower_limit(void) {
    FB_INTEGRATOR_Config_t config = {
        .sample_time = 0.1f,
        .out_min = -10.0f,
        .out_max = 10.0f,
        .enable_limit = true
    };
    FB_INTEGRATOR_Init(&fb, &config);

    // 持续负输入直到超出下限
    for (int i = 0; i < 20; i++) {
        FB_INTEGRATOR_Execute(&fb, -10.0f);
    }

    TEST_ASSERT_EQUAL_FLOAT(-10.0f, fb.state.integral);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_LIMIT_LO, fb.state.status);
}

void test_integrator_within_limits(void) {
    FB_INTEGRATOR_Config_t config = {
        .sample_time = 0.1f,
        .out_min = -10.0f,
        .out_max = 10.0f,
        .enable_limit = true
    };
    FB_INTEGRATOR_Init(&fb, &config);

    FB_INTEGRATOR_Execute(&fb, 10.0f);

    TEST_ASSERT_EQUAL_FLOAT(1.0f, fb.state.integral);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

// ============ 复位功能测试 ============

void test_integrator_reset(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.1f, .enable_limit = false};
    FB_INTEGRATOR_Init(&fb, &config);

    // 累积一些值
    FB_INTEGRATOR_Execute(&fb, 10.0f);
    FB_INTEGRATOR_Execute(&fb, 10.0f);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, fb.state.integral);

    // 复位
    FB_INTEGRATOR_Reset(&fb);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, fb.state.integral);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

// ============ 数值保护测试 ============

void test_integrator_nan_input(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.1f, .enable_limit = false};
    FB_INTEGRATOR_Init(&fb, &config);

    FB_INTEGRATOR_Execute(&fb, 10.0f);
    float prev = fb.state.integral;

    float output = FB_INTEGRATOR_Execute(&fb, NAN);

    // NaN 输入不应该改变积分值
    TEST_ASSERT_EQUAL_FLOAT(prev, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_NAN, fb.state.status);
}

void test_integrator_inf_input(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.1f, .enable_limit = false};
    FB_INTEGRATOR_Init(&fb, &config);

    FB_INTEGRATOR_Execute(&fb, 10.0f);
    float prev = fb.state.integral;

    float output = FB_INTEGRATOR_Execute(&fb, INFINITY);

    TEST_ASSERT_EQUAL_FLOAT(prev, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_INF, fb.state.status);
}

void test_integrator_initial_value_zero(void) {
    FB_INTEGRATOR_Config_t config = {.sample_time = 0.1f, .enable_limit = false};
    FB_INTEGRATOR_Init(&fb, &config);

    // 首次调用前积分值应该为 0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, fb.state.integral);
}

// ============ 测试套件 ============

void run_test_fb_integrator(void) {
    RUN_TEST(test_integrator_init_valid_config);
    RUN_TEST(test_integrator_init_invalid_sample_time);
    RUN_TEST(test_integrator_init_excessive_sample_time);
    RUN_TEST(test_integrator_init_invalid_limits);
    RUN_TEST(test_integrator_init_no_limit);
    RUN_TEST(test_integrator_constant_input);
    RUN_TEST(test_integrator_zero_input);
    RUN_TEST(test_integrator_negative_input);
    RUN_TEST(test_integrator_upper_limit);
    RUN_TEST(test_integrator_lower_limit);
    RUN_TEST(test_integrator_within_limits);
    RUN_TEST(test_integrator_reset);
    RUN_TEST(test_integrator_nan_input);
    RUN_TEST(test_integrator_inf_input);
    RUN_TEST(test_integrator_initial_value_zero);
}

int main(void) {
    UNITY_BEGIN();
    run_test_fb_integrator();
    return UNITY_END();
}
