/**
 * @file test_fb_deadband.c
 * @brief DEADBAND 功能块单元测试
 */

#include "unity.h"
#include "plcopen/fb_deadband.h"
#include <math.h>
#include <string.h>

static FB_DEADBAND_t fb;

void setUp(void) {
    memset(&fb, 0, sizeof(FB_DEADBAND_t));
}

void tearDown(void) {}

// ============ 配置验证测试 ============

void test_deadband_init_valid_config(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};

    int result = FB_DEADBAND_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, fb.config.width);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, fb.config.center);
}

void test_deadband_init_negative_width(void) {
    FB_DEADBAND_Config_t config = {.width = -1.0f, .center = 50.0f};

    int result = FB_DEADBAND_Init(&fb, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_deadband_init_zero_width(void) {
    FB_DEADBAND_Config_t config = {.width = 0.0f, .center = 50.0f};

    int result = FB_DEADBAND_Init(&fb, &config);

    // 零宽度应该是有效的（直通模式）
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_deadband_init_null_fb(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};

    int result = FB_DEADBAND_Init(NULL, &config);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

// ============ 死区内信号测试 ============

void test_deadband_within_zone(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    // 输入 51.5（偏差 1.5 < 2.0），应该输出中心值
    float output = FB_DEADBAND_Execute(&fb, 51.5f);

    TEST_ASSERT_EQUAL_FLOAT(50.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

void test_deadband_at_center(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    float output = FB_DEADBAND_Execute(&fb, 50.0f);

    TEST_ASSERT_EQUAL_FLOAT(50.0f, output);
}

// ============ 死区外信号测试 ============

void test_deadband_outside_zone_positive(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    // 输入 53.0（偏差 3.0 > 2.0），应该跟随输入
    float output = FB_DEADBAND_Execute(&fb, 53.0f);

    TEST_ASSERT_EQUAL_FLOAT(53.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_OK, fb.state.status);
}

void test_deadband_outside_zone_negative(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    // 输入 47.0（偏差 3.0 > 2.0），应该跟随输入
    float output = FB_DEADBAND_Execute(&fb, 47.0f);

    TEST_ASSERT_EQUAL_FLOAT(47.0f, output);
}

// ============ 边界值测试 ============

void test_deadband_at_upper_boundary(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    // 输入 52.0（偏差恰好等于 2.0），应该输出中心值
    float output = FB_DEADBAND_Execute(&fb, 52.0f);

    TEST_ASSERT_EQUAL_FLOAT(50.0f, output);
}

void test_deadband_at_lower_boundary(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    // 输入 48.0（偏差恰好等于 2.0），应该输出中心值
    float output = FB_DEADBAND_Execute(&fb, 48.0f);

    TEST_ASSERT_EQUAL_FLOAT(50.0f, output);
}

// ============ 零死区测试（直通模式） ============

void test_deadband_zero_width_passthrough(void) {
    FB_DEADBAND_Config_t config = {.width = 0.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    // 零宽度时，任何偏差都大于死区，应该直接输出输入
    float output = FB_DEADBAND_Execute(&fb, 55.0f);

    TEST_ASSERT_EQUAL_FLOAT(55.0f, output);
}

// ============ 数值保护测试 ============

void test_deadband_nan_input(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    float output = FB_DEADBAND_Execute(&fb, NAN);

    // NaN 输入应该输出中心值
    TEST_ASSERT_EQUAL_FLOAT(50.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_NAN, fb.state.status);
}

void test_deadband_inf_input(void) {
    FB_DEADBAND_Config_t config = {.width = 2.0f, .center = 50.0f};
    FB_DEADBAND_Init(&fb, &config);

    float output = FB_DEADBAND_Execute(&fb, INFINITY);

    TEST_ASSERT_EQUAL_FLOAT(50.0f, output);
    TEST_ASSERT_EQUAL_INT(FB_STATUS_ERROR_INF, fb.state.status);
}

// ============ 测试套件 ============

void run_test_fb_deadband(void) {
    RUN_TEST(test_deadband_init_valid_config);
    RUN_TEST(test_deadband_init_negative_width);
    RUN_TEST(test_deadband_init_zero_width);
    RUN_TEST(test_deadband_init_null_fb);
    RUN_TEST(test_deadband_within_zone);
    RUN_TEST(test_deadband_at_center);
    RUN_TEST(test_deadband_outside_zone_positive);
    RUN_TEST(test_deadband_outside_zone_negative);
    RUN_TEST(test_deadband_at_upper_boundary);
    RUN_TEST(test_deadband_at_lower_boundary);
    RUN_TEST(test_deadband_zero_width_passthrough);
    RUN_TEST(test_deadband_nan_input);
    RUN_TEST(test_deadband_inf_input);
}

int main(void) {
    UNITY_BEGIN();
    run_test_fb_deadband();
    return UNITY_END();
}
