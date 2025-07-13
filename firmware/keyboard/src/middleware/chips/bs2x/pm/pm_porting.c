/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * Description: Provides pm port \n
 *
 * History: \n
 * 2023-01-13， Create file. \n
 */

#include "pm_sleep.h"
#include "pm_veto.h"
#include "pm_sleep_porting.h"
#include "pm_dev.h"
#include "pm.h"
#include "sfc.h"
#include "cpu_trace.h"
#include "watchdog.h"
#include "uart.h"
#include "los_task_pri.h"
#include "ulp_gpio.h"
#include "debug_print.h"
#include "preserve.h"
#include "tcxo.h"
#if defined(PM_SUPPORT_USB_WKUP)
#include "dwc_otg_pcd.h"
#endif

#define ULP_RST_RECORD           0x5702C520
#define ULP_PIN_RST_BIT          0x5
#define ULP_WDT_RST_BIT          0x4
#define ULP_RST_BIT_CLR          0x0
#define ULP_RST_BIT_LEN          0x2

#if defined(PM_SUPPORT_USB_WKUP)
static pm_usb_cbk_t g_pm_usb_suspend_cbk = NULL;
static pm_usb_cbk_t g_pm_usb_resume_cbk = NULL;
#endif

#if defined(CONFIG_PM_POWER_GATING_ENABLE) && (CONFIG_PM_POWER_GATING_ENABLE == 1)
#if defined(CONFIG_UART_SUPPORT_LPM)
static const pm_dev_ops_t g_dev_uart_register = {
    .suspend = uapi_uart_suspend,
    .suppend_arg = 1,
    .resume = uapi_uart_resume,
    .resume_arg = 1,
};
#endif  /* CONFIG_UART_SUPPORT_LPM */

#if defined(CONFIG_WATCHDOG_SUPPORT_LPM)
static const pm_dev_ops_t g_dev_watchdog_register = {
    .suspend = uapi_watchdog_suspend,
    .suppend_arg = 1,
    .resume = watchdog_resume,
    .resume_arg = 1,
};
#endif  /* CONFIG_WDT_SUPPORT_LPM */

#if defined (CONFIG_SFC_SUPPORT_LPM)
static const pm_dev_ops_t g_dev_cache_register = {
    .suspend = mpu_cache_suspend,
    .suppend_arg = 1,
    .resume = mpu_cache_resume,
    .resume_arg = 1,
};
#endif
#if defined(CONFIG_CPU_TRACE_SUPPORT_LPM)
static const pm_dev_ops_t g_dev_cpu_trace_register = {
    .suspend = cpu_trace_suspend,
    .suppend_arg = 1,
    .resume = cpu_trace_resume,
    .resume_arg = 1,
};
#endif
#endif

#if defined(PM_MCPU_MIPS_STATISTICS_ENABLE) && (PM_MCPU_MIPS_STATISTICS_ENABLE == YES)
static uint64_t g_pm_time_before_sleep = 0;
static uint64_t g_pm_time_after_sleep = 0;
static uint32_t g_pm_total_idle_time = 0;
static uint32_t g_pm_total_work_time = 0;
#endif

#if (PM_ULP_GPIO_WKUP_ENABLE == YES)
ulp_gpio_int_wkup_cfg_t g_wk_cfg[] = {
    { 0, 20, true, ULP_GPIO_INTERRUPT_FALLING_EDGE, NULL }, // ULP_GPIO0: S_MGPIO20(20)
    { 1, 33, true, ULP_GPIO_INTERRUPT_RISING_EDGE, NULL },  // ULP_GPIO1: SWD_IO(33)
#if defined(CONFIG_KEYSCAN_SUPPORT_LPM)
    { 7, 35, true, ULP_GPIO_INTERRUPT_FALLING_EDGE, NULL },  // ULP_GPIO7: MASK0(35)
#endif
};
#endif

static const uint16_t g_wakeup_wait_time_reg_config[] = {
    0x14C,   // PMU_SYSLDO_ECO_EN_BOOT_TIME
    0x154,   // PMU_REF1_IBG_EN_BOOT_TIME
    0x198,   // RST_BOOT_32K_N_WKUP_TIME
    0x19C,   // ULP_WKUP_AON_WKUP_TIME
    0x620,   // EN_XLDO_DELAY_TIME_SOC
    0x624,   // EN_INTLDO2_DELAY_TIME_SOC
    0x120,   // PMU_SYS_OUT_SEL_CFG
    0x634,   // AON_SLP_RDY_WKUP_TIME_SOC
    0x638,   // RST_AON_CRG_WKUP_TIME_SOC
    0x17C,   // PMU_CLDO_VSET_ECO_BOOT_TIME
    0x644,   // REF2_EN_BG_WKUP_TIME_SOC
    0x63C,   // GLB_CLK_FORCE_ON_WKUP_TIME_SOC
    0x648,   // EN_REFBUFFER_WKUP_TIME_SOC
    0x640,   // RST_AON_LGC_WKUP_TIME_SOC
    0x64C,   // EN_INTLDO2_WKUP_TIME_SOC
    0x1A0,   // ULP_SLP_HLD_TIME
    0x16C,   // PMU_MICLDO_EN_BOOT_TIME
    0x650,   // EN_IBG_WKUP_TIME_SOC
    0x654,   // EN_IPOLY_WKUP_TIME_SOC
    0x658,   // EN_ITUNE_WKUP_TIME_SOC
    0x65C,   // EN_XLDO_WKUP_TIME_SOC
    0x158,   // PMU_UVLO_EN_BOOT_TIME
    0x15C,   // TCM_VDDC_POWER_ON_SEL_BOOT_TIME
    0x160,   // BUCK_EN_BOOT_TIME
    0x164,   // BUCK_VSET_ECO_BOOT_TIME
    0x168,   // BUCK_SLEEP_BOOT_TIME
    0x174,   // PMU_CLDO_SW_BOOT_TIME
    0x178,   // PMU_CLDO_EN_BOOT_TIME
    0x660,   // XO_CORE_PD_WKUP_TIME_SOC
    0x664,   // FAST_XO_ISO_WKUP_TIME_SOC
    0x668,   // RC_PD_WKUP_TIME_SOC
    0x66C,   // RC_RSTN_WKUP_TIME_SOC
    0x678,   // FAST_XO_LOOP_RSTN_WKUP_TIME_SOC
    0x170,   // PMU_SYS_OUT_SEL_BOOT_TIME
    0x180,   // VDD0P7_TO_SYS_ISO_EN_BOOT_TIME
    0x184,   // RST_BOOT_32K_N_BOOT_TIME
    0x188,   // ULP_WKUP_AON_BOOT_TIME
    0x18C,   // PMU_FLASHLDO_EN_BOOT_TIME
    0x67C,   // XO2DBB_CLKOUT_EN_WKUP_TIME_SOC
    0x688,   // RC2_DBB_PD_WKUP_TIME_SOC
    0x68C,   // RCCLK2_EN_WKUP_TIME_SOC
    0x690,   // RC2_CLKEN_WKUP_TIME_SOC
    0x694,   // B32M_CLKEN_WKUP_TIME_SOC
    0x698,   // A32M_CLKEN_WKUP_TIME_SOC
    0x69C,   // TCXO_EN_WKUP_TIME_SOC
    0x6A0,   // APERP_32K_SEL_WKUP_TIME_SOC
    0x0E8,   // ram_test1_pulse_boot_time
    0x6A4,   // RST_PWR_C1_CRG_N_WKUP_TIME_SOC
    0x6A8,   // RST_PWR_C1_LGC_N_WKUP_TIME_SOC
    0x6AC,   // RST_PWR_C1_CPU_N_WKUP_TIME_SOC
    0x6B0,   // M_SYS_WKING_COMPLETE_TIME
};

static const uint8_t g_wakeup_wait_time_val_config[] = {
    0x1,    // PMU_SYSLDO_ECO_EN_BOOT_TIME
    0x1,    // PMU_REF1_IBG_EN_BOOT_TIME
    0x1,    // RST_BOOT_32K_N_WKUP_TIME
    0x1,    // ULP_WKUP_AON_WKUP_TIME
    0x4,    // EN_XLDO_DELAY_TIME_SOC
    0x4,    // EN_INTLDO2_DELAY_TIME_SOC
    0x1,    // PMU_SYS_OUT_SEL_CFG
    0x1,    // AON_SLP_RDY_WKUP_TIME_SOC
    0x1,    // RST_AON_CRG_WKUP_TIME_SOC
    0x1,    // PMU_CLDO_VSET_ECO_BOOT_TIME
    0x1,    // REF2_EN_BG_WKUP_TIME_SOC
    0x2,    // GLB_CLK_FORCE_ON_WKUP_TIME_SOC
    0x2,    // EN_REFBUFFER_WKUP_TIME_SOC
    0x3,    // RST_AON_LGC_WKUP_TIME_SOC
    0x3,    // EN_INTLDO2_WKUP_TIME_SOC
    0x6,    // ULP_SLP_HLD_TIME
#ifdef CONFIG_PM_POWER_GATING_ENABLE
    0x7,    // PMU_MICLDO_EN_BOOT_TIME
#else
    0x2,    // PMU_MICLDO_EN_BOOT_TIME
#endif
    0x8,    // EN_IBG_WKUP_TIME_SOC
    0x8,    // EN_IPOLY_WKUP_TIME_SOC
    0x8,    // EN_ITUNE_WKUP_TIME_SOC
    0x8,    // EN_XLDO_WKUP_TIME_SOC
    0xB,    // PMU_UVLO_EN_BOOT_TIME
    0xB,    // TCM_VDDC_POWER_ON_SEL_BOOT_TIME
    0xB,    // BUCK_EN_BOOT_TIME
    0xB,    // BUCK_VSET_ECO_BOOT_TIME
    0xA,    // BUCK_SLEEP_BOOT_TIME
    0xD,    // PMU_CLDO_SW_BOOT_TIME
    0xD,    // PMU_CLDO_EN_BOOT_TIME
    0xE,    // XO_CORE_PD_WKUP_TIME_SOC
    0xF,    // FAST_XO_ISO_WKUP_TIME_SOC
    0xF,    // RC_PD_WKUP_TIME_SOC
    0xF,    // RC_RSTN_WKUP_TIME_SOC
    0x10,   // FAST_XO_LOOP_RSTN_WKUP_TIME_SOC
#ifdef CONFIG_PM_POWER_GATING_ENABLE
    0x12,   // PMU_SYS_OUT_SEL_BOOT_TIME
#else
    0x9,    // PMU_SYS_OUT_SEL_BOOT_TIME
#endif
    0x13,   // VDD0P7_TO_SYS_ISO_EN_BOOT_TIME
    0x13,   // RST_BOOT_32K_N_BOOT_TIME
#ifdef CONFIG_PM_POWER_GATING_ENABLE
    0x13,   // ULP_WKUP_AON_BOOT_TIME
    0x13,   // PMU_FLASHLDO_EN_BOOT_TIME
#else
    0xA,    // ULP_WKUP_AON_BOOT_TIME
    0xA,    // PMU_FLASHLDO_EN_BOOT_TIME
#endif
#ifdef CONFIG_PM_XO_FAST_START_ENABLE
    0x18,   // XO2DBB_CLKOUT_EN_WKUP_TIME_SOC
    0x18,   // RC2_DBB_PD_WKUP_TIME_SOC
    0x18,   // RCCLK2_EN_WKUP_TIME_SOC
    0x18,   // RC2_CLKEN_WKUP_TIME_SOC
    0x19,   // B32M_CLKEN_WKUP_TIME_SOC
    0x19,   // A32M_CLKEN_WKUP_TIME_SOC
    0x19,   // TCXO_EN_WKUP_TIME_SOC
    0x19,   // APERP_32K_SEL_WKUP_TIME_SOC
    0x1A,   // ram_test1_pulse_boot_time
    0x1A,   // RST_PWR_C1_CRG_N_WKUP_TIME_SOC
    0x1B,   // RST_PWR_C1_LGC_N_WKUP_TIME_SOC
    0x1C,   // RST_PWR_C1_CPU_N_WKUP_TIME_SOC
    0x1C,   // M_SYS_WKING_COMPLETE_TIME
#else
    0x30,   // XO2DBB_CLKOUT_EN_WKUP_TIME_SOC
    0x30,   // RC2_DBB_PD_WKUP_TIME_SOC
    0x30,   // RCCLK2_EN_WKUP_TIME_SOC
    0x30,   // RC2_CLKEN_WKUP_TIME_SOC
    0x31,   // B32M_CLKEN_WKUP_TIME_SOC
    0x31,   // A32M_CLKEN_WKUP_TIME_SOC
    0x31,   // TCXO_EN_WKUP_TIME_SOC
    0x31,   // APERP_32K_SEL_WKUP_TIME_SOC
    0x1A,   // ram_test1_pulse_boot_time
    0x32,   // RST_PWR_C1_CRG_N_WKUP_TIME_SOC
    0x33,   // RST_PWR_C1_LGC_N_WKUP_TIME_SOC
    0x34,   // RST_PWR_C1_CPU_N_WKUP_TIME_SOC
    0x34,   // M_SYS_WKING_COMPLETE_TIME
#endif
};

static const uint16_t g_sleep_wait_time_reg_config0[] = {
    0x0E4,    // PMU_SYSLDO_ECO_EN_SLP_TIME_SOC
    0x1A8,    // PMU_REF1_IBG_EN_SLP_TIME_SOC
    0x1AC,    // PMU_UVLO_EN_SLP_TIME_SOC
    0x1B0,    // TCM_VDDC_POWER_ON_SEL_SLP_TIME_SOC
    0x1B4,    // BUCK_EN_SLP_TIME_SOC
    0x0C0,    // PMU_MICLDO_EN_SLP_TIME_SOC
    0x0C4,    // PMU_SYS_OUT_SEL_SLP_TIME_SOC
    0x0C8,    // PMU_CLDO_SW_SLP_TIME_SOC
    0x0D0,    // PMU_CLDO_EN_SLP_TIME_SOC
    0x1B8,    // BUCK_VSET_ECO_SLP_TIME_SOC
    0x1BC,    // BUCK_SLEEP_SLP_TIME_SOC
    0x0D4,    // PMU_CLDO_VSET_ECO_SLP_TIME_SOC
    0x0D8,    // VDD0P7_TO_SYS_ISO_EN_SLP_TIME_SOC
    0x0DC,    // RST_BOOT_32K_N_SLP_TIME_SOC
    0x0E0,    // ULP_WKUP_AON_SLP_TIME_SOC
    0x0EC,    // PMU_FLASHLDO_EN_SLP_TIME_SOC
};

static const uint8_t g_sleep_wait_time_val_config0[] = {
#ifdef CONFIG_PM_POWER_GATING_ENABLE
    0xF,    // PMU_SYSLDO_ECO_EN_SLP_TIME_SOC
    0xF,    // PMU_REF1_IBG_EN_SLP_TIME_SOC
    0xE,    // PMU_UVLO_EN_SLP_TIME_SOC
    0xD,    // TCM_VDDC_POWER_ON_SEL_SLP_TIME_SOC
    0xD,    // BUCK_EN_SLP_TIME_SOC
    0xD,    // PMU_MICLDO_EN_SLP_TIME_SOC
    0xC,    // PMU_SYS_OUT_SEL_SLP_TIME_SOC
    0xB,    // PMU_CLDO_SW_SLP_TIME_SOC
    0xC,    // PMU_CLDO_EN_SLP_TIME_SOC
#else
    0x6,    // PMU_SYSLDO_ECO_EN_SLP_TIME_SOC
    0x6,    // PMU_REF1_IBG_EN_SLP_TIME_SOC
    0x5,    // PMU_UVLO_EN_SLP_TIME_SOC
    0x4,    // TCM_VDDC_POWER_ON_SEL_SLP_TIME_SOC
    0x4,    // BUCK_EN_SLP_TIME_SOC
    0x4,    // PMU_MICLDO_EN_SLP_TIME_SOC
    0x3,    // PMU_SYS_OUT_SEL_SLP_TIME_SOC
    0x2,    // PMU_CLDO_SW_SLP_TIME_SOC
    0x2,    // PMU_CLDO_EN_SLP_TIME_SOC
#endif
    0x8,    // BUCK_VSET_ECO_SLP_TIME_SOC
    0x8,    // BUCK_SLEEP_SLP_TIME_SOC
    0x2,    // PMU_CLDO_VSET_ECO_SLP_TIME_SOC
    0x2,    // VDD0P7_TO_SYS_ISO_EN_SLP_TIME_SOC
    0x1,    // RST_BOOT_32K_N_SLP_TIME_SOC
    0x1,    // ULP_WKUP_AON_SLP_TIME_SOC
    0x1,    // PMU_FLASHLDO_EN_SLP_TIME_SOC
};

static const uint16_t g_sleep_wait_time_reg_config1[] = {
    0x2A0,    // AON_SLP_RDY_SLP_TIME
    0x2A4,    // REF2_EN_BG_SLP_TIME_SOC
    0x2A8,    // EN_REFBUFFER_SLP_TIME_SOC
    0x2AC,    // EN_INTLDO2_SLP_TIME_SOC
    0x2B0,    // EN_IBG_SLP_TIME_SOC
    0x2B4,    // EN_IPOLY_SLP_TIME_SOC
    0x2B8,    // EN_ITUNE_SLP_TIME_SOC
    0x2C4,    // EN_CLKLDO1_SLP_TIME
    0x2C8,    // EN_XLDO_SLP_TIME
    0x2D0,    // XO_CORE_PD_SLP_TIME
    0x2D4,    // FAST_XO_ISO_SLP_TIME
    0x2D8,    // RC_PD_SLP_TIME
    0x2DC,    // RC_RSTN_SLP_TIME
    0x2E8,    // FAST_XO_LOOP_RSTN_SLP_TIME
    0x2EC,    // XO2DBB_CLKOUT_EN_SLP_TIME
    0x2F8,    // RC2_DBB_PD_SLP_TIME
    0x2FC,    // RCCLK2_EN_SLP_TIME
    0x300,    // RC2_CLKEN_SLP_TIME
    0x304,    // B32M_CLKEN_SLP_TIME
    0x308,    // A32M_CLKEN_SLP_TIME
    0x30C,    // TCXO_EN_SLP_TIME
    0x310,    // APERP_32K_SEL_SLP_TIME
    0x314,    // RST_PWR_C1_CRG_N_SLP_TIME
    0x318,    // RST_PWR_C1_LGC_N_SLP_TIME
    0x31C,    // RST_PWR_C1_CPU_N_SLP_TIME
};

static const uint8_t g_sleep_wait_time_val_config1[] = {
    0x3,    // AON_SLP_RDY_SLP_TIME
    0x3,    // REF2_EN_BG_SLP_TIME_SOC
    0x3,    // EN_REFBUFFER_SLP_TIME_SOC
    0x2,    // EN_INTLDO2_SLP_TIME_SOC
    0x3,    // EN_IBG_SLP_TIME_SOC
    0x3,    // EN_IPOLY_SLP_TIME_SOC
    0x3,    // EN_ITUNE_SLP_TIME_SOC
    0x3,    // EN_CLKLDO1_SLP_TIME
    0x2,    // EN_XLDO_SLP_TIME
    0x2,    // XO_CORE_PD_SLP_TIME
    0x2,    // FAST_XO_ISO_SLP_TIME
    0x2,    // RC_PD_SLP_TIME
    0x2,    // RC_RSTN_SLP_TIME
    0x2,    // FAST_XO_LOOP_RSTN_SLP_TIME
    0x2,    // XO2DBB_CLKOUT_EN_SLP_TIME
    0x2,    // RC2_DBB_PD_SLP_TIME
    0x2,    // RCCLK2_EN_SLP_TIME
    0x2,    // RC2_CLKEN_SLP_TIME
    0x2,    // B32M_CLKEN_SLP_TIME
    0x2,    // A32M_CLKEN_SLP_TIME
    0x2,    // TCXO_EN_SLP_TIME
    0x1,    // APERP_32K_SEL_SLP_TIME
    0x1,    // RST_PWR_C1_CRG_N_SLP_TIME
    0x1,    // RST_PWR_C1_LGC_N_SLP_TIME
    0x1,    // RST_PWR_C1_CPU_N_SLP_TIME
};

// time from SOC wakeup to btcbb wakeup, us
static uint16_t g_protocol_wakeup_advance_time = 0;

// time from btcbb wakeup to tx/rx en, us
static uint16_t g_protocol_work_advance_time = 0;

void pm_set_protocol_wakeup_advance_time(uint16_t advance_time)
{
    g_protocol_wakeup_advance_time = advance_time;
}

uint16_t pm_get_protocol_wakeup_advance_time(void)
{
    return g_protocol_wakeup_advance_time;
}

void pm_set_protocol_work_advance_time(uint16_t advance_time)
{
    g_protocol_work_advance_time = advance_time;
}

uint16_t pm_get_protocol_work_advance_time(void)
{
    return g_protocol_work_advance_time;
}

static void pm_protocol_wakeup_time_init(void)
{
    uint16_t wk_us = 6000;  // 6000：基准提前时间
#if defined(CONFIG_PM_POWER_GATING_ENABLE) && (CONFIG_PM_POWER_GATING_ENABLE == 1)
#if defined(CONFIG_SFC_SUPPORT_LPM)
    wk_us += 200;   // 200：SFC恢复
#endif
#if defined(CONFIG_WATCHDOG_SUPPORT_LPM)
    wk_us += 150;   // 150：wdt恢复
#endif
#if defined(CONFIG_UART_SUPPORT_LPM)
    wk_us += 1200;  // 1200：uart恢复
#endif
#if defined(CONFIG_CPU_TRACE_SUPPORT_LPM)
    wk_us += 200;   // 200：trace恢复
#endif
#ifndef CONFIG_PM_XO_FAST_START_ENABLE
    wk_us += 720;   // 720：xo起振增量
#endif
#if defined(PM_SLEEP_DEBUG_ENABLE) && (PM_SLEEP_DEBUG_ENABLE == YES)
    wk_us += 3000;  // 3000：唤醒源维测时间增量
#endif
#endif

    pm_set_protocol_wakeup_advance_time(wk_us);
}

static void pm_wakeup_wait_time_config(void)
{
    for (uint32_t i = 0; i < sizeof(g_wakeup_wait_time_reg_config) / sizeof(uint16_t); i++) {
        writew(ULP_AON_CTL_RB_ADDR + g_wakeup_wait_time_reg_config[i], g_wakeup_wait_time_val_config[i]);
    }
}

static void pm_sleep_wait_time_config(void)
{
    for (uint32_t i = 0; i < sizeof(g_sleep_wait_time_reg_config0) / sizeof(uint16_t); i++) {
        writew(ULP_AON_CTL_RB_ADDR + g_sleep_wait_time_reg_config0[i], g_sleep_wait_time_val_config0[i]);
    }
    for (uint32_t i = 0; i < sizeof(g_sleep_wait_time_reg_config1) / sizeof(uint16_t); i++) {
        writew(PMU1_CTL_RB_BASE + g_sleep_wait_time_reg_config1[i], g_sleep_wait_time_val_config1[i]);
    }
}

__attribute__((section(".PMRAMCODE"))) void pm_auto_cg_config(void)
{
        writew(0x57000a00, 0x0);    // AON_AUTO_CG_CFG
        writew(0x52000190, 0x0);    // DAP_H2P_AUTOCG_BYPASS
        writew(0x52000194, 0x0);    // DMA_LP_CTL
        writew(0x52000a2c, 0x0);    // BUS_CG_CTL0
        writew(0x52000a30, 0x0);    // BUS_CG_CTL1
        writew(0x52000a34, 0x0);    // BUS_CG_CTL2
        writew(0x52000ba4, 0x4);    // PWM_AUTO_CG_BYPASS_EN
        writew(0x52000bb0, 0x0);    // SPI_AUTO_CG_CFG
        writew(0x52000bb4, 0x0);    // SPI_SSI_CLK_AUTO_CG_CFG
#if defined(CONFIG_DRIVER_SUPPORT_DMA)
        writew(0x52000bb8, 0xF);    // DMA_AUTO_CG_CFG
#else
        writew(0x52000bb8, 0);      // DMA_AUTO_CG_CFG
#endif
}

static void pm_register_dev_resume_interface(void)
{
#if defined(CONFIG_PM_POWER_GATING_ENABLE) && (CONFIG_PM_POWER_GATING_ENABLE == 1)
#if defined(CONFIG_WATCHDOG_SUPPORT_LPM)
    uapi_pm_register_dev_ops(PM_DEV_M_WDT, (pm_dev_ops_t *)&g_dev_watchdog_register);
#endif  /* CONFIG_WDT_SUPPORT_LPM */
#if defined(CONFIG_UART_SUPPORT_LPM)
    uapi_pm_register_dev_ops(PM_DEV_M_UART, (pm_dev_ops_t *)&g_dev_uart_register);
#endif  /* CONFIG_UART_SUPPORT_LPM */
#if defined(CONFIG_CPU_TRACE_SUPPORT_LPM)
    uapi_pm_register_dev_ops(PM_DEV_M_CPU_TRACE, (pm_dev_ops_t *)&g_dev_cpu_trace_register);
#endif
#if defined (CONFIG_SFC_SUPPORT_LPM)
    uapi_pm_register_dev_ops(PM_DEV_M_CACHE, (pm_dev_ops_t *)&g_dev_cache_register);
#endif  /* CONFIG_SFC_SUPPORT_LPM */
#endif
}

static void pm_pmu_regs_config_for_slp(void)
{
    // cldo和sysldo睡眠电压改成最小，即0.7v
    writew(0x5702C08C, 0x0);
    // 睡眠时关闭cpu/bus/dma时钟
    writew(0x520003e4, 0x0);
    writew(0x57000060, 0x0);    // CLK_1M_DIV
    writew(0x57000064, 0x0);    // CLK_32K_DIV
    writew(0x570000a8, 0x182);  // AON_BUS_DIV
#ifdef CONFIG_POWER_SUPPLY_BY_LDO
    // micldo硬件控，以保证睡眠时可关闭
    writew(0x5702C204, 0x7000);
#else
#ifdef CONFIG_PM_POWER_GATING_ENABLE
    // buck_sleep_frc_on：保持sleep=0；buck掉电模式可这样配置，buck不掉电模式需soc仿真给出确切结论。
    writew(0x5702C118, 0x0);
#endif
    // buck_en_frc_on：硬件控
    writew(0x5702C110, 0x0);
#endif

    writew(0x570000F8, 0x113);  // 主频ch0时钟通道降频

#if defined(PM_REDUCE_BG_REFRESH_RATE)
    // 车上等对高低温要求高的场景不建议用，键鼠、水气表、遥控器这种可以用
    writew(0x5702C4D0, 0x20);   // ref_vbg_en_ret在睡眠状态下维持1的时间=1ms
    writew(0x5702C4D4, 0x1FF);  // ref_vbg_en_ret在睡眠状态下维持0的时间=16ms
#endif

    // RAM retention enable.
    writew(0x5702C0F8, 0x3F);
    writew(0x5702C900, 0x3F7F);

    // 调整CLDO、MICLDO、FLASHLDO使能延时手动控制值
    reg32_setbits(0x5702C200, 0x12, 0x4, 0x4);
    reg32_setbits(0x5702C204, 0x12, 0x4, 0x5);
    reg32_setbits(0x5702C208, 0x12, 0x4, 0x5);

    // EFUSE disable autoread.
    writew(0x5702C99C, 0x0);
}

void uapi_pm_lpc_init(void)
{
    pm_sleep_funcs_t funcs = {
        .start_tickless            = pm_port_start_tickless,
        .stop_tickless             = pm_port_stop_tickless,
        .get_sleep_ms              = pm_port_get_sleep_ms,
        .start_wakeup_timer        = pm_port_start_wakeup_timer,
        .allow_deepsleep           = pm_port_allow_deepsleep,
        .lightsleep_config         = pm_port_lightsleep_config,
        .deepsleep_config          = pm_port_deepsleep_config,
        .light_wakeup_config       = pm_port_light_wakeup_config,
        .deep_wakeup_config        = pm_port_deep_wakeup_config,
        .enter_wfi                 = pm_port_enter_wfi,
#if defined(CONFIG_PM_POWER_GATING_ENABLE) && (CONFIG_PM_POWER_GATING_ENABLE == 1)
        .cpu_suspend               = pm_port_cpu_suspend,
        .cpu_resume                = pm_port_cpu_resume,
#endif
    };
    uapi_pm_register_sleep_funcs(&funcs);

    uapi_pm_veto_init();
    pm_wakeup_rtc_init();

    pm_wakeup_wait_time_config();
    pm_sleep_wait_time_config();
    pm_auto_cg_config();
    pm_port_sleep_config_int();
    pm_pmu_regs_config_for_slp();
    pm_register_dev_resume_interface();
#if (PM_ULP_GPIO_WKUP_ENABLE == YES)
    ulp_gpio_init();
    ulp_gpio_int_wkup_config(g_wk_cfg, sizeof(g_wk_cfg) / sizeof(ulp_gpio_int_wkup_cfg_t));
#endif
    uapi_pm_add_sleep_veto(PM_VETO_ID_MCU);
#ifdef CONFIG_ULTRA_DEEP_SLEEP_ENABLE
    pm_enable_ultra_deep_sleep_mode(false);
#endif

    pm_protocol_wakeup_time_init();
}

#if defined(PM_MCPU_MIPS_STATISTICS_ENABLE) && (PM_MCPU_MIPS_STATISTICS_ENABLE == YES)
void pm_record_time_before_sleep(void)
{
    g_pm_time_before_sleep = uapi_tcxo_get_us();
    g_pm_total_work_time += (uint32_t)(g_pm_time_before_sleep - g_pm_time_after_sleep);
}

void pm_record_time_after_sleep(void)
{
    g_pm_time_after_sleep = uapi_tcxo_get_us();
    g_pm_total_idle_time += (uint32_t)(g_pm_time_after_sleep - g_pm_time_before_sleep);
}

uint64_t pm_get_time_before_sleep(void)
{
    return g_pm_time_before_sleep;
}

uint32_t pm_get_total_work_time(void)
{
    return g_pm_total_work_time;
}

uint64_t pm_get_time_after_sleep(void)
{
    return g_pm_time_after_sleep;
}

uint32_t pm_get_total_idle_time(void)
{
    return g_pm_total_idle_time;
}
#endif

void watchdog_porting_pmu_reboot(void)
{
    uint16_t i = 0;
    uint16_t stat = 0;
    if (reg32_getbit(ULP_RST_RECORD, ULP_PIN_RST_BIT) == true) {
        set_system_boot_status(REBOOT_ULP_PIN_RST_TRIGER_STATUS);
        set_cpu_utils_reset_cause(REBOOT_CAUSE_APPLICATION_STD_ULP_PIN_FRST);
        set_update_reset_cause_on_boot(false);
    } else if (reg32_getbit(ULP_RST_RECORD, ULP_WDT_RST_BIT) == true) {
        set_system_boot_status(REBOOT_ULP_WDGTIMEOUT_TRIGER_STATUS);
        set_cpu_utils_reset_cause(REBOOT_CAUSE_APPLICATION_STD_ULP_WDT_FRST);
        set_update_reset_cause_on_boot(false);
    }
    do {
        reg32_setbit(ULP_RST_RECORD, ULP_RST_BIT_CLR);
        stat = reg32_getbits(ULP_RST_RECORD, ULP_WDT_RST_BIT, ULP_RST_BIT_LEN);
        i++;
    } while ((stat != 0) && (i < UINT16_MAX));
    if (stat != 0) {
        PRINT("ULP_WDT_RST clear fail = %x\r\n", readl(ULP_RST_RECORD));
    }
}

#if defined(PM_SUPPORT_USB_WKUP)
void pm_usb_suspend_cbk(void)
{
    if (g_pm_usb_suspend_cbk == NULL) {
        return;
    }
    g_pm_usb_suspend_cbk();
}

void pm_usb_resume_cbk(void)
{
    if (g_pm_usb_resume_cbk == NULL) {
        return;
    }
    g_pm_usb_resume_cbk();
}

void pm_usb_register_suspend_cbk(pm_usb_cbk_t suspend)
{
    g_pm_usb_suspend_cbk = suspend;
}

void pm_usb_register_resume_cbk(pm_usb_cbk_t resume)
{
    g_pm_usb_resume_cbk = resume;
}

int pm_usb_wkup_host(void)
{
    return dwc_otg_remote_wakeup();
}
#endif

void pm_gpio_state_print(void)
{
    osal_printk("<io, mode, dir(0:input,1:output), pull(0:none,1:up,2:down), level(0:low,1:high)>:\n");
    for (int i = 0; i < PIN_MAX_NUMBER; i++) {
        osal_printk("%d  %d  %d  %d  %d\n", i, uapi_pin_get_mode((pin_t)i), uapi_gpio_get_dir((pin_t)i),
            uapi_pin_get_pull((pin_t)i), uapi_gpio_get_output_val((pin_t)i));
    }
}

void pm_gpio_group_config(gpio_info_cfg_t *cfg, uint8_t array_num)
{
    for (int i = 0; i < array_num; i++) {
        if (cfg[i].pin >= PIN_MAX_NUMBER) {
            continue;
        }
        uapi_gpio_set_dir((pin_t)cfg[i].pin, cfg[i].dir);
        uapi_pin_set_pull((pin_t)cfg[i].pin, cfg[i].pull_type);
        uapi_gpio_set_val((pin_t)cfg[i].pin, cfg[i].level);
    }
}

bool pm_is_xo_32k_enabled(void)
{
#ifdef CONFIG_XO_32K_ENABLE
    return true;
#else
    return false;
#endif
}