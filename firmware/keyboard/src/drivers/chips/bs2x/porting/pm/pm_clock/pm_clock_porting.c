/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides pm clock port \n
 *
 * History: \n
 * 2023-07-29， Create file. \n
 */

#include "chip_io.h"
#include "platform_core.h"
#include "osal_interrupt.h"
#include "pm_clock.h"
#include "pm_pmu.h"
#include "clocks_switch.h"
#include "tcxo.h"
#include "pinctrl.h"
#include "pm_clock_porting.h"

#define M_CTL_M_CLKEN0_REG                  (M_CTL_RB_BASE + 0x40)
#define M_CTL_M_CLKEN1_REG                  (M_CTL_RB_BASE + 0x44)
#define M_CTL_M_CLKEN2_REG                  (M_CTL_RB_BASE + 0x48)
#define M_CTL_GLB_CLKEN_REG                 (M_CTL_RB_BASE + 0x548)
#define PMU2_CMU_CTL_XO_CLK_OUT_EN          (PMU2_CMU_CTL_RB_BASE + 0x410)

#define pm_setbit(src, pos)   ((src) |= ((uint8_t)((1U) << (uint8_t)(pos))))
#define pm_clrbit(src, pos)   ((src) &= ~((uint8_t)((1U) << (uint8_t)(pos))))

#if (BS21_DLL2_ENABLE == YES)
static uint8_t g_pm_cldo_1v1_vote = BIT(CLOCK_CRG_ID_MCU_CORE) | BIT(CLOCK_CRG_ID_XIP_QSPI);
static clock_vset_level_t g_pm_current_cldo_vset = CLOCK_VSET_LEVEL_1V1;
#else
static uint8_t g_pm_cldo_1v1_vote = 0;
static clock_vset_level_t g_pm_current_cldo_vset = CLOCK_VSET_LEVEL_1V0;
#endif

static uint8_t g_pm_mcpu_64m_freq_used = 0;
#define PM_MCPU_FREQ_USED_BY_MCU    0
#define PM_MCPU_FREQ_USED_BY_BTC    1
#define PM_MCPU_FREQ_USED_BY_BTH    2

static uint8_t g_lpc_ccrg_clk_spi_used = 0;

static clock_vset_level_t pm_pm_get_vset_by_clk(clock_clk_src_t clk_src)
{
    if (clk_src == CLOCK_CLK_SRC_TCXO_2X) {
        return CLOCK_VSET_LEVEL_1V1;
    }

    return CLOCK_VSET_LEVEL_1V0;
}

static void pm_cldo_vset_vote(uint8_t id, clock_vset_level_t level)
{
    if (level == CLOCK_VSET_LEVEL_1V1) {
        pm_setbit(g_pm_cldo_1v1_vote, id);
    } else {
        pm_clrbit(g_pm_cldo_1v1_vote, id);
    }
}

static void pm_buck_vset_raise_judge(void)
{
    if ((g_pm_cldo_1v1_vote != 0) && (g_pm_current_cldo_vset != CLOCK_VSET_LEVEL_1V1)) {
        writew(0x52000540, 0x7);    // Open dll2.
#ifdef CONFIG_POWER_SUPPLY_BY_LDO
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_1V3);
#else
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_BUCK_1P1, PMU_BUCK_1P1_VSET_1V2);
#endif
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_CLDO, PMU_CLDO_VSET_1V1);
        uapi_tcxo_delay_us(120ULL); // 升压之后delay120us
        g_pm_current_cldo_vset = CLOCK_VSET_LEVEL_1V1;
    }
}

static void pm_buck_vset_reduce_judge(void)
{
    if ((g_pm_cldo_1v1_vote == 0) && (g_pm_current_cldo_vset == CLOCK_VSET_LEVEL_1V1)) {
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_CLDO, PMU_CLDO_VSET_1V05);
#ifdef CONFIG_POWER_SUPPLY_BY_LDO
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_1V1);
#else
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_BUCK_1P1, PMU_BUCK_1P1_VSET_1V1);
#endif
        g_pm_current_cldo_vset = CLOCK_VSET_LEVEL_1V0;
        writew(0x52000540, 0x6);    // Close dll2.
    }
}

errcode_t uapi_clock_crg_config(clock_crg_id_t id, clock_clk_src_t clk_src, uint8_t clk_div)
{
    clock_vset_level_t level = pm_pm_get_vset_by_clk(clk_src);

    pm_cldo_vset_vote((uint8_t)id, level);

    pm_buck_vset_raise_judge();

    system_ccrg_clock_config((clocks_ccrg_module_t)id, (clocks_clk_src_t)clk_src, clk_div);

    pm_buck_vset_reduce_judge();

    return ERRCODE_SUCC;
}

uint32_t uapi_clock_crg_get_freq(clock_crg_id_t id)
{
    return clocks_get_module_frequency((clocks_ccrg_module_t)id);
}

void uapi_clock_clken_config(clock_clken_id_t id, bool clk_en)
{
    unused(id);
    unused(clk_en);
}

static bool clock_control_freq_level_config(uint8_t level, uint8_t module)
{
    if (level == CLOCK_FREQ_LEVEL_LOW_POWER) {
        pm_clrbit(g_pm_mcpu_64m_freq_used, module);
        if (g_pm_mcpu_64m_freq_used == 0) {
            uapi_clock_crg_config(CLOCK_CRG_ID_MCU_CORE, CLOCK_CLK_SRC_TCXO, 0x1);
            uapi_clock_crg_config(CLOCK_CRG_ID_XIP_QSPI, CLOCK_CLK_SRC_TCXO, 0x1);
        }
    } else if (level == CLOCK_FREQ_LEVEL_HIGH) {
        if (g_pm_mcpu_64m_freq_used == 0) {
            uapi_clock_crg_config(CLOCK_CRG_ID_MCU_CORE, CLOCK_CLK_SRC_TCXO_2X, 0x1);
            uapi_clock_crg_config(CLOCK_CRG_ID_XIP_QSPI, CLOCK_CLK_SRC_TCXO_2X, 0x1);
        }
        pm_setbit(g_pm_mcpu_64m_freq_used, module);
    }
    return true;
}

static bool clock_control_mclken_switch(uint8_t type, bool on)
{
    if (type < CLOCK_APERP_MTIMER_CLKEN) {
        on ? reg16_setbit(M_CTL_M_CLKEN0_REG, type) : reg16_clrbit(M_CTL_M_CLKEN0_REG, type);
    } else if (type < CLOCK_APERP_MTIMER_PERP_CLKEN) {
        on ? reg16_setbit(M_CTL_M_CLKEN1_REG, (type - CLOCK_APERP_MTIMER_CLKEN)) : \
            reg16_clrbit(M_CTL_M_CLKEN1_REG, (type - CLOCK_APERP_MTIMER_CLKEN));
    } else {
        on ? reg16_setbit(M_CTL_M_CLKEN2_REG, (type - CLOCK_APERP_MTIMER_PERP_CLKEN)) : \
            reg16_clrbit(M_CTL_M_CLKEN2_REG, (type - CLOCK_APERP_MTIMER_PERP_CLKEN));
    }
    return true;
}

static bool clock_control_glb_clken_switch(uint8_t type, bool on)
{
    on ? reg16_setbit(M_CTL_GLB_CLKEN_REG, type) : reg16_clrbit(M_CTL_GLB_CLKEN_REG, type);
    return true;
}

static bool clock_control_xo_out_switch(uint8_t type, bool on)
{
    on ? reg16_setbit(PMU2_CMU_CTL_XO_CLK_OUT_EN, type) : reg16_clrbit(PMU2_CMU_CTL_XO_CLK_OUT_EN, type);
    return true;
}

#if defined(CLOCK_TEST_ENABLE)
static bool clock_control_32k_test_enable(pin_t pin)
{
    writew(0x52004414, 0x5);    // 观测时钟0：CLK_32K_TEST
    writew(0x52004418, 0x0);    // 分频使能：disable
    writew(0x52004400, 0x8);    // DIAG选择：clk_test
    writew(0x52004404, 0x0);    // DIAG PIN选择：观测时钟0
    writew(0x52004408, 0x0);    // DIAG PIN选择：观测时钟0
    writew(0x5200440C, 0x0);    // DIAG PIN选择：观测时钟0
    writew(0x52004410, 0x0);    // DIAG PIN选择：观测时钟0
    writew(0x52004020, 0x1);    // CLOCK_TEST时钟CG使能
    writew(0x52004434, 0x1);    // 输出时钟使能
    uapi_pin_set_mode(pin, 0x7D);
    return true;
}

static bool clock_control_32k_test_disable(pin_t pin)
{
    writew(0x52004414, 0x0);    // 观测时钟0：NONE
    writew(0x52004020, 0x0);    // CLOCK_TEST时钟CG使能
    writew(0x52004434, 0x0);    // 输出时钟使能
    uapi_pin_set_mode(pin, 0x0);
    return true;
}
#endif

errcode_t uapi_clock_control(clock_control_type_t type, uint8_t param)
{
    if (type >= CLOCK_CONTROL_TYPE_MAX) {
        return ERRCODE_INVALID_PARAM;
    }
    bool ret;
    switch (type) {
        case CLOCK_CONTROL_FREQ_LEVEL_CONFIG:
        case CLOCK_CONTROL_BTC_FREQ_CONFIG:
        case CLOCK_CONTROL_BTH_FREQ_CONFIG:
        case CLOCK_CONTROL_USB_FREQ_CONFIG:
        case CLOCK_CONTROL_CAN_FREQ_CONFIG:
            ret = clock_control_freq_level_config(param, type);
            break;
        case CLOCK_CONTROL_MCLKEN_ENABLE:
            ret = clock_control_mclken_switch(param, true);
            break;
        case CLOCK_CONTROL_MCLKEN_DISABLE:
            ret = clock_control_mclken_switch(param, false);
            break;
        case CLOCK_CONTROL_GLB_CLKEN_ENABLE:
            ret = clock_control_glb_clken_switch(param, true);
            break;
        case CLOCK_CONTROL_GLB_CLKEN_DISABLE:
            ret = clock_control_glb_clken_switch(param, false);
            break;
        case CLOCK_CONTROL_XO_OUT_ENABLE:
            ret = clock_control_xo_out_switch(param, true);
            break;
        case CLOCK_CONTROL_XO_OUT_DISABLE:
            ret = clock_control_xo_out_switch(param, false);
            break;
#if defined(CLOCK_TEST_ENABLE)
        case CLOCK_CONTROL_32K_TEST_ENABLE:
            ret = clock_control_32k_test_enable(param);
            break;
        case CLOCK_CONTROL_32K_TEST_DISABLE:
            ret = clock_control_32k_test_disable(param);
            break;
#endif
        default:
            break;
    }
    return (ret == true) ? ERRCODE_SUCC : ERRCODE_FAIL;
}

void clock_ccrg_spi_enable(clock_ccrg_spi_used_t dev, bool en)
{
    if (en) {
        if (g_lpc_ccrg_clk_spi_used == 0) {
            uapi_clock_crg_config(CLOCK_CRG_ID_MCU_PERP_SPI, CLOCK_CLK_SRC_TCXO, 0x1);
        }
        pm_setbit(g_lpc_ccrg_clk_spi_used, dev);
    } else {
        pm_clrbit(g_lpc_ccrg_clk_spi_used, dev);
        if (g_lpc_ccrg_clk_spi_used == 0) {
            uapi_clock_crg_config(CLOCK_CRG_ID_MCU_PERP_SPI, CLOCK_CLK_SRC_NONE, 0x1);
        }
    }
}