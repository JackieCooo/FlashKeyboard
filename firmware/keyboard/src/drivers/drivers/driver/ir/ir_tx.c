/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: IR TX Sample Source. \n
 *
 * History: \n
 * 2023-09-10, Create file. \n
 */

#include <stdbool.h>
#include "soc_osal.h"
#include "securec.h"
#include "gpio_porting.h"
#include "gpio.h"
#include "pwm.h"
#include "pwm_porting.h"
#include "tcxo.h"
#include "ir_tx.h"

#define S_2_NS                         1000000000
#define S_2_US                         1000000
#define US_2_NS                        1000
#define BIT_LEN_8                      8
#define PARAM_2                        2
#define PWM_HIGH_TICK_PARAM_3          3
#define GPIO_DELAY_NS                  7500
#define ONE_INSTRUCTION_CYCLE_NS       156
#define PWM_TEST_CHANNEL               0
#define PWM_GROUP0                     0
#define IR_HIGH_TCXO_DELAY_US          20
#define IR_LOW_TCXO_DELAY_US           70
#define IR_PATTERN_LEN                 100

#define IR_PWM_BASE_ADDR               PWM_0_BASE
#define IR_PWM_GROUP0                  (IR_PWM_BASE_ADDR + PWM_GROUP0 * 0X10)
#define IR_PWM_CHANNEL0                (IR_PWM_BASE_ADDR + PWM_TEST_CHANNEL * 0x40)

typedef struct ir_pwm_param {
    int pwm_high_ticks;
    int pwm_low_ticks;
    int freq;
    int len;
} ir_pwm_param_t;

static ir_pwm_param_t g_ir_pwm_param;
static int16_t g_ir_pattern[IR_PATTERN_LEN] = { 0 };
static int g_cur_pattern = 0;

void uapi_ir_init(pin_t gpio)
{
    uapi_pin_set_mode(gpio, HAL_PIO_PWM0);
}

static void ir_pwm_config_preload(void)
{
    pwm_config_t cfg_repeat = {0, 0, 0, 0, false};
    cfg_repeat.offset_time = 0x0;
    cfg_repeat.cycles = g_ir_pattern[g_cur_pattern + 1] * g_ir_pwm_param.freq / S_2_US;
    if (g_ir_pattern[g_cur_pattern + 1] > 0) {
        /* 高电平 */
        cfg_repeat.high_time = g_ir_pwm_param.pwm_high_ticks;
        cfg_repeat.low_time = (g_ir_pwm_param.pwm_high_ticks + 1) * PARAM_2;
    } else {
        /* 低电平 */
        cfg_repeat.high_time = 0;
        cfg_repeat.low_time = g_ir_pwm_param.pwm_low_ticks;
        cfg_repeat.cycles = -cfg_repeat.cycles;
    }
#if defined(CONFIG_PWM_PRELOAD)
    uapi_pwm_config_preload(PWM_GROUP0, PWM_TEST_CHANNEL, &cfg_repeat);
#else
    uapi_pwm_open(PWM_TEST_CHANNEL, &cfg_repeat);
    uapi_pwm_start(PWM_TEST_CHANNEL);
#endif /* CONFIG_PWM_PRELOAD */
}

static errcode_t ir_pwm_callback(uint8_t channel)
{
    unused(channel);
    if (g_cur_pattern >= (g_ir_pwm_param.len - 1)) {
        uapi_pwm_unregister_interrupt(channel);
        uapi_pwm_close(PWM_GROUP0);
        return 0;
    }

    ir_pwm_config_preload();
    g_cur_pattern++;
    return 0;
}

errcode_t uapi_ir_transmit(int freq, int16_t *pattern, int len)
{
    if (freq <= 0 || pattern == NULL || len <= 0 || len > IR_PATTERN_LEN) {
        return ERRCODE_INVALID_PARAM;
    }
    if (memcpy_s(g_ir_pattern, len * sizeof(int16_t), pattern, len * sizeof(int16_t)) != EOK) {
        return ERRCODE_INVALID_PARAM;
    }
    g_cur_pattern = 0;
    g_ir_pwm_param.freq = freq;
    int pwm_freq = uapi_pwm_get_frequency(PWM_TEST_CHANNEL);
    g_ir_pwm_param.pwm_high_ticks = pwm_freq / freq / PWM_HIGH_TICK_PARAM_3;
    g_ir_pwm_param.pwm_low_ticks = pwm_freq / freq;
    g_ir_pwm_param.len = len;
    pwm_config_t cfg_repeat = {0, 0, 0, 0, false};
    cfg_repeat.offset_time = 0x0;
    cfg_repeat.cycles = g_ir_pattern[g_cur_pattern] * g_ir_pwm_param.freq / S_2_US;

    uapi_pwm_init();
    uint8_t channel = PWM_TEST_CHANNEL;
    uapi_pwm_set_group(PWM_GROUP0, &channel, 1);

    if (g_ir_pattern[g_cur_pattern] > 0) {
        /* 高电平 */
        cfg_repeat.high_time = g_ir_pwm_param.pwm_high_ticks;
        cfg_repeat.low_time = (g_ir_pwm_param.pwm_high_ticks + 1) * PARAM_2;
    } else {
        /* 低电平 */
        cfg_repeat.high_time = 0;
        cfg_repeat.low_time = g_ir_pwm_param.pwm_low_ticks;
        cfg_repeat.cycles = -cfg_repeat.cycles;
    }
    uapi_pwm_open(PWM_TEST_CHANNEL, &cfg_repeat);
    uapi_pwm_register_interrupt(PWM_TEST_CHANNEL, ir_pwm_callback);
    uapi_pwm_start(PWM_TEST_CHANNEL);

    return ERRCODE_SUCC;
}