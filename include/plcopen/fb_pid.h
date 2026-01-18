/**
 * @file fb_pid.h
 * @brief PLCopen PID 控制器功能块
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 实现符合 PLCopen 标准的 PID（比例-积分-微分）控制器。
 *
 * 主要特性：
 * - 微分项先行滤波（Derivative on Measurement）：避免设定值突变冲击
 * - 条件积分法抗饱和（Conditional Integration）：输出限幅时停止积分累加
 * - 手自动模式无扰切换：切换时保持输出连续
 * - 完整的数值保护：溢出、NaN/Inf 检测
 *
 * 典型应用：
 * - 温度控制
 * - 压力控制
 * - 流量控制
 * - 位置伺服控制
 *
 * 使用示例：
 * @code
 * FB_PID_t my_pid;
 * FB_PID_Config_t config = {
 *     .kp = 1.0f, .ki = 0.1f, .kd = 0.05f,
 *     .sample_time = 0.01f,
 *     .out_min = 0.0f, .out_max = 100.0f,
 *     .int_min = -50.0f, .int_max = 50.0f
 * };
 * FB_PID_Init(&my_pid, &config);
 *
 * // 周期执行（10ms）
 * float output = FB_PID_Execute(&my_pid, 50.0f, 45.0f);
 * @endcode
 */

#ifndef PLCOPEN_FB_PID_H
#define PLCOPEN_FB_PID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plcopen/common.h"

/**
 * @brief PID 控制器配置参数
 *
 * 所有参数必须在 Init 函数调用前设置。
 */
typedef struct {
    float kp;              /**< 比例增益（>= 0） */
    float ki;              /**< 积分增益（>= 0） */
    float kd;              /**< 微分增益（>= 0） */
    float sample_time;     /**< 采样周期（秒，> 0 且 < 1000） */
    float out_min;         /**< 输出下限 */
    float out_max;         /**< 输出上限（必须 > out_min） */
    float int_min;         /**< 积分限幅下限 */
    float int_max;         /**< 积分限幅上限（必须 > int_min） */
} FB_PID_Config_t;

/**
 * @brief PID 控制器运行时状态
 *
 * 内部状态变量，用户不应直接修改。
 */
typedef struct {
    float integral;           /**< 积分累加值 */
    float prev_measurement;   /**< 上次测量值（用于微分计算） */
    float prev_output;        /**< 上次输出值 */
    bool manual_mode;         /**< 手动模式标志（true=手动，false=自动） */
    bool first_run;           /**< 首次运行标志 */
    FB_Status_t status;       /**< 功能块状态码 */
} FB_PID_State_t;

/**
 * @brief PID 控制器功能块实例
 *
 * 用户需要为每个控制回路定义一个实例。
 */
typedef struct {
    FB_PID_Config_t config;   /**< 配置参数 */
    FB_PID_State_t state;     /**< 运行时状态 */
} FB_PID_t;

/**
 * @brief 初始化 PID 控制器
 *
 * 必须在第一次调用 Execute 前调用。
 * 验证配置参数的有效性并重置内部状态。
 *
 * @param fb PID 功能块实例指针
 * @param config 配置参数指针
 * @return int 返回码：0=成功，-1=配置无效
 *
 * 配置验证规则：
 * - sample_time > 0 && < MAX_SAMPLE_TIME (1000s)
 * - out_max > out_min
 * - int_max > int_min
 * - kp, ki, kd >= 0（允许全零配置）
 *
 * @code
 * FB_PID_t pid;
 * FB_PID_Config_t config = {...};
 * if (FB_PID_Init(&pid, &config) != 0) {
 *     // 处理配置错误
 * }
 * @endcode
 */
int FB_PID_Init(FB_PID_t* fb, const FB_PID_Config_t* config);

/**
 * @brief 执行 PID 控制算法
 *
 * 应在固定采样周期内调用，周期与 sample_time 一致。
 *
 * 算法说明：
 * 1. 检测输入有效性（NaN/Inf）
 * 2. 计算比例项：P = Kp * error
 * 3. 计算积分项：I += Ki * error * Ts（条件积分法）
 * 4. 计算微分项：D = -Kd * d(PV)/dt（微分项先行）
 * 5. 输出限幅：out = clamp(P + I + D, out_min, out_max)
 * 6. 更新状态和状态码
 *
 * @param fb PID 功能块实例指针
 * @param setpoint 设定值（SP）
 * @param measurement 测量值（PV，过程变量）
 * @return float 控制输出（MV，操纵变量）
 *
 * @note 首次调用时，使用测量值作为初始输出，避免启动冲击
 * @note 如果检测到 NaN/Inf，返回 0.0 并设置错误状态码
 * @note 手动模式下，返回上次输出值，不执行控制算法
 *
 * @code
 * float output = FB_PID_Execute(&pid, 50.0f, 45.0f);
 * if (pid.state.status == FB_STATUS_ERROR_NAN) {
 *     // 处理异常输入
 * }
 * @endcode
 */
float FB_PID_Execute(FB_PID_t* fb, float setpoint, float measurement);

/**
 * @brief 切换到手动模式
 *
 * 在手动模式下，PID 算法不执行，输出保持切换前的值。
 * 积分器跟踪当前输出，确保切回自动模式时无扰动。
 *
 * @param fb PID 功能块实例指针
 * @param manual_output 手动模式输出值
 *
 * @note 手动输出会被限制在 [out_min, out_max] 范围内
 * @note 积分器会根据手动输出反推，以实现无扰切换
 *
 * @code
 * FB_PID_SetManual(&pid, 50.0f);  // 手动输出 50.0
 * @endcode
 */
void FB_PID_SetManual(FB_PID_t* fb, float manual_output);

/**
 * @brief 切换到自动模式
 *
 * 从手动模式切换回自动模式，无扰动切换。
 *
 * @param fb PID 功能块实例指针
 *
 * @note 由于手动模式时积分器已跟踪输出，切换时不会产生跳变
 *
 * @code
 * FB_PID_SetAuto(&pid);  // 恢复自动控制
 * @endcode
 */
void FB_PID_SetAuto(FB_PID_t* fb);

/**
 * @brief 获取当前状态码
 *
 * @param fb PID 功能块实例指针
 * @return FB_Status_t 当前状态码
 *
 * @code
 * FB_Status_t status = FB_PID_GetStatus(&pid);
 * @endcode
 */
static inline FB_Status_t FB_PID_GetStatus(const FB_PID_t* fb) {
    return fb->state.status;
}

/**
 * @brief 检查是否处于手动模式
 *
 * @param fb PID 功能块实例指针
 * @return bool true=手动模式，false=自动模式
 */
static inline bool FB_PID_IsManual(const FB_PID_t* fb) {
    return fb->state.manual_mode;
}

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_PID_H */
