/**
 * @file test_common.c
 * @brief PLCopen 通用函数单元测试
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 使用 Unity 测试框架测试通用数值保护函数。
 * 包含溢出检测、除零保护、NaN/Inf 检测、限幅函数的测试用例。
 */

#include "unity.h"
#include "plcopen/common.h"
#include <math.h>

/* Unity 测试框架要求的 setUp 和 tearDown 函数 */
void setUp(void) {
    /* 每个测试用例执行前调用 */
}

void tearDown(void) {
    /* 每个测试用例执行后调用 */
}

/* ========== 溢出检测测试 ========== */

/**
 * @brief 测试正常浮点数不被识别为溢出
 */
void test_check_overflow_normal_values(void) {
    TEST_ASSERT_FALSE(check_overflow(0.0f));
    TEST_ASSERT_FALSE(check_overflow(1.0f));
    TEST_ASSERT_FALSE(check_overflow(-1.0f));
    TEST_ASSERT_FALSE(check_overflow(FLT_MAX));
    TEST_ASSERT_FALSE(check_overflow(-FLT_MAX));
    TEST_ASSERT_FALSE(check_overflow(FLT_MIN));
}

/**
 * @brief 测试 NaN 被正确识别
 */
void test_check_overflow_nan(void) {
    float nan_value = 0.0f / 0.0f;
    TEST_ASSERT_TRUE(check_overflow(nan_value));
}

/**
 * @brief 测试正负无穷大被正确识别
 */
void test_check_overflow_inf(void) {
    float pos_inf = 1.0f / 0.0f;
    float neg_inf = -1.0f / 0.0f;
    TEST_ASSERT_TRUE(check_overflow(pos_inf));
    TEST_ASSERT_TRUE(check_overflow(neg_inf));
}

/* ========== 除零保护测试 ========== */

/**
 * @brief 测试正常除法运算
 */
void test_safe_divide_normal(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, safe_divide(10.0f, 5.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -2.0f, safe_divide(10.0f, -5.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -2.0f, safe_divide(-10.0f, 5.0f));
}

/**
 * @brief 测试分母为零的情况（使用最小有效值）
 */
void test_safe_divide_by_zero(void) {
    float result = safe_divide(10.0f, 0.0f);
    // 应该使用 MIN_VALID_VALUE (1e-6) 替代
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 10.0f / 1e-6f, result);
}

/**
 * @brief 测试分母接近零的情况
 */
void test_safe_divide_near_zero(void) {
    float small_denominator = 1e-7f;  // 小于 MIN_VALID_VALUE
    float result = safe_divide(10.0f, small_denominator);
    // 应该使用 MIN_VALID_VALUE 替代
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 10.0f / 1e-6f, result);
}

/**
 * @brief 测试分母为负零的情况
 */
void test_safe_divide_by_negative_zero(void) {
    float result = safe_divide(10.0f, -1e-7f);
    // 应该使用 -MIN_VALID_VALUE 替代（保持符号）
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 10.0f / (-1e-6f), result);
}

/* ========== NaN 检测测试 ========== */

/**
 * @brief 测试 NaN 检测函数
 */
void test_check_nan_function(void) {
    float nan_value = 0.0f / 0.0f;
    TEST_ASSERT_TRUE(check_nan(nan_value));

    // 正常值不应该被识别为 NaN
    TEST_ASSERT_FALSE(check_nan(0.0f));
    TEST_ASSERT_FALSE(check_nan(1.0f));
    TEST_ASSERT_FALSE(check_nan(-1.0f));

    // 无穷大不是 NaN
    float pos_inf = 1.0f / 0.0f;
    TEST_ASSERT_FALSE(check_nan(pos_inf));
}

/* ========== Inf 检测测试 ========== */

/**
 * @brief 测试无穷大检测函数
 */
void test_check_inf_function(void) {
    float pos_inf = 1.0f / 0.0f;
    float neg_inf = -1.0f / 0.0f;

    TEST_ASSERT_TRUE(check_inf(pos_inf));
    TEST_ASSERT_TRUE(check_inf(neg_inf));

    // 正常值不应该被识别为无穷大
    TEST_ASSERT_FALSE(check_inf(0.0f));
    TEST_ASSERT_FALSE(check_inf(1.0f));
    TEST_ASSERT_FALSE(check_inf(FLT_MAX));

    // NaN 不是无穷大
    float nan_value = 0.0f / 0.0f;
    TEST_ASSERT_FALSE(check_inf(nan_value));
}

/* ========== NaN/Inf 组合检测测试 ========== */

/**
 * @brief 测试 NaN/Inf 组合检测函数
 */
void test_check_nan_inf_function(void) {
    float nan_value = 0.0f / 0.0f;
    float pos_inf = 1.0f / 0.0f;
    float neg_inf = -1.0f / 0.0f;

    // NaN 和 Inf 都应该被检测到
    TEST_ASSERT_TRUE(check_nan_inf(nan_value));
    TEST_ASSERT_TRUE(check_nan_inf(pos_inf));
    TEST_ASSERT_TRUE(check_nan_inf(neg_inf));

    // 正常值不应该被检测到
    TEST_ASSERT_FALSE(check_nan_inf(0.0f));
    TEST_ASSERT_FALSE(check_nan_inf(1.0f));
    TEST_ASSERT_FALSE(check_nan_inf(-1.0f));
    TEST_ASSERT_FALSE(check_nan_inf(FLT_MAX));
}

/* ========== 限幅函数测试 ========== */

/**
 * @brief 测试正常范围内的值不被限幅
 */
void test_clamp_output_within_range(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, clamp_output(50.0f, 0.0f, 100.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, clamp_output(0.0f, 0.0f, 100.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, clamp_output(100.0f, 0.0f, 100.0f));
}

/**
 * @brief 测试超出上限的值被限幅
 */
void test_clamp_output_above_max(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, clamp_output(150.0f, 0.0f, 100.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, clamp_output(20.0f, -10.0f, 10.0f));
}

/**
 * @brief 测试低于下限的值被限幅
 */
void test_clamp_output_below_min(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, clamp_output(-10.0f, 0.0f, 100.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -10.0f, clamp_output(-20.0f, -10.0f, 10.0f));
}

/**
 * @brief 测试边界值（精确等于上下限）
 */
void test_clamp_output_boundary_values(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, clamp_output(0.0f, 0.0f, 100.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, clamp_output(100.0f, 0.0f, 100.0f));
}

/* ========== Unity 测试运行器 ========== */

/**
 * @brief 主测试运行器
 */
int main(void) {
    UNITY_BEGIN();

    /* 溢出检测测试 */
    RUN_TEST(test_check_overflow_normal_values);
    RUN_TEST(test_check_overflow_nan);
    RUN_TEST(test_check_overflow_inf);

    /* 除零保护测试 */
    RUN_TEST(test_safe_divide_normal);
    RUN_TEST(test_safe_divide_by_zero);
    RUN_TEST(test_safe_divide_near_zero);
    RUN_TEST(test_safe_divide_by_negative_zero);

    /* NaN 检测测试 */
    RUN_TEST(test_check_nan_function);

    /* Inf 检测测试 */
    RUN_TEST(test_check_inf_function);

    /* NaN/Inf 组合检测测试 */
    RUN_TEST(test_check_nan_inf_function);

    /* 限幅函数测试 */
    RUN_TEST(test_clamp_output_within_range);
    RUN_TEST(test_clamp_output_above_max);
    RUN_TEST(test_clamp_output_below_min);
    RUN_TEST(test_clamp_output_boundary_values);

    return UNITY_END();
}
