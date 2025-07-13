/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * Description: Provides pinctrl port template \n
 *
 * History: \n
 * 2024-04-10， Create file. \n
 */
#include "pinctrl_porting.h"
#include "hal_pinctrl_bs2x.h"

extern hal_pio_config_t g_pio_function_config[PIN_MAX_NUMBER];

void get_pio_func_config(size_t *pin_num, hal_pio_config_t **pin_func_array)
{
    if (pin_num == NULL || pin_func_array == NULL) { return; }
    *pin_num = sizeof(g_pio_function_config) / sizeof(g_pio_function_config[0]);
    *pin_func_array = (hal_pio_config_t *)g_pio_function_config;
}

bool pin_check_mode_is_valid(pin_t pin, pin_mode_t mode)
{
    UNUSED(pin);
    if (mode > PIN_MODE_MAX) {
        return false;
    }
    return true;
}

void pin_port_register_hal_funcs(void)
{
    hal_pin_register_funcs(hal_pin_bs2x_funcs_get());
}

void pin_port_unregister_hal_funcs(void)
{
    hal_pin_unregister_funcs();
}

uint32_t pin_gpio_mode_get(void)
{
    return g_pin_is_gpio;
}