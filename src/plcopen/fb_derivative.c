/**
 * @file fb_derivative.c
 * @brief PLCopen 微分器实现
 */

#include "plcopen/fb_derivative.h"
#include <string.h>

int FB_DERIVATIVE_Init(FB_DERIVATIVE_t* fb, const FB_DERIVATIVE_Config_t* config) {
    if (fb == NULL || config == NULL) return -1;
    if (config->sample_time <= 0.0f || config->sample_time >= MAX_SAMPLE_TIME) return -1;
    if (config->filter_time_constant < 0.0f) return -1;

    memcpy(&fb->config, config, sizeof(FB_DERIVATIVE_Config_t));
    fb->state.prev_input = 0.0f;
    fb->state.filtered_output = 0.0f;
    fb->state.first_run = true;
    fb->state.status = FB_STATUS_OK;
    return 0;
}

float FB_DERIVATIVE_Execute(FB_DERIVATIVE_t* fb, float input) {
    if (check_nan_inf(input)) {
        fb->state.status = check_nan(input) ? FB_STATUS_ERROR_NAN : FB_STATUS_ERROR_INF;
        return 0.0f;
    }

    if (fb->state.first_run) {
        fb->state.prev_input = input;
        fb->state.filtered_output = 0.0f;
        fb->state.first_run = false;
        fb->state.status = FB_STATUS_OK;
        return 0.0f;
    }

    float raw_derivative = (input - fb->state.prev_input) / fb->config.sample_time;

    if (fb->config.filter_time_constant > 0.0f) {
        float alpha = fb->config.sample_time /
                      (fb->config.filter_time_constant + fb->config.sample_time);
        fb->state.filtered_output += alpha * (raw_derivative - fb->state.filtered_output);
    } else {
        fb->state.filtered_output = raw_derivative;
    }

    fb->state.prev_input = input;
    fb->state.status = FB_STATUS_OK;
    return fb->state.filtered_output;
}
