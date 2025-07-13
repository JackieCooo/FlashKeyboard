/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * Description: Provides V150 HAL gpio \n
 *
 * History: \n
 * 2022-11-26, Create file. \n
 */

#include <stdint.h>
#include "interrupt/osal_interrupt.h"
#include "common_def.h"
#include "gpio_porting.h"
#include "tcxo.h"
#include "hal_gpio_v150.h"

#pragma weak hal_gpio_init = hal_gpio_v150_init
void hal_gpio_v150_init(void)
{
    uint32_t channel, group;

    hal_gpio_regs_init();
    hal_gpio_v150_callback_list_clean();
    for (channel = GPIO_CHANNEL_0; channel < GPIO_CHANNEL_MAX_NUM; channel++) {
        for (group = 0; group < hal_gpio_v150_group_num_get(channel); group++) {
            hal_gpio_v150_intr_rebase(channel, group);
        }
        gpio_porting_channel_context_clean(channel, hal_gpio_v150_group_num_get(channel));
        hal_gpio_v150_register_irq(hal_gpio_v150_irq_num_get(channel));
    }
}

#pragma weak hal_gpio_deinit = hal_gpio_v150_deinit
void hal_gpio_v150_deinit(void)
{
    uint32_t channel, group;

    for (channel = GPIO_CHANNEL_0; channel < GPIO_CHANNEL_MAX_NUM; channel++) {
        hal_gpio_v150_unregister_irq(hal_gpio_v150_irq_num_get(channel));
        gpio_porting_channel_context_clean(channel, hal_gpio_v150_group_num_get(channel));
        for (group = 0; group < hal_gpio_v150_group_num_get(channel); group++) {
            hal_gpio_v150_intr_rebase(channel, group);
        }
    }
    hal_gpio_v150_callback_list_clean();
}

#pragma weak hal_gpio_setdir = hal_gpio_v150_setdir
errcode_t hal_gpio_v150_setdir(pin_t pin, gpio_direction_t dir)
{
    if (dir != GPIO_DIRECTION_INPUT && dir != GPIO_DIRECTION_OUTPUT) {
        return ERRCODE_INVALID_PARAM;
    }

    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    if (dir == GPIO_DIRECTION_INPUT) {
        // 输入模式默认使能去毛刺
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
        hal_gpio_gpio_int_debounce_set_set_bit(channel, group, group_pin);
        hal_gpio_gpio_data_oen_set_set_bit(channel, group, group_pin);
#else
        hal_gpio_gpio_sw_oen_set_bit(channel, group, group_pin, HAL_GPIO_DIRECTION_INPUT);
#endif
    } else {
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
        hal_gpio_gpio_int_debounce_clr_set_bit(channel, group, group_pin);
        hal_gpio_gpio_data_oen_clr_set_bit(channel, group, group_pin);
#else
        hal_gpio_gpio_sw_oen_set_bit(channel, group, group_pin, HAL_GPIO_DIRECTION_OUTPUT);
#endif
    }

    return ERRCODE_SUCC;
}

#pragma weak hal_gpio_getdir = hal_gpio_v150_getdir
gpio_direction_t hal_gpio_v150_getdir(pin_t pin)
{
    errcode_t ret;
    gpio_direction_t direction;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
    direction = hal_gpio_gpio_sw_oen_get_bit(channel, group, group_pin);
    if (direction == (gpio_direction_t)HAL_GPIO_DIRECTION_OUTPUT) {
        direction = GPIO_DIRECTION_OUTPUT;
    } else {
        direction = GPIO_DIRECTION_INPUT;
    }

    return direction;
}

STATIC void hal_gpio_v150_set_output(uint32_t channel, uint32_t group, uint32_t group_pin, gpio_level_t level)
{
    if (level == GPIO_LEVEL_LOW) {
        hal_gpio_gpio_data_clr_set_bit(channel, group, group_pin, HAL_GPIO_LEVEL_HIGH);
    } else {
        hal_gpio_gpio_data_set_set_bit(channel, group, group_pin, HAL_GPIO_LEVEL_HIGH);
    }
}

#pragma weak hal_gpio_output = hal_gpio_v150_output
errcode_t hal_gpio_v150_output(pin_t pin, gpio_level_t level)
{
    if (level != GPIO_LEVEL_LOW && level != GPIO_LEVEL_HIGH) {
        return ERRCODE_INVALID_PARAM;
    }

    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    hal_gpio_v150_set_output(channel, group, group_pin, level);
    return ERRCODE_SUCC;
}

#pragma weak hal_gpio_get_outval = hal_gpio_v150_get_outval
gpio_level_t hal_gpio_v150_get_outval(pin_t pin)
{
    gpio_level_t level;
    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
    level = (gpio_level_t)hal_gpio_gpio_sw_out_get_bit(channel, group, group_pin);
    return level;
}

#pragma weak hal_gpio_get_inval = hal_gpio_v150_get_inval
gpio_level_t hal_gpio_v150_get_inval(pin_t pin)
{
    errcode_t ret;
    uint32_t channel, group, group_pin, hal_level;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return GPIO_LEVEL_LOW;
    }

    hal_level = hal_gpio_gpio_sw_out_get_bit(channel, group, group_pin);
    return (hal_level == HAL_GPIO_LEVEL_HIGH) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
}

#pragma weak hal_gpio_unregister = hal_gpio_v150_unregister
errcode_t hal_gpio_v150_unregister(pin_t pin)
{
    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    // 屏蔽中断（多核子系统同时屏蔽IP中断和SOC中断绑定）
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
    hal_gpio_gpio_int_mask_set_set_bit(channel, group, group_pin);
    gpio_port_subsystem_int_mask_pin(pin);
    gpio_port_subsystem_syswkup_mask_pin(pin);
#else
    hal_gpio_gpio_int_mask_set_bit(channel, group, group_pin, HAL_GPIO_INTR_MASK);
#endif
    hal_gpio_gpio_int_eoi_set_bit(channel, group, group_pin, HAL_GPIO_INTR_CLEAR);

    // 去注册中断回调
    hal_gpio_v150_unregister_cb(channel, group, group_pin);
    return ERRCODE_SUCC;
}

STATIC errcode_t hal_gpio_v150_ctrl_toggle(pin_t pin, hal_gpio_ctrl_id_t id)
{
    unused(id);

    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    if (hal_gpio_gpio_sw_oen_get_bit(channel, group, group_pin) != HAL_GPIO_DIRECTION_OUTPUT) {
        return ERRCODE_GPIO_STATE_MISMATCH;
    }

    uint32_t level = hal_gpio_gpio_sw_out_get_bit(channel, group, group_pin);
    hal_gpio_v150_set_output(channel, group, group_pin, HAL_GPIO_LEVEL_HIGH - level);
    return ERRCODE_SUCC;
}

STATIC errcode_t hal_gpio_v150_ctrl_enable_interrupt(pin_t pin, hal_gpio_ctrl_id_t id)
{
    unused(id);

    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    if (hal_gpio_gpio_get_int_en(channel, group) == 0) {
        osal_irq_enable(hal_gpio_v150_irq_num_get(channel));
    }
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
    hal_gpio_gpio_int_en_set_set_bit(channel, group, group_pin);
#else
    hal_gpio_gpio_int_en_set_bit(channel, group, group_pin, HAL_GPIO_INTR_ENABLE);
#endif

    return ERRCODE_SUCC;
}

STATIC errcode_t hal_gpio_v150_ctrl_disable_interrupt(pin_t pin, hal_gpio_ctrl_id_t id)
{
    unused(id);

    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
    hal_gpio_gpio_int_en_clr_set_bit(channel, group, group_pin);
#else
    hal_gpio_gpio_int_en_set_bit(channel, group, group_pin, HAL_GPIO_INTR_DISABLE);
#endif
    if (hal_gpio_gpio_get_int_en(channel, group) == 0) {
        osal_irq_disable(hal_gpio_v150_irq_num_get(channel));
    }

    return ERRCODE_SUCC;
}

STATIC errcode_t hal_gpio_v150_ctrl_clear_interrupt(pin_t pin, hal_gpio_ctrl_id_t id)
{
    unused(id);

    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    hal_gpio_gpio_int_eoi_set_bit(channel, group, group_pin, HAL_GPIO_INTR_CLEAR);
    osal_irq_clear(hal_gpio_v150_irq_num_get(channel));

    return ERRCODE_SUCC;
}

#pragma weak hal_gpio_register = hal_gpio_v150_register
errcode_t hal_gpio_v150_register(pin_t pin, uint32_t trigger, gpio_callback_t callback, bool need_callback)
{
    if ((need_callback == true && callback == NULL) || (need_callback == false && callback != NULL)) {
        return ERRCODE_INVALID_PARAM;
    }
    errcode_t ret;
    uint32_t channel, group, group_pin;
    ret = hal_gpio_v150_pin_info_get(pin, &channel, &group, &group_pin);
    if (ret != ERRCODE_SUCC) { return ret; }
    if (need_callback) {
        // 注册中断回调
        ret = hal_gpio_v150_register_cb(channel, group, group_pin, callback);
        if (ret != ERRCODE_SUCC) {
            return ret;
        }
    }
    // 屏蔽中断
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
    hal_gpio_gpio_int_mask_set_set_bit(channel, group, group_pin);
#else
    hal_gpio_gpio_int_mask_set_bit(channel, group, group_pin, HAL_GPIO_INTR_MASK);
#endif
    hal_gpio_gpio_int_eoi_set_bit(channel, group, group_pin, HAL_GPIO_INTR_CLEAR);
    // 配置中断极性
    switch (trigger) {
        case GPIO_INTERRUPT_LOW:
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
            hal_gpio_gpio_int_type_clr_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_polarity_clr_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_dedge_clr_set_bit(channel, group, group_pin);
#else
            hal_gpio_gpio_int_type_set_bit(channel, group, group_pin, GPIO_LVL_LEVEL_SENSITIVE);
            hal_gpio_v150_gpio_int_polarity_set_bit(channel, group, group_pin, GPIO_ACTIVE_LOW);
            hal_gpio_gpio_int_dedge_set_bit(channel, group, group_pin, GPIO_DEDGE_DISABLED);
#endif
            break;
        case GPIO_INTERRUPT_HIGH:
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
            hal_gpio_gpio_int_type_clr_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_polarity_set_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_dedge_clr_set_bit(channel, group, group_pin);
#else
            hal_gpio_gpio_int_type_set_bit(channel, group, group_pin, GPIO_LVL_LEVEL_SENSITIVE);
            hal_gpio_v150_gpio_int_polarity_set_bit(channel, group, group_pin, GPIO_ACTIVE_HIGH);
            hal_gpio_gpio_int_dedge_set_bit(channel, group, group_pin, GPIO_DEDGE_DISABLED);
#endif
            break;
        case GPIO_INTERRUPT_FALLING_EDGE:
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
            hal_gpio_gpio_int_type_set_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_polarity_clr_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_dedge_clr_set_bit(channel, group, group_pin);
#else
            hal_gpio_gpio_int_type_set_bit(channel, group, group_pin, GPIO_LVL_EDGE_SENSITIVE);
            hal_gpio_v150_gpio_int_polarity_set_bit(channel, group, group_pin, GPIO_ACTIVE_LOW);
            hal_gpio_gpio_int_dedge_set_bit(channel, group, group_pin, GPIO_DEDGE_DISABLED);
#endif
            break;
        case GPIO_INTERRUPT_RISING_EDGE:
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
            hal_gpio_gpio_int_type_set_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_polarity_set_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_dedge_clr_set_bit(channel, group, group_pin);
#else
            hal_gpio_gpio_int_type_set_bit(channel, group, group_pin, GPIO_LVL_EDGE_SENSITIVE);
            hal_gpio_v150_gpio_int_polarity_set_bit(channel, group, group_pin, GPIO_ACTIVE_HIGH);
            hal_gpio_gpio_int_dedge_set_bit(channel, group, group_pin, GPIO_DEDGE_DISABLED);
#endif
            break;
        case GPIO_INTERRUPT_DEDGE:
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
            hal_gpio_gpio_int_type_set_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_polarity_clr_set_bit(channel, group, group_pin);
            hal_gpio_gpio_int_dedge_set_set_bit(channel, group, group_pin);
#else
            hal_gpio_gpio_int_type_set_bit(channel, group, group_pin, GPIO_LVL_EDGE_SENSITIVE);
            hal_gpio_v150_gpio_int_polarity_set_bit(channel, group, group_pin, GPIO_ACTIVE_LOW);
            hal_gpio_gpio_int_dedge_set_bit(channel, group, group_pin, GPIO_DEDGE_ENABLED);
#endif
            break;
        default:
            hal_gpio_v150_unregister_cb(channel, group, group_pin);
            return ERRCODE_INVALID_PARAM;
    }
    // 去屏蔽中断(多核子系统同时去屏蔽IP中断及SOC中断绑定)
#if defined(CONFIG_GPIO_SUPPORT_MULTISYSTEM)
    hal_gpio_gpio_int_mask_clr_set_bit(channel, group, group_pin);
    gpio_port_subsystem_int_unmask_pin(pin);
    gpio_port_subsystem_syswkup_unmask_pin(pin);
#else
    hal_gpio_gpio_int_debounce_set_bit(channel, group, group_pin, HAL_GPIO_DEBOUNCE_DISABLED);
    hal_gpio_gpio_int_mask_set_bit(channel, group, group_pin, HAL_GPIO_INTR_UNMASK);
#endif
    // 继承原逻辑, 注册回调后默认使能中断
    uapi_tcxo_delay_ms(1);
    hal_gpio_v150_ctrl_enable_interrupt(pin, GPIO_CTRL_ENABLE_INTERRUPT);
    hal_gpio_gpio_int_debounce_set_bit(channel, group, group_pin, HAL_GPIO_DEBOUNCE_ENABLED);
    return ERRCODE_SUCC;
}

#if defined(CONFIG_GPIO_SUPPORT_LPM)
STATIC gpio_group_regs_t g_pm_gpio_regs[GPIO_CHANNEL_MAX_NUM] = { 0 };
STATIC errcode_t hal_gpio_v150_ctrl_suspend(pin_t pin, hal_gpio_ctrl_id_t id)
{
    unused(pin);
    unused(id);
    for (uint32_t i = 0; i < GPIO_CHANNEL_MAX_NUM; i++) {
#ifdef CONFIG_GPIO_SUPPORT_MULTISYSTEM
        if (memcpy_s(&g_pm_gpio_regs[i], sizeof(gpio_group_regs_t),
            (void *)hal_gpio_base_addrs_get(i), sizeof(gpio_group_regs_t)) != EOK) {
#else
        if (memcpy_s(&g_pm_gpio_regs[i], sizeof(gpio_group_regs_t) - RESERVED_MAX_NUM,
            (void *)hal_gpio_base_addrs_get(i), sizeof(gpio_group_regs_t) - RESERVED_MAX_NUM) != EOK) {
#endif
            return ERRCODE_MEMCPY;
        }
    }
    return ERRCODE_SUCC;
}

STATIC errcode_t hal_gpio_v150_ctrl_resume(pin_t pin, hal_gpio_ctrl_id_t id)
{
    unused(pin);
    unused(id);
    for (uint32_t i = 0; i < GPIO_CHANNEL_MAX_NUM; i++) {
#ifdef CONFIG_GPIO_SUPPORT_MULTISYSTEM
        if (memcpy_s((void *)hal_gpio_base_addrs_get(i), sizeof(gpio_group_regs_t),
            &g_pm_gpio_regs[i], sizeof(gpio_group_regs_t)) != EOK) {
#else
        if (memcpy_s((void *)hal_gpio_base_addrs_get(i), sizeof(gpio_group_regs_t) - RESERVED_MAX_NUM,
            &g_pm_gpio_regs[i], sizeof(gpio_group_regs_t) - RESERVED_MAX_NUM) != EOK) {
#endif
            return ERRCODE_MEMCPY;
        }
    }
    return ERRCODE_SUCC;
}
#endif

STATIC hal_gpio_ctrl_t g_hal_gpio_ctrl_func_array[GPIO_CTRL_MAX] = {
    hal_gpio_v150_ctrl_toggle,                        /* uapi_gpio_toggle */
    hal_gpio_v150_ctrl_enable_interrupt,              /* uapi_gpio_enable_interrupt */
    hal_gpio_v150_ctrl_disable_interrupt,             /* uapi_gpio_disable_interrupt */
    hal_gpio_v150_ctrl_clear_interrupt,               /* uapi_gpio_clear_interrupt */
#if defined(CONFIG_GPIO_SUPPORT_LPM)
    hal_gpio_v150_ctrl_suspend,                       /* GPIO_CTRL_SUSPEND */
    hal_gpio_v150_ctrl_resume                         /* GPIO_CTRL_RESUME */
#endif
};

#pragma weak hal_gpio_ctrl = hal_gpio_v150_ctrl
errcode_t hal_gpio_v150_ctrl(pin_t pin, hal_gpio_ctrl_id_t id)
{
    return g_hal_gpio_ctrl_func_array[id](pin, id);
}