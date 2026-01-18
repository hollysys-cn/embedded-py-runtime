/**
 * @file test_fb_limit.c
 * @brief LIMIT 功能块单元测试
 */

#include "unity.h"
#include "plcopen/fb_limit.h"
#include <math.h>
#include <string.h>

static FB_LIMIT_t fb;

void setUp(void) {
    // 每个测试前清零
    memset(&fb, 0, sizeof(FB_LIMIT_t));
}

void tearDown(void) {}

// ============ 配置验证测试 ============

void test_limit_init_valid_config(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};

    int result = FB_LIMIT_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_FLOAT(config.min_val, fb.config.min_val);
    TEST_ASSERT_EQUAL_FLOAT(config.max_val, fb.config.max_val);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

void test_limit_init_invalid_range(void) {
    // max <= min 应该失败
    FB_LIMIT_Config_t config = {.min_val = 10.0f, .max_val = 10.0f};

    int result = FB_LIMIT_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_limit_init_null_fb(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};

    int result = FB_LIMIT_Init(NULL, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_limit_init_null_config(void) {
    int result = FB_LIMIT_Init(&fb, NULL);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

// ============ 限幅功能测试 ============

void test_limit_upper_clamp(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, 15.0f);

    TEST_ASSERT_EQUAL_FLOAT(10.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_LIMIT_HI, fb.state.status);
}

void test_limit_lower_clamp(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, -15.0f);

    TEST_ASSERT_EQUAL_FLOAT(-10.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_LIMIT_LO, fb.state.status);
}

void test_limit_within_range(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, 5.0f);

    TEST_ASSERT_EQUAL_FLOAT(5.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

// ============ 边界值测试 ============

void test_limit_at_upper_boundary(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, 10.0f);

    // 等于最大值应该不限幅
    TEST_ASSERT_EQUAL_FLOAT(10.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

void test_limit_at_lower_boundary(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, -10.0f);

    // 等于最小值应该不限幅
    TEST_ASSERT_EQUAL_FLOAT(-10.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

void test_limit_zero_input(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, 0.0f);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

// ============ 数值保护测试 ============

void test_limit_nan_input(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, NAN);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_NAN, fb.state.status);
}

void test_limit_inf_input(void) {
    FB_LIMIT_Config_t config = {.min_val = -10.0f, .max_val = 10.0f};
    FB_LIMIT_Init(&fb, &config);

    float output = FB_LIMIT_Execute(&fb, INFINITY);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_INF, fb.state.status);
}

// ============ 测试套件 ============

void run_test_fb_limit(void) {
    RUN_TEST(test_limit_init_valid_config);
    RUN_TEST(test_limit_init_invalid_range);
    RUN_TEST(test_limit_init_null_fb);
    RUN_TEST(test_limit_init_null_config);
    RUN_TEST(test_limit_upper_clamp);
    RUN_TEST(test_limit_lower_clamp);
    RUN_TEST(test_limit_within_range);
    RUN_TEST(test_limit_at_upper_boundary);
    RUN_TEST(test_limit_at_lower_boundary);
    RUN_TEST(test_limit_zero_input);
    RUN_TEST(test_limit_nan_input);
    RUN_TEST(test_limit_inf_input);
}

int main(void) {
    UNITY_BEGIN();
    run_test_fb_limit();
    return UNITY_END();
}
