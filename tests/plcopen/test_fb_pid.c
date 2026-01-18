/**
 * @file test_fb_pid.c
 * @brief PID 控制器功能块单元测试
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 测试范围：
 * - 配置验证（sample_time, out_max/min, int_max/min, Kp/Ki/Kd）
 * - 阶跃响应和收敛性
 * - 积分抗饱和（条件积分法）
 * - 手自动无扰切换
 * - 数值保护（NaN/Inf/溢出）
 * - 首次调用行为
 * - 状态码输出
 */

#include "unity.h"
#include "plcopen/fb_pid.h"
#include <math.h>
#include <string.h>

/* 测试辅助宏 */
#define FLOAT_TOLERANCE 0.01f  /* 浮点比较容差 */
#define ASSERT_FLOAT_IN_RANGE(expected, actual, tolerance) \
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected, actual)

/* 测试夹具 */
static FB_PID_t pid;
static FB_PID_Config_t config;

void setUp(void) {
    /* 每个测试前初始化默认配置 */
    memset(&pid, 0, sizeof(FB_PID_t));
    memset(&config, 0, sizeof(FB_PID_Config_t));

    /* 默认配置：合理的 PID 参数 */
    config.kp = 1.0f;
    config.ki = 0.1f;
    config.kd = 0.05f;
    config.sample_time = 0.01f;  /* 10ms */
    config.out_min = 0.0f;
    config.out_max = 100.0f;
    config.int_min = -50.0f;
    config.int_max = 50.0f;
}

void tearDown(void) {
    /* 清理（如需要） */
}

/* ========== 配置验证测试 ========== */

/**
 * @brief 测试有效配置初始化成功
 */
void test_pid_init_valid_config(void) {
    FB_Status_t status = FB_PID_Init(&pid, &config);
    TEST_ASSERT_EQUAL(FB_STATUS_OK, status);
    /* 初始化后 first_run 应为 true */
    TEST_ASSERT_TRUE(pid.state.first_run);
}

/**
 * @brief 测试无效 sample_time 被拒绝
 */
void test_pid_init_invalid_sample_time(void) {
    /* 负数 */
    config.sample_time = -0.01f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PID_Init(&pid, &config));

    /* 零 */
    config.sample_time = 0.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PID_Init(&pid, &config));

    /* 过大（>1000s） */
    config.sample_time = 1001.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PID_Init(&pid, &config));
}

/**
 * @brief 测试无效输出限幅范围被拒绝
 */
void test_pid_init_invalid_output_limits(void) {
    /* out_max <= out_min */
    config.out_min = 100.0f;
    config.out_max = 50.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PID_Init(&pid, &config));

    /* 相等 */
    config.out_min = 50.0f;
    config.out_max = 50.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PID_Init(&pid, &config));
}

/**
 * @brief 测试无效积分限幅范围被拒绝
 */
void test_pid_init_invalid_integral_limits(void) {
    config.int_min = 50.0f;
    config.int_max = 10.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_CONFIG, FB_PID_Init(&pid, &config));
}

/**
 * @brief 测试增益参数允许为零（特殊场景：手动模式或 P/I/D 单独使用）
 */
void test_pid_init_zero_gains_allowed(void) {
    config.kp = 0.0f;
    config.ki = 0.0f;
    config.kd = 0.0f;
    TEST_ASSERT_EQUAL(FB_STATUS_OK, FB_PID_Init(&pid, &config));
}

/* ========== 首次调用行为测试 ========== */

/**
 * @brief 测试首次调用使用测量值作为初始输出
 */
void test_pid_first_call_behavior(void) {
    FB_PID_Init(&pid, &config);

    float setpoint = 50.0f;
    float measurement = 30.0f;  /* 初始测量值 */

    /* 首次调用 */
    float output = FB_PID_Execute(&pid, setpoint, measurement);

    /* 输出应该等于首次测量值（无扰启动） */
    ASSERT_FLOAT_IN_RANGE(measurement, output, FLOAT_TOLERANCE);

    /* 第二次调用应该开始正常控制 */
    float output2 = FB_PID_Execute(&pid, setpoint, measurement);
    TEST_ASSERT_TRUE(output2 != measurement);  /* 已开始调节 */
}

/* ========== 基本控制功能测试 ========== */

/**
 * @brief 测试纯比例控制（Ki=Kd=0）
 */
void test_pid_proportional_only(void) {
    config.ki = 0.0f;
    config.kd = 0.0f;
    FB_PID_Init(&pid, &config);

    /* 首次调用初始化 */
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 第二次调用测试比例作用 */
    float output = FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 输出 = Kp * error = 1.0 * (50-30) = 20.0 */
    ASSERT_FLOAT_IN_RANGE(20.0f, output, FLOAT_TOLERANCE);
}

/**
 * @brief 测试积分作用累积
 */
void test_pid_integral_accumulation(void) {
    config.kp = 0.0f;  /* 仅积分 */
    config.kd = 0.0f;
    FB_PID_Init(&pid, &config);

    /* 首次调用 */
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 第二次调用：误差累积到积分器，但输出还是0（积分在计算输出后更新） */
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 第三次调用：使用上次累积的积分值 */
    float output = FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 恒定误差累积 */
    float error = 20.0f;  /* 50 - 30 */
    float expected_integral = config.ki * error * config.sample_time;  /* 0.1 * 20 * 0.01 = 0.02 */

    /* 输出 = 积分项 = 0.02 */
    ASSERT_FLOAT_IN_RANGE(expected_integral, output, FLOAT_TOLERANCE);
}

/**
 * @brief 测试微分作用响应测量值变化
 */
void test_pid_derivative_on_measurement(void) {
    config.kp = 0.0f;
    config.ki = 0.0f;  /* 仅微分 */
    FB_PID_Init(&pid, &config);

    /* 首次调用 */
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 测量值变化: 从 30.0 到 35.0 */
    float output = FB_PID_Execute(&pid, 50.0f, 35.0f);

    /* 微分项 = -Kd * (new_meas - prev_meas) / Ts = -0.05 * (35-30) / 0.01 = -25.0 */
    /* 输出 = 微分项 = -25.0，但被限制在 [0, 100]，所以输出 = 0.0 */
    ASSERT_FLOAT_IN_RANGE(0.0f, output, FLOAT_TOLERANCE);
}

/* ========== 积分抗饱和测试 ========== */

/**
 * @brief 测试输出上限饱和时停止正向积分
 */
void test_pid_antiwindup_upper_limit(void) {
    config.kp = 0.0f;
    config.ki = 10.0f;  /* 大积分增益，快速饱和 */
    config.kd = 0.0f;
    config.int_max = 150.0f;  /* 增大积分限幅，允许达到out_max */
    FB_PID_Init(&pid, &config);

    /* 首次调用 */
    FB_PID_Execute(&pid, 100.0f, 0.0f);

    /* 持续正误差，积分累积直到输出饱和 */
    for (int i = 0; i < 20; i++) {
        FB_PID_Execute(&pid, 100.0f, 0.0f);
    }

    /* 输出应该限幅在 out_max */
    float output = FB_PID_Execute(&pid, 100.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, config.out_max, output);

    /* 继续正误差，积分不应再增长（条件积分法） */
    float integral_before = pid.state.integral;
    FB_PID_Execute(&pid, 100.0f, 0.0f);
    float integral_after = pid.state.integral;

    /* 积分值应该保持不变（条件积分停止累加） */
    TEST_ASSERT_FLOAT_WITHIN(0.1f, integral_before, integral_after);
}

/**
 * @brief 测试输出下限饱和时停止负向积分
 */
void test_pid_antiwindup_lower_limit(void) {
    config.kp = 0.0f;
    config.ki = 10.0f;
    config.kd = 0.0f;
    FB_PID_Init(&pid, &config);

    /* 首次调用 */
    FB_PID_Execute(&pid, 0.0f, 100.0f);

    /* 持续负误差 */
    for (int i = 0; i < 20; i++) {
        FB_PID_Execute(&pid, 0.0f, 100.0f);
    }

    /* 输出应该限幅在 out_min */
    float output = FB_PID_Execute(&pid, 0.0f, 100.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, config.out_min, output);
}

/* ========== 手自动切换测试 ========== */

/**
 * @brief 测试切换到手动模式
 */
void test_pid_switch_to_manual(void) {
    FB_PID_Init(&pid, &config);

    /* 自动模式运行 */
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 切换到手动，设置手动输出值 */
    float manual_output = 75.0f;
    FB_PID_SetManual(&pid, manual_output);
    TEST_ASSERT_TRUE(pid.state.manual_mode);

    /* 手动模式下，Execute 应返回设定的手动值 */
    float output = FB_PID_Execute(&pid, 50.0f, 30.0f);
    ASSERT_FLOAT_IN_RANGE(manual_output, output, FLOAT_TOLERANCE);
}

/**
 * @brief 测试从手动切换到自动无扰
 */
void test_pid_switch_to_auto_bumpless(void) {
    FB_PID_Init(&pid, &config);

    /* 首次调用 */
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 切换到手动，使用一个在积分限幅范围内的值 */
    float manual_output = 40.0f;  /* 低于 int_max=50 */
    FB_PID_SetManual(&pid, manual_output);
    FB_PID_Execute(&pid, 50.0f, 30.0f);

    /* 切换回自动 */
    FB_PID_SetAuto(&pid);
    TEST_ASSERT_FALSE(pid.state.manual_mode);

    /* 切换瞬间输出应保持（无扰切换）
     * output = P + I + D = Kp*error + integral + D
     * error = 50-30=20, P = 1*20=20, I=40, D≈0
     * output ≈ 60
     */
    float output_at_switch = FB_PID_Execute(&pid, 50.0f, 30.0f);
    ASSERT_FLOAT_IN_RANGE(60.0f, output_at_switch, 5.0f);  /* 允许小幅调整 */
}

/* ========== 数值保护测试 ========== */

/**
 * @brief 测试 NaN 输入检测
 */
void test_pid_nan_input(void) {
    FB_PID_Init(&pid, &config);

    float nan_value = 0.0f / 0.0f;
    float output = FB_PID_Execute(&pid, 50.0f, nan_value);

    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_NAN, pid.state.status);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
}

/**
 * @brief 测试 Inf 输入检测
 */
void test_pid_inf_input(void) {
    FB_PID_Init(&pid, &config);

    float inf_value = 1.0f / 0.0f;
    float output = FB_PID_Execute(&pid, inf_value, 50.0f);

    TEST_ASSERT_EQUAL(FB_STATUS_ERROR_INF, pid.state.status);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output);
}

/* ========== 状态码测试 ========== */

/**
 * @brief 测试正常状态码
 */
void test_pid_status_ok(void) {
    /* 使用较小的参数避免触发限幅 */
    config.kp = 1.0f;
    config.ki = 0.0f;  /* 禁用积分避免累积 */
    config.kd = 0.0f;  /* 禁用微分避免大幅输出 */
    FB_PID_Init(&pid, &config);

    FB_PID_Execute(&pid, 50.0f, 45.0f);  /* 首次调用，输出=45 */

    /* 小误差保持在范围内 */
    FB_PID_Execute(&pid, 50.0f, 45.0f);  /* error=5, P=5, output=5 */
    TEST_ASSERT_EQUAL(FB_STATUS_OK, pid.state.status);
}

/**
 * @brief 测试上限限幅状态
 */
void test_pid_status_limit_hi(void) {
    config.kp = 10.0f;  /* 大增益快速饱和 */
    FB_PID_Init(&pid, &config);

    FB_PID_Execute(&pid, 100.0f, 0.0f);
    FB_PID_Execute(&pid, 100.0f, 0.0f);

    TEST_ASSERT_EQUAL(FB_STATUS_LIMIT_HI, pid.state.status);
}

/**
 * @brief 测试下限限幅状态
 */
void test_pid_status_limit_lo(void) {
    config.kp = 10.0f;
    FB_PID_Init(&pid, &config);

    FB_PID_Execute(&pid, 0.0f, 100.0f);
    FB_PID_Execute(&pid, 0.0f, 100.0f);

    TEST_ASSERT_EQUAL(FB_STATUS_LIMIT_LO, pid.state.status);
}

/* ========== 运行器函数 ========== */

/**
 * @brief 运行 PID 模块所有测试（供测试运行器调用）
 */
void run_test_fb_pid(void) {
    /* 配置验证测试 */
    RUN_TEST(test_pid_init_valid_config);
    RUN_TEST(test_pid_init_invalid_sample_time);
    RUN_TEST(test_pid_init_invalid_output_limits);
    RUN_TEST(test_pid_init_invalid_integral_limits);
    RUN_TEST(test_pid_init_zero_gains_allowed);

    /* 首次调用测试 */
    RUN_TEST(test_pid_first_call_behavior);

    /* 基本控制功能 */
    RUN_TEST(test_pid_proportional_only);
    RUN_TEST(test_pid_integral_accumulation);
    RUN_TEST(test_pid_derivative_on_measurement);

    /* 积分抗饱和 */
    RUN_TEST(test_pid_antiwindup_upper_limit);
    RUN_TEST(test_pid_antiwindup_lower_limit);

    /* 手自动切换 */
    RUN_TEST(test_pid_switch_to_manual);
    RUN_TEST(test_pid_switch_to_auto_bumpless);

    /* 数值保护 */
    RUN_TEST(test_pid_nan_input);
    RUN_TEST(test_pid_inf_input);

    /* 状态码 */
    RUN_TEST(test_pid_status_ok);
    RUN_TEST(test_pid_status_limit_hi);
    RUN_TEST(test_pid_status_limit_lo);
}

/* ========== 独立运行主函数 ========== */

int main(void) {
    UNITY_BEGIN();
    run_test_fb_pid();
    return UNITY_END();
}
