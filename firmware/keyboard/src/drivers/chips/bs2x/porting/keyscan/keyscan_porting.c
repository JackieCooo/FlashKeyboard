/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * Description: Provides keyscan port template \n
 *
 * History: \n
 * 2022-09-15， Create file. \n
 */

#include "chip_io.h"
#include "pinctrl_porting.h"
#include "pinctrl.h"
#include "platform_types.h"
#include "platform_core.h"
#include "arch_port.h"
#include "soc_osal.h"
#include "oal_interface.h"
#include "chip_core_irq.h"
#include "common_def.h"
#include "timer.h"
#if defined (CONFIG_KEYSCAN_SUPPORT_LPM)
#include "ulp_gpio.h"
#include "gpio.h"
#endif
#include "pm_veto.h"
#include "pm_veto_porting.h"
#include "tcxo.h"
#include "hal_keyscan_v150.h"
#include "hal_keyscan_v150_regs_op.h"
#include "keyscan_porting.h"

#define KEYSCAN_BASE_ADDR 0x5208D000
#define M_CTL_BASE_ADDR 0x520004A0

uintptr_t g_keyscan_base_addr =  (uintptr_t)KEYSCAN_BASE_ADDR;
uintptr_t g_keyscan_m_ctl_base_addr =  (uintptr_t)M_CTL_BASE_ADDR;
#if defined (CONFIG_KEYSCAN_USE_FULL_KEYS_TYPE)
static uint8_t g_gpio_map[CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL] = {
    31, 24, 14, 23, 27, 28, 10, 11, 30, 13, 15, 16, 25, 26, 12, 22, 2, 3, 4, 5, 6, 21, 9, 29};
#endif
#if defined (CONFIG_KEYSCAN_USE_SIX_KEYS_TYPE)
static uint8_t g_gpio_map[CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL] = {31, 24, 14, 2, 3};
#endif
#if defined (CONFIG_KEYSCAN_USER_CONFIG_TYPE)
static uint8_t g_gpio_map[CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL] = { 0 };
#endif

#if defined (CONFIG_KEYSCAN_SUPPORT_LPM)
/* The value of g_gpio_map's clos' sum. */
static uint32_t g_lpm_pin_mask = 0;
static bool g_suspend_flag = false;
#endif

#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
static timer_handle_t g_keyscan_timer = NULL;

static void keyscan_timer_irq(uintptr_t data)
{
    unused(data);
    hal_keyscan_regs_set_slp_req(1);
    uint64_t base_time = uapi_tcxo_get_us();
    uint64_t cur_time = base_time;
    while (cur_time - base_time < REQ_WAIT_FOR_ACK) {
        if (hal_keyscan_regs_get_slp_step_sta(KEYSCAN_SLEEP_ACK) == 1) {
            hal_keyscan_regs_set_clk_keep(1);
            osal_irq_enable(KEY_SCAN_LOW_POWER_IRQN);
            uapi_pm_remove_sleep_veto(PM_VETO_ID_KEYSCAN);
            return;
        }
        cur_time = uapi_tcxo_get_us();
    }
    hal_keyscan_regs_set_slp_req(0);
}
#endif

uintptr_t keyscan_porting_base_addr_get(void)
{
    return g_keyscan_base_addr;
}

uintptr_t keyscan_m_ctl_porting_base_addr_get(void)
{
    return g_keyscan_m_ctl_base_addr;
}

void keyscan_port_register_hal_funcs(void)
{
#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
    uapi_pm_add_sleep_veto(PM_VETO_ID_KEYSCAN);
#endif
}

void keyscan_port_unregister_hal_funcs(void)
{
#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
    uapi_pm_remove_sleep_veto(PM_VETO_ID_KEYSCAN);
#endif
}

static int irq_keyscan_handler(int a, void *tmp)
{
    unused(a);
    unused(tmp);
#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
    uapi_timer_stop(g_keyscan_timer);
#endif
    hal_keyscan_v150_irq();
    osal_irq_clear(KEY_SCAN_IRQN);
#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
    uapi_timer_start(g_keyscan_timer, CONFIG_KEYSCAN_IDLE_WAIT_US, keyscan_timer_irq, 1);
#endif
    return 0;
}

#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
static int irq_keyscan_slp_handler(int a, void *tmp)
{
    unused(a);
    unused(tmp);
    if (hal_keyscan_regs_get_slp_step_sta(KEYSCAN_SLEEP_REQUIRE) == 0) {
        return 0;
    }
    osal_irq_disable(KEY_SCAN_LOW_POWER_IRQN);
    uapi_pm_add_sleep_veto(PM_VETO_ID_KEYSCAN);
    hal_keyscan_regs_set_clk_keep(0);
    hal_keyscan_regs_set_slp_req(0);
    hal_keyscan_v150_irq();
    osal_irq_clear(KEY_SCAN_LOW_POWER_IRQN);
    uapi_timer_start(g_keyscan_timer, CONFIG_KEYSCAN_IDLE_WAIT_US, keyscan_timer_irq, 1);
    return 0;
}
#endif

void keyscan_port_register_irq(uint32_t int_id)
{
#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
    uapi_timer_adapter(TIMER_INDEX_2, TIMER_2_IRQN, irq_prio(TIMER_2_IRQN));
    uapi_timer_create(TIMER_INDEX_2, &g_keyscan_timer);
#endif
    osal_irq_request(int_id, irq_keyscan_handler, NULL, NULL, NULL);
    osal_irq_set_priority(int_id, irq_prio(int_id));
    osal_irq_enable(int_id);
#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
    osal_irq_request(KEY_SCAN_LOW_POWER_IRQN, irq_keyscan_slp_handler, NULL, NULL, NULL);
    osal_irq_set_priority(KEY_SCAN_LOW_POWER_IRQN, irq_prio(KEY_SCAN_LOW_POWER_IRQN));
    uapi_timer_start(g_keyscan_timer, CONFIG_KEYSCAN_IDLE_WAIT_US, keyscan_timer_irq, 1);
#endif
}

void keyscan_port_unregister_irq(uint32_t int_id)
{
    osal_irq_free(int_id, NULL);
#if defined (CONFIG_KEYSCAN_SUPPORT_SLEEP)
    uapi_timer_delete(g_keyscan_timer);
    osal_irq_free(KEY_SCAN_LOW_POWER_IRQN, NULL);
#endif
#if defined (CONFIG_KEYSCAN_SUPPORT_LPM)
    g_suspend_flag = false;
#endif
}

void keyscan_porting_config_pins(void)
{
}

int keyscan_porting_set_gpio(uint8_t *user_gpio_map)
{
    int ret = memcpy_s(g_gpio_map, CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL,
                       user_gpio_map, CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL);
    return ret;
}

void keyscan_porting_pin_set(uint8_t row, uint8_t col)
{
    UNUSED(row);
    UNUSED(col);
    for (uint8_t i = 0; i < CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL; i++) {
        if (i > (CONFIG_KEYSCAN_ENABLE_ROW - 1)) {
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
            uapi_pin_set_ie(g_gpio_map[i], PIN_IE_1);
#endif /* CONFIG_PINCTRL_SUPPORT_IE */
        }
        uapi_pin_set_mode((pin_t)g_gpio_map[i], HAL_PIO_KEY_SCAN);
    }
    for (int i = 0; i < CONFIG_KEYSCAN_ENABLE_ROW; i++) {
        uapi_pin_set_pull(g_gpio_map[i], PIN_PULL_DOWN);
        /* 32bits register control 2 row. */
        reg16_setbits(KEYSCAN_PIN_SEL_ROW0_REG + ((i >> 1) * INTERVAL_OF_REGS),
                      (i % 0x2) * ODD_SEL_BIT, BIT_SEL_LEN, g_gpio_map[i]);
    }
    for (int i = 0; i < CONFIG_KEYSCAN_ENABLE_COL; i++) {
        /* 32bits register control 2 col. */
        uapi_pin_set_pull(g_gpio_map[i + CONFIG_KEYSCAN_ENABLE_ROW], PIN_PULL_UP);
        reg16_setbits(KEYSCAN_PIN_SEL_COL_REG + ((i >> 1) * INTERVAL_OF_REGS),
                      (i % 0x2) * ODD_SEL_BIT, BIT_SEL_LEN, g_gpio_map[i + CONFIG_KEYSCAN_ENABLE_ROW]);
    }
#if defined CONFIG_KEYSCAN_SUPPORT_SW_DEFENCE
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    uapi_pin_set_ie(CONFIG_KEYSCAN_ANOTHER_COL, PIN_IE_1);
#endif
    uapi_pin_set_mode(CONFIG_KEYSCAN_ANOTHER_COL, HAL_PIO_KEY_SCAN);
    uapi_pin_set_pull(CONFIG_KEYSCAN_ANOTHER_COL, PIN_PULL_DOWN);
    writew(MCU_KEYSCAN_PUPD_MAN_ADDR, 1);
#endif
}

void keyscan_porting_pin_resume(void)
{
    for (uint8_t i = 0; i < CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL; i++) {
        uapi_pin_set_mode((pin_t)g_gpio_map[i], HAL_PIO_KEY_SCAN);
    }
    for (int i = 0; i < CONFIG_KEYSCAN_ENABLE_ROW; i++) {
        /* 32bits register control 2 row. */
        reg16_setbits(KEYSCAN_PIN_SEL_ROW0_REG + ((i >> 1) * INTERVAL_OF_REGS),
                      (i % 0x2) * ODD_SEL_BIT, BIT_SEL_LEN, g_gpio_map[i]);
    }
    for (int i = 0; i < CONFIG_KEYSCAN_ENABLE_COL; i++) {
        /* 32bits register control 2 col. */
        reg16_setbits(KEYSCAN_PIN_SEL_COL_REG + ((i >> 1) * INTERVAL_OF_REGS),
                      (i % 0x2) * ODD_SEL_BIT, BIT_SEL_LEN, g_gpio_map[i + CONFIG_KEYSCAN_ENABLE_ROW]);
    }
}

#if defined (CONFIG_KEYSCAN_SUPPORT_LPM)
void keyscan_porting_suspend_pin_set(void)
{
    uapi_gpio_init();
    for (int8_t i = 0; i < CONFIG_KEYSCAN_ENABLE_ROW; i++) {
            /* Config the pins as low level output while they are controlled by ulp. */
            uapi_gpio_set_dir(g_gpio_map[i],GPIO_DIRECTION_OUTPUT);
            uapi_gpio_set_val(g_gpio_map[i],GPIO_LEVEL_LOW);
        }
    hal_keyscan_disable();
    for (int i = 0; i < CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL; i++) {
        uapi_pin_set_mode(g_gpio_map[i], HAL_PIO_FUNC_GPIO);
    }

    if (!g_suspend_flag) {
        for (int8_t i = 0; i < CONFIG_KEYSCAN_ENABLE_COL; i++) {
            g_lpm_pin_mask += 1 << g_gpio_map[i + CONFIG_KEYSCAN_ENABLE_ROW];
        }
        ulp_gpio_mask_pin_set(ULP_GPIO_WK0_MASK, g_lpm_pin_mask);
        g_suspend_flag = true;
    }
}
#endif