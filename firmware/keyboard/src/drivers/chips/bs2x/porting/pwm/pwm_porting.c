/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides pwm port \n
 *
 * History: \n
 * 2022-09-16， Create file. \n
 */

#include "soc_osal.h"
#include "common_def.h"
#include "chip_io.h"
#include "arch_port.h"
#include "hal_pwm_v151.h"
#include "chip_core_irq.h"
#include "lpm_dev_ops.h"
#include "platform_core.h"
#include "pm_clock.h"
#include "pwm_porting.h"

#define M_CTL_SOFT_RST_OFFSET   0x54
#define PWM_FRE_DIV_OFFSET      0x78
#define PWM_DIV_LOAD_EN_BIT     4
#define PWM_DIV_EN_BIT          5
#define PWM_SOFT_RST_BIT        2

uintptr_t g_pwm_base_addr =  (uintptr_t)PWM_0_BASE;

static uint16_t g_pwm_irq_created = 0;

void pwm_port_clock_enable(bool on)
{
    if (on) {
        reg16_setbit(M_CTL_RB_BASE + M_CTL_SOFT_RST_OFFSET, PWM_SOFT_RST_BIT);
        reg16_setbit(M_CTL_RB_BASE + PWM_FRE_DIV_OFFSET, PWM_DIV_LOAD_EN_BIT);
        reg16_setbit(M_CTL_RB_BASE + PWM_FRE_DIV_OFFSET, PWM_DIV_EN_BIT);
    } else {
        reg16_clrbit(M_CTL_RB_BASE + M_CTL_SOFT_RST_OFFSET, PWM_SOFT_RST_BIT);
    }
}

uintptr_t pwm_porting_base_addr_get(void)
{
    return g_pwm_base_addr;
}

void pwm_port_register_hal_funcs(void)
{
    hal_pwm_register_funcs(hal_pwm_v151_funcs_get());
}

void pwm_port_unregister_hal_funcs(void)
{
    hal_pwm_unregister_funcs();
}

void pwm_port_register_irq(pwm_channel_t channel)
{
    if (g_pwm_irq_created == 0) {
        osal_irq_request(PWM_0_IRQN, (osal_irq_handler)hal_pwm_v151_irq_handler, NULL, NULL, NULL);
        osal_irq_request(PWM_1_IRQN, (osal_irq_handler)hal_pwm_v151_irq_handler, NULL, NULL, NULL);
        osal_irq_set_priority(PWM_0_IRQN, irq_prio(PWM_0_IRQN));
        osal_irq_set_priority(PWM_1_IRQN, irq_prio(PWM_1_IRQN));
        osal_irq_enable(PWM_0_IRQN);
        osal_irq_enable(PWM_1_IRQN);
    }
    g_pwm_irq_created |= 1 << channel;
}

void pwm_port_unregister_irq(pwm_channel_t channel)
{
    g_pwm_irq_created &= ~(1 << channel);
    if (g_pwm_irq_created == 0) {
        osal_irq_free(PWM_0_IRQN, NULL);
        osal_irq_free(PWM_1_IRQN, NULL);
    }
}

uint32_t pwm_port_get_clock_value(pwm_channel_t channel)
{
    if (channel >= CONFIG_PWM_CHANNEL_NUM) {
        return 0;
    }
    return uapi_clock_crg_get_freq(CLOCK_CRG_ID_MCU_PERP_LS);
}

errcode_t pwm_port_param_check(const pwm_config_t *cfg)
{
    unused(cfg);
    return ERRCODE_SUCC;
}