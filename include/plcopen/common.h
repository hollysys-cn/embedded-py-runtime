/**
 * @file common.h
 * @brief PLCopen 功能块通用定义和工具函数
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 本文件提供所有功能块共享的通用定义、状态码枚举和数值保护函数。
 * 包含溢出检测、除零保护、NaN/Inf 检测、输出限幅等关键功能。
 *
 * 设计原则：
 * - 所有数值保护函数使用单精度浮点数（float）
 * - 采用防御性编程策略，避免异常值导致系统崩溃
 * - 提供详细的状态码反馈，便于故障诊断
 */

#ifndef PLCOPEN_COMMON_H
#define PLCOPEN_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

/* 最小有效值定义（用于除零保护） */
#define MIN_VALID_VALUE 1e-6f

/* 最大采样周期（秒） */
#define MAX_SAMPLE_TIME 1000.0f

/**
 * @brief 功能块状态码枚举
 *
 * 用于指示功能块的运行状态和错误信息。
 * 正值表示警告状态，负值表示错误状态。
 */
typedef enum {
    FB_STATUS_OK = 0,           /**< 正常运行 */
    FB_STATUS_LIMIT_HI = 1,     /**< 输出达到上限 */
    FB_STATUS_LIMIT_LO = 2,     /**< 输出达到下限 */
    FB_STATUS_ERROR_NAN = -1,   /**< 输入为 NaN（非数） */
    FB_STATUS_ERROR_INF = -2,   /**< 输入为 Inf（无穷大） */
    FB_STATUS_ERROR_CONFIG = -3 /**< 配置参数无效 */
} FB_Status_t;

/**
 * @brief 检查浮点数是否溢出（NaN 或 Inf）
 *
 * @param value 待检查的浮点数
 * @return true 如果值为 NaN 或 Inf
 * @return false 如果值为正常有限数
 *
 * @note 使用 C 标准库的 isnan() 和 isinf() 函数
 */
bool check_overflow(float value);

/**
 * @brief 安全除法（带除零保护）
 *
 * 当分母的绝对值小于 MIN_VALID_VALUE 时，自动替换为 MIN_VALID_VALUE，
 * 避免除零错误。保持分母的符号。
 *
 * @param numerator 分子
 * @param denominator 分母
 * @return float 除法结果
 *
 * @note 如果分母为零或接近零，将使用最小有效值替代
 *
 * @code
 * float result = safe_divide(10.0f, 0.0f);  // 返回 10.0f / 1e-6f
 * @endcode
 */
float safe_divide(float numerator, float denominator);

/**
 * @brief 检查浮点数是否为 NaN
 *
 * @param value 待检查的浮点数
 * @return true 如果值为 NaN
 * @return false 如果值为正常数（包括无穷大）
 */
bool check_nan(float value);

/**
 * @brief 检查浮点数是否为无穷大
 *
 * @param value 待检查的浮点数
 * @return true 如果值为正无穷或负无穷
 * @return false 如果值为有限数（包括 NaN）
 */
bool check_inf(float value);

/**
 * @brief 检查浮点数是否为 NaN 或无穷大
 *
 * @param value 待检查的浮点数
 * @return true 如果值为 NaN 或 Inf
 * @return false 如果值为正常有限数
 *
 * @note 这是 check_nan() 和 check_inf() 的组合
 */
bool check_nan_inf(float value);

/**
 * @brief 输出限幅函数
 *
 * 将值限制在 [min_val, max_val] 范围内。
 * 如果 value < min_val，返回 min_val；
 * 如果 value > max_val，返回 max_val；
 * 否则返回 value 本身。
 *
 * @param value 待限幅的值
 * @param min_val 最小值
 * @param max_val 最大值
 * @return float 限幅后的值
 *
 * @note 要求 max_val >= min_val，否则行为未定义
 *
 * @code
 * float result = clamp_output(105.0f, 0.0f, 100.0f);  // 返回 100.0f
 * @endcode
 */
float clamp_output(float value, float min_val, float max_val);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_COMMON_H */
