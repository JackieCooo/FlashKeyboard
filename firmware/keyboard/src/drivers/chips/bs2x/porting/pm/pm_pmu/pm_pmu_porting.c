/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides pm pmu port \n
 *
 * History: \n
 * 2023-07-29， Create file. \n
 */

#include "chip_io.h"
#include "soc_osal.h"
#include "platform_core.h"
#include "pinctrl.h"
#include "gpio.h"
#include "tcxo.h"
#include "pmu_ldo.h"
#include "pm_pmu.h"
#include "pm_pmu_porting.h"

typedef bool (*pmu_control_func_t)(uint8_t param);
#if defined(CONFIG_SUPPORT_NFC_SERVICE)
static bool pmu_control_nfcldo_power(pmu_control_power_t power);
#endif
static uint8_t g_micldo_vset = 0;
#ifdef PIN_RESET_ENABLE
#define PIN_RESET_DELAY 5ULL
#endif

#if defined(CONFIG_SUPPORT_NFC_SERVICE)
static bool pmu_control_nfcldo_power(pmu_control_power_t power)
{
    switch (power) {
        case PMU_CONTROL_POWER_OFF:
        case PMU_CONTROL_POWER_ON:
            pmu_nfcldo_power_cfg((uint8_t)power);
            break;

        default:
            return false;
    }
    return true;
}
#endif

static bool pmu_control_flashldo_power(pmu_control_power_t power)
{
    switch (power) {
        case PMU_CONTROL_POWER_OFF:
        case PMU_CONTROL_POWER_ON:
        case PMU_CONTROL_POWER_AUTO:
            pmu_flashldo_power_cfg((uint8_t)power);
            break;

        default:
            return false;
    }
    return true;
}

#ifdef PIN_RESET_ENABLE
static bool pmu_control_pin_reset_enable(pin_t pin)
{
    uapi_pin_set_mode(pin, 0);
    uapi_pin_set_pull(pin, PIN_PULL_UP);
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    uapi_pin_set_ie(pin, PIN_IE_1);
#endif
    reg16_setbit(0x5702C51C, 0x0);
    reg16_setbits(0x5702C51C, 0x4, 0x5, (uint8_t)pin);
    uapi_tcxo_delay_ms(PIN_RESET_DELAY);
    reg16_clrbit(0x5702C51C, 0x0);
    return true;
}

static bool pmu_control_pin_reset_disable(pin_t pin)
{
    reg16_setbit(0x5702C51C, 0x0);
    uapi_pin_set_mode(pin, 0);
    uapi_pin_set_pull(pin, PIN_PULL_NONE);
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    uapi_pin_set_ie(pin, PIN_IE_0);
#endif
    return true;
}
#endif

static bool pmu_control_micldo_power(pmu_control_power_t power)
{
    switch (power) {
        case PMU_CONTROL_POWER_OFF:
        case PMU_CONTROL_POWER_ON:
        case PMU_CONTROL_POWER_AUTO:
            pmu_micldo_power_cfg((uint8_t)power);
            break;

        default:
            return false;
    }
    return true;
}

static const pmu_control_func_t g_pmu_control_funcs[] = {
    pmu_control_flashldo_power,
    pmu_control_micldo_power,
#if defined(CONFIG_SUPPORT_NFC_SERVICE)
    pmu_control_nfcldo_power,
#endif
#ifdef PIN_RESET_ENABLE
    pmu_control_pin_reset_enable,
    pmu_control_pin_reset_disable,
#endif
};

errcode_t uapi_pmu_control(pmu_control_type_t type, uint8_t param)
{
    if (type >= PMU_CONTROL_TYPE_MAX) {
        return ERRCODE_INVALID_PARAM;
    }
    bool ret = false;
    ret = g_pmu_control_funcs[(uint8_t)type](param);
    return (ret == true) ? ERRCODE_SUCC : ERRCODE_FAIL;
}

errcode_t uapi_pmu_ldo_set_voltage(pmu_ldo_id_t id, uint8_t vset)
{
    uint32_t status = osal_irq_lock();
    switch (id) {
        case PMU_LDO_ID_BUCK_1P1:
            pmu_buck_ldo_vset_cfg(vset);
            break;
        case PMU_LDO_ID_SYSLDO:
            pmu_sysldo_vset_cfg(vset);
            break;
        case PMU_LDO_ID_FLASHLDO:
            pmu_flashldo_vset_cfg(vset);
            break;
        case PMU_LDO_ID_CLDO:
            pmu_cldo_vset_cfg(vset);
            break;
        case PMU_LDO_ID_MICLDO:
            pmu_micldo_vset_cfg(vset);
            break;
#if defined(CONFIG_SUPPORT_NFC_SERVICE)
        case PMU_LDO_ID_NFCLDO:
            pmu_nfcldo_vset_cfg(vset);
#endif
            break;
        default:
            osal_irq_restore(status);
            return ERRCODE_FAIL;
    }
    osal_irq_restore(status);
    return ERRCODE_SUCC;
}

uint8_t uapi_pmu_ldo_get_voltage(pmu_ldo_id_t id)
{
    switch (id) {
        case PMU_LDO_ID_BUCK_1P1:
            return pmu_buck_ldo_get_vset();
        case PMU_LDO_ID_SYSLDO:
            return pmu_sysldo_get_vset();
        case PMU_LDO_ID_FLASHLDO:
            return pmu_flashldo_get_vset();
        case PMU_LDO_ID_CLDO:
            return pmu_cldo_get_vset();
        case PMU_LDO_ID_MICLDO:
            return pmu_micldo_get_vset();
#if defined(CONFIG_SUPPORT_NFC_SERVICE)
        case PMU_LDO_ID_NFCLDO:
            return pmu_nfcldo_get_vset();
#endif
        default:
            return 0;
    }
}

void pm_efuse_ldo_power(bool on)
{
    if (on) {
        g_micldo_vset = uapi_pmu_ldo_get_voltage(PMU_LDO_ID_MICLDO);
        // efuse写依赖micldo电压2.5v
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_2V5);
        // 打开micldo
        uapi_pmu_control(PMU_CONTROL_MICLDO_POWER, PMU_CONTROL_POWER_ON);
    } else {
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, g_micldo_vset);
        // 关闭micldo
        uapi_pmu_control(PMU_CONTROL_MICLDO_POWER, PMU_CONTROL_POWER_OFF);
    }
}