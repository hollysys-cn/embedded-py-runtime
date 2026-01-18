/**
 * @file common.c
 * @brief PLCopen 功能块通用工具函数实现
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 实现通用的数值保护函数，供所有功能块调用。
 * 包含溢出检测、除零保护、NaN/Inf 检测、输出限幅等。
 */

#include "plcopen/common.h"

/**
 * @brief 检查浮点数是否溢出（NaN 或 Inf）
 */
bool check_overflow(float value) {
    return isnan(value) || isinf(value);
}

/**
 * @brief 安全除法（带除零保护）
 *
 * 实现说明：
 * 1. 检查分母绝对值是否小于最小有效值（1e-6）
 * 2. 如果是，则用最小有效值替换，保持原符号
 * 3. 执行除法运算
 */
float safe_divide(float numerator, float denominator) {
    // 检查分母是否接近零
    if (fabsf(denominator) < MIN_VALID_VALUE) {
        // 保持符号，使用最小有效值
        denominator = (denominator >= 0.0f) ? MIN_VALID_VALUE : -MIN_VALID_VALUE;
    }
    return numerator / denominator;
}

/**
 * @brief 检查浮点数是否为 NaN
 */
bool check_nan(float value) {
    return isnan(value);
}

/**
 * @brief 检查浮点数是否为无穷大
 */
bool check_inf(float value) {
    return isinf(value);
}

/**
 * @brief 检查浮点数是否为 NaN 或无穷大
 */
bool check_nan_inf(float value) {
    return isnan(value) || isinf(value);
}

/**
 * @brief 输出限幅函数
 *
 * 实现说明：
 * 1. 如果 value < min_val，返回 min_val
 * 2. 如果 value > max_val，返回 max_val
 * 3. 否则返回 value 本身
 *
 * @note 使用简单的 if-else 比较，避免复杂的宏定义
 */
float clamp_output(float value, float min_val, float max_val) {
    if (value < min_val) {
        return min_val;
    } else if (value > max_val) {
        return max_val;
    } else {
        return value;
    }
}
