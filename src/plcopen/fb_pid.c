/**
 * @file fb_pid.c
 * @brief PLCopen PID 控制器功能块实现
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * PID 控制算法实现说明：
 *
 * 1. 比例项（P）：
 *    P = Kp * error
 *    error = setpoint - measurement
 *
 * 2. 积分项（I）：
 *    使用条件积分法（Conditional Integration）防止饱和：
 *    - 如果输出未达到限幅，积分器正常累加
 *    - 如果输出达到上限且误差为正，停止积分累加
 *    - 如果输出达到下限且误差为负，停止积分累加
 *    这样可以避免积分饱和（Integral Windup）问题
 *
 * 3. 微分项（D）：
 *    使用微分项先行滤波（Derivative on Measurement）：
 *    D = -Kd * d(PV)/dt = -Kd * (PV[k] - PV[k-1]) / Ts
 *    相比对误差求微分，这种方法避免了设定值突变时的输出尖峰
 *
 * 4. 输出限幅：
 *    output = clamp(P + I + D, out_min, out_max)
 *
 * 5. 首次调用处理：
 *    首次调用时，使用当前测量值作为初始输出，避免启动冲击
 */

#include "plcopen/fb_pid.h"
#include <string.h>  // for memcpy

/**
 * @brief 初始化 PID 控制器
 */
FB_Status_t FB_PID_Init(FB_PID_t* fb, const FB_PID_Config_t* config) {
    /* 参数验证 */
    if (fb == NULL || config == NULL) {
        return FB_STATUS_ERROR_CONFIG;
    }

    /* 验证采样周期 */
    if (config->sample_time <= 0.0f || config->sample_time >= MAX_SAMPLE_TIME) {
        return FB_STATUS_ERROR_CONFIG;
    }

    /* 验证输出限幅范围 */
    if (config->out_max <= config->out_min) {
        return FB_STATUS_ERROR_CONFIG;
    }

    /* 验证积分限幅范围 */
    if (config->int_max <= config->int_min) {
        return FB_STATUS_ERROR_CONFIG;
    }

    /* 验证增益参数（允许零值，用于手动模式或特殊场景） */
    if (config->kp < 0.0f || config->ki < 0.0f || config->kd < 0.0f) {
        return FB_STATUS_ERROR_CONFIG;
    }

    /* 复制配置 */
    memcpy(&fb->config, config, sizeof(FB_PID_Config_t));

    /* 重置状态 */
    fb->state.integral = 0.0f;
    fb->state.prev_measurement = 0.0f;
    fb->state.prev_output = 0.0f;
    fb->state.manual_mode = false;
    fb->state.first_run = true;
    fb->state.status = FB_STATUS_OK;

    return FB_STATUS_OK;
}

/**
 * @brief 执行 PID 控制算法
 */
float FB_PID_Execute(FB_PID_t* fb, float setpoint, float measurement) {
    /* 检测输入有效性 */
    if (check_nan(setpoint) || check_nan(measurement)) {
        fb->state.status = FB_STATUS_ERROR_NAN;
        return 0.0f;
    }

    if (check_inf(setpoint) || check_inf(measurement)) {
        fb->state.status = FB_STATUS_ERROR_INF;
        return 0.0f;
    }

    /* 手动模式：返回上次输出 */
    if (fb->state.manual_mode) {
        return fb->state.prev_output;
    }

    /* 首次调用：使用测量值作为初始输出，避免启动冲击 */
    if (fb->state.first_run) {
        fb->state.prev_measurement = measurement;
        fb->state.prev_output = clamp_output(measurement,
                                              fb->config.out_min,
                                              fb->config.out_max);
        fb->state.integral = 0.0f;
        fb->state.first_run = false;
        fb->state.status = FB_STATUS_OK;
        return fb->state.prev_output;
    }

    /* 计算误差 */
    float error = setpoint - measurement;

    /* 计算比例项 */
    float p_term = fb->config.kp * error;

    /* 计算微分项（微分项先行：对测量值求微分，而不是误差） */
    float d_term = 0.0f;
    if (fb->config.kd > 0.0f) {
        float d_measurement = (measurement - fb->state.prev_measurement) / fb->config.sample_time;
        d_term = -fb->config.kd * d_measurement;
    }

    /* 计算期望输出（不含积分项） */
    float output_without_integral = p_term + d_term;

    /* 积分项限幅 */
    fb->state.integral = clamp_output(fb->state.integral,
                                       fb->config.int_min,
                                       fb->config.int_max);

    /* 计算完整输出 */
    float desired_output = output_without_integral + fb->state.integral;

    /* 输出限幅 */
    float output = clamp_output(desired_output, fb->config.out_min, fb->config.out_max);

    /* 条件积分法：仅当输出未饱和时才更新积分器 */
    bool output_saturated_hi = (desired_output > fb->config.out_max);
    bool output_saturated_lo = (desired_output < fb->config.out_min);

    /* 双向条件积分：
     * - 上限饱和时，仅当误差为负时才继续积分（减少积分值）
     * - 下限饱和时，仅当误差为正时才继续积分（增加积分值）
     */
    bool should_integrate = true;
    if (output_saturated_hi && error > 0.0f) {
        should_integrate = false;  // 上限饱和且误差为正，停止积分
    }
    if (output_saturated_lo && error < 0.0f) {
        should_integrate = false;  // 下限饱和且误差为负，停止积分
    }

    /* 更新积分器 */
    if (should_integrate && fb->config.ki > 0.0f) {
        fb->state.integral += fb->config.ki * error * fb->config.sample_time;
        /* 再次限幅，防止单次累加过大 */
        fb->state.integral = clamp_output(fb->state.integral,
                                           fb->config.int_min,
                                           fb->config.int_max);
    }

    /* 更新状态码 */
    if (output_saturated_hi) {
        fb->state.status = FB_STATUS_LIMIT_HI;
    } else if (output_saturated_lo) {
        fb->state.status = FB_STATUS_LIMIT_LO;
    } else {
        fb->state.status = FB_STATUS_OK;
    }

    /* 保存当前状态 */
    fb->state.prev_measurement = measurement;
    fb->state.prev_output = output;

    return output;
}

/**
 * @brief 切换到手动模式
 */
void FB_PID_SetManual(FB_PID_t* fb, float manual_output) {
    /* 限制手动输出 */
    manual_output = clamp_output(manual_output, fb->config.out_min, fb->config.out_max);

    /* 计算积分器跟踪值，实现无扰切换 */
    /* 假设 P 和 D 项为零（手动模式下没有控制动作） */
    /* 因此 integral ≈ manual_output */
    fb->state.integral = clamp_output(manual_output, fb->config.int_min, fb->config.int_max);

    fb->state.prev_output = manual_output;
    fb->state.manual_mode = true;
    fb->state.status = FB_STATUS_OK;
}

/**
 * @brief 切换到自动模式
 */
void FB_PID_SetAuto(FB_PID_t* fb) {
    fb->state.manual_mode = false;
    /* 积分器已在手动模式时跟踪输出，无需额外调整 */
    /* 切换时不会产生输出跳变 */
}
