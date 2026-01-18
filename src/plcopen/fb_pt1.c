/**
 * @file fb_pt1.c
 * @brief PLCopen 一阶惯性滤波器实现
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 */

#include "plcopen/fb_pt1.h"
#include <string.h>

int FB_PT1_Init(FB_PT1_t* fb, const FB_PT1_Config_t* config) {
    if (fb == NULL || config == NULL) {
        return -1;
    }

    if (config->time_constant < MIN_VALID_VALUE) {
        return -1;
    }

    if (config->sample_time <= 0.0f || config->sample_time >= MAX_SAMPLE_TIME) {
        return -1;
    }

    memcpy(&fb->config, config, sizeof(FB_PT1_Config_t));

    fb->state.output = 0.0f;
    fb->state.first_run = true;
    fb->state.status = FB_STATUS_OK;

    return 0;
}

float FB_PT1_Execute(FB_PT1_t* fb, float input) {
    if (check_nan(input)) {
        fb->state.status = FB_STATUS_ERROR_NAN;
        return 0.0f;
    }

    if (check_inf(input)) {
        fb->state.status = FB_STATUS_ERROR_INF;
        return 0.0f;
    }

    /* 首次运行：输出 = 输入，无跳变启动 */
    if (fb->state.first_run) {
        fb->state.output = input;
        fb->state.first_run = false;
        fb->state.status = FB_STATUS_OK;
        return input;
    }

    /* 前向欧拉离散化：alpha = Ts / (τ + Ts) */
    float alpha = fb->config.sample_time /
                  (fb->config.time_constant + fb->config.sample_time);

    /* 更新公式：y[k] = y[k-1] + alpha * (u[k] - y[k-1]) */
    fb->state.output += alpha * (input - fb->state.output);

    fb->state.status = FB_STATUS_OK;
    return fb->state.output;
}
