/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: imu_wakeup_port \n
 *
 * History: \n
 * 2024-03-16, Create file. \n
 */
#include "gpio.h"
#include "ulp_gpio.h"
#include "pinctrl.h"

extern void tiot_board_gpio_callback(pin_t pin, uintptr_t param);

ulp_gpio_int_wkup_cfg_t g_pm_wk_cfg[] = {
    { 0, S_MGPIO14, true, ULP_GPIO_INTERRUPT_RISING_EDGE, NULL },    // ulpgpio唤醒
};

void slp_sleep_pin_config(void)
{
    // power en
    (void)uapi_pin_set_pull(S_MGPIO13, PIN_PULL_UP);
    // power on
    (void)uapi_pin_set_pull(S_MGPIO12, PIN_PULL_UP);
    // uart tx
    (void)uapi_pin_set_pull(S_MGPIO26, PIN_PULL_UP);
    (void)uapi_gpio_set_dir(S_MGPIO26, GPIO_DIRECTION_OUTPUT);
    (void)uapi_gpio_set_val(S_MGPIO26, GPIO_LEVEL_HIGH);
    (void)uapi_pin_set_mode(S_MGPIO26, (pin_mode_t)HAL_PIO_FUNC_GPIO);
    // imu_int2
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    (void)uapi_pin_set_ie(S_MGPIO14, PIN_IE_1);
#endif
}

void ulp_wakeup_congif(ulp_gpio_irq_cb_t irq_cb)
{
    // 配置唤醒回调
    g_pm_wk_cfg[0].ulp_gpio = 0;
    g_pm_wk_cfg[0].wk_mux = S_MGPIO14;
    g_pm_wk_cfg[0].int_enable = true;
    g_pm_wk_cfg[0].trigger = ULP_GPIO_INTERRUPT_RISING_EDGE;
    g_pm_wk_cfg[0].irq_cb = irq_cb;
    ulp_gpio_int_wkup_config(g_pm_wk_cfg, sizeof(g_pm_wk_cfg) / sizeof(ulp_gpio_int_wkup_cfg_t));
}

void slp_wakeup_pin_config(void)
{
    uapi_pin_set_mode(S_MGPIO10, PIN_MODE_61); // GPIO10置为sync管脚
    // wakeup host
    (void)uapi_gpio_register_isr_func(S_MGPIO15, GPIO_INTERRUPT_RISING_EDGE, tiot_board_gpio_callback);
    (void)uapi_gpio_enable_interrupt(S_MGPIO15);
    // imu_int2
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    (void)uapi_pin_set_ie(S_MGPIO14, PIN_IE_0);
#endif
}
