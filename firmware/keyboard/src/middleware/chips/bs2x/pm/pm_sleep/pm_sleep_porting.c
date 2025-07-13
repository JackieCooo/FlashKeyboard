/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides pm sleep port \n
 *
 * History: \n
 * 2023-01-13， Create file. \n
 */

#include "chip_io.h"
#include "chip_core_irq.h"
#include "platform_core.h"
#include "osal_interrupt.h"
#include "los_task_pri.h"
#include "non_os.h"
#include "arch_barrier.h"
#include "arch_encoding.h"
#include "debug_print.h"
#include "ulp_rtc.h"
#include "tcxo.h"
#include "tcxo_porting.h"
#include "pinctrl.h"
#include "gpio.h"
#include "timer.h"
#include "rtc.h"
#include "sfc.h"
#include "patch.h"
#ifdef CONFIG_ULTRA_DEEP_SLEEP_ENABLE
#if defined(CONFIG_KEYSCAN_SUPPORT_LPM)
#include "keyscan.h"
#endif
#endif
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
#include "ulp_gpio.h"
#include "watchdog.h"
#include "watchdog_porting.h"
#endif
#include "pm_porting.h"
#ifdef CONFIG_ULTRA_DEEP_SLEEP_ENABLE
#include "hal_reboot.h"
#endif
#include "pm_sleep_porting.h"

#define ULP_AON_CTL_MCPU_POR_RST_PC_H_REG       (ULP_AON_CTL_RB_ADDR + 0xF0)
#define ULP_AON_CTL_MCPU_POR_RST_PC_L_REG       (ULP_AON_CTL_RB_ADDR + 0xF4)

#define PMU1_CTL_LPM_MCU_ALW_TO_SLP_REG         (PMU1_CTL_RB_BASE + 0x200)

/* Ulp sleep event. */
#define ULP_AON_CTL_ULP_SLP_EVT_STS_REG         (ULP_AON_CTL_RB_ADDR + 0x1C8)
#define ULP_AON_CTL_ULP_SLP_EVT_CLR_REG         (ULP_AON_CTL_RB_ADDR + 0x1CC)
#define ULP_AON_CTL_ULP_SLP_EVT_EN_REG          (ULP_AON_CTL_RB_ADDR + 0x1D0)

/* Ulp sleep interrupt. */
#define ULP_AON_CTL_ULP_SLP_INT_STS_REG         (ULP_AON_CTL_RB_ADDR + 0x1C0)
#define ULP_AON_CTL_ULP_SLP_INT_CLR_REG         (ULP_AON_CTL_RB_ADDR + 0x1C4)
#define ULP_AON_CTL_ULP_SLP_INT_EN_REG          (ULP_AON_CTL_RB_ADDR + 0x1D4)

/* Ulp wakeup interrupt. */
#define ULP_AON_CTL_ULP_WKUP_INT_STS_REG        (ULP_AON_CTL_RB_ADDR + 0x1E0)
#define ULP_AON_CTL_ULP_WKUP_INT_CLR_REG        (ULP_AON_CTL_RB_ADDR + 0x1E4)
#define ULP_AON_CTL_ULP_WKUP_INT_EN_REG         (ULP_AON_CTL_RB_ADDR + 0x1F8)

/* Ulp wakeup event. */
#define ULP_AON_CTL_ULP_WKUP_EVT_STS_REG        (ULP_AON_CTL_RB_ADDR + 0x1E8)
#define ULP_AON_CTL_ULP_WKUP_EVT_CLR_REG        (ULP_AON_CTL_RB_ADDR + 0x1EC)
#define ULP_AON_CTL_ULP_WKUP_EVT_EN_REG         (ULP_AON_CTL_RB_ADDR + 0x1F0)

#define PM_NFC_FIELD_DET_WAKEUP                 4
#define PM_ULP_OSC_EN_WAKEUP                    3
#define PM_ULP_RTC_WAKEUP                       2
#define PM_ULP_GPIO_WAKEUP                      1
#define PM_AON_WKUP_ULP_WAKEUP                  0
#define PM_ULP_WKUP_ALL_MASK                    0x1F
#if defined(CONFIG_SUPPORT_NFC_SERVICE)
#define PM_ULP_WKUP_MASK                        (BIT(PM_NFC_FIELD_DET_WAKEUP) | \
                                                BIT(PM_ULP_OSC_EN_WAKEUP) | \
                                                BIT(PM_ULP_GPIO_WAKEUP) | \
                                                BIT(PM_AON_WKUP_ULP_WAKEUP))
#else
#define PM_ULP_WKUP_MASK                         (BIT(PM_ULP_OSC_EN_WAKEUP) | \
                                                  BIT(PM_ULP_GPIO_WAKEUP) | \
                                                  BIT(PM_AON_WKUP_ULP_WAKEUP))
#endif

/* Wakeup event. */
#define PMU1_CTL_LPM_MCPU_WKUP_EVT_CLR_REG      (PMU1_CTL_RB_BASE + 0x210)
#define PMU1_CTL_LPM_MCPU_WKUP_EVT_EN_REG       (PMU1_CTL_RB_BASE + 0x214)
#define PMU1_CTL_LPM_MCPU_WKUP_EVT_STS_REG      (PMU1_CTL_RB_BASE + 0x218)
/* Wakeup interrupt. */
#define PMU1_CTL_LPM_MCPU_WKUP_INT_CLR_REG      (PMU1_CTL_RB_BASE + 0x220)
#define PMU1_CTL_LPM_MCPU_WKUP_INT_EN_REG       (PMU1_CTL_RB_BASE + 0x224)
#define PMU1_CTL_LPM_MCPU_WKUP_INT_STS_REG      (PMU1_CTL_RB_BASE + 0x228)

#define PM_LPM_MCPU_CWDT_INT_WAKEUP             14
#define PM_LPM_MCPU_ULP_INT_WAKEUP              13
#define PM_LPM_MCPU_BT_OSC_EN_WAKEUP            12
#define PM_LPM_MCPU_DAP_WAKEUP                  11
#define PM_LPM_MCPU_SSI_WAKEUP                  10
#define PM_LPM_MCPU_GPIO_WAKEUP                 9
#define PM_LPM_MCPU_M_RTC_WAKEUP                8
#define PM_LPM_MCPU_SPI1_INT_WAKEUP             7
#define PM_LPM_MCPU_UART_L1_RX_WAKEUP           6
#define PM_LPM_MCPU_UART_H0_RX_WAKEUP           5
#define PM_LPM_MCPU_UART_L0_RX_WAKEUP           4
#define PM_LPM_MCPU_SPI2_INT_WAKEUP             3
#define PM_LPM_MCPU_QDEC_INT_WAKEUP             2
#define PM_LPM_MCPU_KEYSCAN_INT_WAKEUP          0
#define PM_LPM_MCPU_WKUP_ALL_MASK               0xFFFF
#define PM_LPM_MCPU_WKUP_MASK                   (BIT(PM_LPM_MCPU_CWDT_INT_WAKEUP) | \
                                                BIT(PM_LPM_MCPU_ULP_INT_WAKEUP) | \
                                                BIT(PM_LPM_MCPU_BT_OSC_EN_WAKEUP) | \
                                                BIT(PM_LPM_MCPU_M_RTC_WAKEUP))

/* Sleep event. */
#define PMU1_CTL_LPM_MCPU_SLP_EVT_CLR_REG       (PMU1_CTL_RB_BASE + 0x240)
#define PMU1_CTL_LPM_MCPU_SLP_EVT_EN_REG        (PMU1_CTL_RB_BASE + 0x244)
#define PMU1_CTL_LPM_MCPU_SLP_EVT_STS_REG       (PMU1_CTL_RB_BASE + 0x248)
/* Sleep int. */
#define PMU1_CTL_LPM_MCPU_SLP_INT_CLR_REG       (PMU1_CTL_RB_BASE + 0x250)
#define PMU1_CTL_LPM_MCPU_SLP_INT_EN_REG        (PMU1_CTL_RB_BASE + 0x254)
#define PMU1_CTL_LPM_MCPU_SLP_INT_STS_REG       (PMU1_CTL_RB_BASE + 0x258)

#define PM_LPM_MCPU_BT_OSC_EN_SLP               2
#define PM_LPM_MCPU_SLEEPING_SLP                1
#define PM_LPM_MCPU_SLEEPDEEP_SLP               0
#define PM_LPM_MCPU_SLP_ALL_MASK                0x7
#define PM_LPM_MCPU_SLP_MASK                    BIT(PM_LPM_MCPU_SLEEPING_SLP)

#define PMU1_CTL_USB_WKUP_EVT_SEL_REG           (PMU1_CTL_RB_BASE + 0x700)
#define PMU1_CTL_USB_WKUP_EVT_EN_REG            (PMU1_CTL_RB_BASE + 0x704)
#define PMU1_CTL_USB_WKUP_EVT_CLR_REG           (PMU1_CTL_RB_BASE + 0x708)
#define PMU1_CTL_USB_WKUP_EVT_STS_REG           (PMU1_CTL_RB_BASE + 0x70C)

#define PMU1_CTL_USB_WKUP_INT_EN_REG            (PMU1_CTL_RB_BASE + 0x710)
#define PMU1_CTL_USB_WKUP_INT_CLR_REG           (PMU1_CTL_RB_BASE + 0x714)
#define PMU1_CTL_USB_WKUP_INT_STS_REG           (PMU1_CTL_RB_BASE + 0x718)

#define PMU1_CTL_RST_MAN_REG                    (PMU1_CTL_RB_BASE + 0xE4)
#define PM_RST_PWR_C1_CRG_N_FRC_ON              2
#define PM_RST_PWR_C1_LOGIC_N_FRC_ON            1
#define PM_RST_PWR_C1_CPU_N_FRC_ON              0
#define PM_RST_PWR_C1_FRC_ON_MASK               (BIT(PM_RST_PWR_C1_CRG_N_FRC_ON) | \
                                                BIT(PM_RST_PWR_C1_LOGIC_N_FRC_ON) | \
                                                BIT(PM_RST_PWR_C1_CPU_N_FRC_ON))

#define ULP_AON_CTL_PMU_SYSLDO_ECO_EN_CFG_REG   (ULP_AON_CTL_RB_ADDR + 0xFC)
#define ULP_AON_CTL_PMU_REF1_IBG_EN_CFG_REG     (ULP_AON_CTL_RB_ADDR + 0x104)
#define ULP_AON_CTL_PMU_UVLO_EN_CFG_REG         (ULP_AON_CTL_RB_ADDR + 0x108)
#define ULP_AON_CTL_PMU_BUCK_EN_CFG_REG         (ULP_AON_CTL_RB_ADDR + 0x110)
#define ULP_AON_CTL_BUCK_VSET_ECO_CFG_REG       (ULP_AON_CTL_RB_ADDR + 0x114)
#define ULP_AON_CTL_VDD0P7_TO_SYS_ISO_CFG_REG   (ULP_AON_CTL_RB_ADDR + 0x130)
#define ULP_AON_CTL_RST_BOOT_32K_CFG_REG        (ULP_AON_CTL_RB_ADDR + 0x134)
#define ULP_AON_CTL_PMU_CLDO_EN_CFG_REG         (ULP_AON_CTL_RB_ADDR + 0x128)

#define ULP_AON_CTL_PAD_CONTROL_REG             (ULP_AON_CTL_RB_ADDR + 0x840)
#define PM_PAD_CONTROL_BY_ULP                   1
#define PM_PAD_CONTROL_BY_AON                   0

#define ULP_AON_CTL_ULP_GPIO_CLK_CFG_REG        (ULP_AON_CTL_RB_ADDR + 0x24)
#define PM_PCLK_INTR_SEL_BIT                    1
#define PM_PCLK_INTR_EN_BIT                     0

#define PM_OS_TICKS_PER_S   LOSCFG_BASE_CORE_TICK_PER_SECOND
#define PM_OS_TICKS_PER_MS  (PM_OS_TICKS_PER_S / LOSCFG_BASE_CORE_TICK_PER_SECOND)
static uint64_t g_entry_sleep_time = 0;
static uint64_t g_exit_sleep_time = 0;
static uint64_t g_32k_time_calibrate = 0;
uint32_t g_xo_core_trim_get = 0;
uint32_t g_xo_core_trim_rev_get = 0;
bool g_get_xo_core_vals = false;

#define PM_SLEEP_CPU_SUSPEND_REG_NUM 58
uint32_t g_cpu_suspend_regs[PM_SLEEP_CPU_SUSPEND_REG_NUM] = { 0 };

#define LS_EARLY_WKUP_MS    1
#define DS_EARLY_WKUP_MS    4

#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
static uint32_t g_cpu_reset_pc;
static uint64_t g_resume_current_count = 0;

#define PM_M_CLDEN0_REG             0x52000040
static uint16_t g_pm_m_clken0_val = 0;
#define PM_M_CLDEN1_REG             0x52000044
static uint16_t g_pm_m_clken1_val = 0;
#define PM_M_SOFT_RST_N_REG         0x52000054
static uint16_t g_pm_m_soft_rst_n_val = 0;
#define PM_M2B_H2H_SEL_REG          0x52000A40
static uint16_t g_pm_m2b_h2h_sel_val = 0;
#define PM_GLB_CLKEN_REG            0x52000548
static uint16_t g_pm_glb_clken_val = 0;
#define PM_COM_DLL2_REG             0x52000540
static uint16_t g_pm_com_dll2_val = 0;
#define PM_MCU_CORE_CR_CH1_REG      0x570000FC
static uint16_t g_pm_mcu_core_cr_ch1_val = 0;
#define PM_PERP_BUS_DIV_REG         0x5200007C
static uint16_t g_pm_perp_bus_div_val = 0;
#define PM_XO_CLK_OUT_EN_REG        0x57008410
static uint16_t g_pm_xo_clk_out_en_val = 0;
#define PM_MCU_PERP_QSPI_CR_REG     0x52000574
static uint16_t g_pm_mcu_perp_qspi_cr_val = 0;
#define PM_MCU_PERP_LS_CR_REG       0x52000554
static uint16_t g_pm_mcu_perp_ls_cr_val = 0;
#define PM_MCU_PERP_UART_CR_REG     0x52000558
static uint16_t g_pm_mcu_perp_uart_cr_val = 0;
#define PM_MCU_PERP_SPI_CR_REG      0x5200055C
static uint16_t g_pm_mcu_perp_spi_cr_val = 0;
#define PM_MSUB_NOR_CFG_REG         0x520003E0
static uint16_t g_pm_msub_nor_cfg_val = 0;
#define PM_MSUB_SLP_CFG_REG         0x520003E4
static uint16_t g_pm_msub_slp_cfg_val = 0;
#define PM_CLK_1M_DIV_REG           0x57000060
static uint16_t g_pm_clk_1m_div_val = 0;
#define PM_CLK_32K_DIV_REG          0x57000064
static uint16_t g_pm_clk_32k_div_val = 0;
#define PM_AON_BUS_DIV_REG          0x570000A8
static uint16_t g_pm_aon_bus_div_val = 0;
#define PM_M_CLKSEL_REG             0x5200004C
static uint16_t g_pm_m_clksel_val = 0;

// BT解复位相关寄存器
#define GLB_CTL_M_SOFT_RST_2        0x570000B8
#define M_CTL_DTCM_SHARE            0x52000F90
static uint16_t g_m_ctl_dtcm_share = 0;
#define B_CTL_B_CRG_SOFT_RST_N      0x5900080C
static uint16_t g_b_ctl_b_crg_soft_rst_n = 0;
#define BT_CTL_BTCRG_SOFT_RESET     0x59403050
// 通用寄存器, 平台通知BT芯片需要恢复bsoc和bt_sub相关配置
#define RF_CTL_RF_GP_REG0           0x59001010
#define BT_EM_BASE_ADDR             0x59410000

#define DTCM_SHARE_MODE           0xF90
#define SHARE_MODE_GT             12
#define SHARE_MODE_CFG            8

typedef struct reg_cfg {
    uint8_t reg_addr;
    uint8_t value;
} reg_cfg_t;

#define SLEEP_CFG0 0x57004200  // 0x57004200 + addr
static const reg_cfg_t g_sleep_wait_time_config0[] = {
    { 0xA0, 0x3 },    // AON_SLP_RDY_SLP_TIME
    { 0xA4, 0x3 },    // REF2_EN_BG_SLP_TIME_SOC
    { 0xA8, 0x3 },    // EN_REFBUFFER_SLP_TIME_SOC
    { 0xAC, 0x2 },    // EN_INTLDO2_SLP_TIME_SOC
    { 0xB0, 0x3 },    // EN_IBG_SLP_TIME_SOC
    { 0xB4, 0x3 },    // EN_IPOLY_SLP_TIME_SOC
    { 0xB8, 0x3 },    // EN_ITUNE_SLP_TIME_SOC
    { 0xC4, 0x3 },    // EN_CLKLDO1_SLP_TIME
    { 0xC8, 0x2 },    // EN_XLDO_SLP_TIME
    { 0xD0, 0x2 },    // XO_CORE_PD_SLP_TIME
    { 0xD4, 0x2 },    // FAST_XO_ISO_SLP_TIME
    { 0xD8, 0x2 },    // RC_PD_SLP_TIME
    { 0xDC, 0x2 },    // RC_RSTN_SLP_TIME
    { 0xE8, 0x2 },    // FAST_XO_LOOP_RSTN_SLP_TIME
    { 0xEC, 0x2 },    // XO2DBB_CLKOUT_EN_SLP_TIME
    { 0xF8, 0x2 },    // RC2_DBB_PD_SLP_TIME
    { 0xFC, 0x2 },    // RCCLK2_EN_SLP_TIME
};

#define SLEEP_CFG1 0x57004300  // 0x57004300 + addr
static const reg_cfg_t g_sleep_wait_time_config1[] = {
    { 0x00, 0x2 },    // RC2_CLKEN_SLP_TIME
    { 0x04, 0x2 },    // B32M_CLKEN_SLP_TIME
    { 0x08, 0x2 },    // A32M_CLKEN_SLP_TIME
    { 0x0C, 0x2 },    // TCXO_EN_SLP_TIME
    { 0x10, 0x1 },    // APERP_32K_SEL_SLP_TIME
    { 0x14, 0x1 },    // RST_PWR_C1_CRG_N_SLP_TIME
    { 0x18, 0x1 },    // RST_PWR_C1_LGC_N_SLP_TIME
    { 0x1C, 0x1 },    // RST_PWR_C1_CPU_N_SLP_TIME
};

static void pm_sleep_wait_time_config(void)
{
    for (uint32_t i = 0; i < sizeof(g_sleep_wait_time_config0) / sizeof(reg_cfg_t); i++) {
        writew(SLEEP_CFG0 + g_sleep_wait_time_config0[i].reg_addr, g_sleep_wait_time_config0[i].value);
    }
    for (uint32_t i = 0; i < sizeof(g_sleep_wait_time_config1) / sizeof(reg_cfg_t); i++) {
        writew(SLEEP_CFG1 + g_sleep_wait_time_config1[i].reg_addr, g_sleep_wait_time_config1[i].value);
    }
}

#else
static rtc_handle_t g_pm_rtc = NULL;

static void pm_rtc1_irq(uintptr_t data)
{
    unused(data);
}
#endif

static void pm_clear_slp_wkup_event(void)
{
    writew(PMU1_CTL_LPM_MCPU_SLP_EVT_EN_REG, 0);                                // Disable.
    do {
        writew(PMU1_CTL_LPM_MCPU_SLP_EVT_CLR_REG, PM_LPM_MCPU_SLP_ALL_MASK);    // Clear status.
    } while (readw(PMU1_CTL_LPM_MCPU_SLP_EVT_STS_REG) != 0);                    // Get status.

    writew(PMU1_CTL_LPM_MCPU_WKUP_EVT_EN_REG, 0);                               // Disable.
    do {
        writew(PMU1_CTL_LPM_MCPU_WKUP_EVT_CLR_REG, PM_LPM_MCPU_WKUP_ALL_MASK);  // Clear status.
    } while (readw(PMU1_CTL_LPM_MCPU_WKUP_EVT_STS_REG) != 0);                   // Get status.

    writew(ULP_AON_CTL_ULP_SLP_EVT_EN_REG, 0);                                // Disable.
    do {
        writew(ULP_AON_CTL_ULP_SLP_EVT_CLR_REG, 0x1);                         // Clear status.
    } while (readw(ULP_AON_CTL_ULP_SLP_EVT_STS_REG) != 0);                    // Get status.

    writew(ULP_AON_CTL_ULP_WKUP_EVT_EN_REG, 0);                               // Disable.
    do {
        writew(ULP_AON_CTL_ULP_WKUP_EVT_CLR_REG, PM_ULP_WKUP_ALL_MASK);  // Clear status.
    } while (readw(ULP_AON_CTL_ULP_WKUP_EVT_STS_REG) != 0);                   // Get status.
}

void pm_wakeup_rtc_init(void)
{
    ulp_rtc_init();
}

void pm_wakeup_rtc_start(uint32_t time_ms)
{
    if (time_ms <= CONFIG_PM_DEEP_SLEEP_THRESHOLD_MS) {
        ulp_rtc_start(time_ms - LS_EARLY_WKUP_MS);
    } else {
        ulp_rtc_start(time_ms - DS_EARLY_WKUP_MS);
    }
}

void pm_port_start_tickless(void)
{
    g_entry_sleep_time = 0; // ulp_rtc_get_count获取的值未重置，先按0计算即可
    os_tick_timer_disable();
}

static void pm_port_tickless_compensation(uint32_t ticks)
{
    g_exit_sleep_time = ulp_rtc_get_count();
    uint64_t actual_sleep_ms = ((g_exit_sleep_time - g_entry_sleep_time) * PM_OS_TICKS_PER_S) +
                                 g_32k_time_calibrate;
    g_32k_time_calibrate = actual_sleep_ms & 0x7FFF;
    actual_sleep_ms = actual_sleep_ms >> 0xF;

    if (actual_sleep_ms > ticks) {
        oal_ticks_restore(ticks);
    } else {
        oal_ticks_restore(actual_sleep_ms);
    }

#if defined(PM_SLEEP_DEBUG_ENABLE) && (PM_SLEEP_DEBUG_ENABLE == YES)
    PRINT("ulp-wkup evt: 0x%x\r\n", readw(ULP_AON_CTL_ULP_WKUP_EVT_STS_REG));
    PRINT("aon-wkup evt: 0x%x\r\n", readw(PMU1_CTL_LPM_MCPU_WKUP_EVT_STS_REG));
#endif
    pm_clear_slp_wkup_event();
}

void pm_port_stop_tickless(uint32_t sleep_ms)
{
    os_tick_timer_enable();
    /* Avoid overcompensation. */
    pm_port_tickless_compensation(sleep_ms - 1);
}

#ifdef CONFIG_SUPPORT_CLOSE_ULP_WDT_DURING_SLP
static bool g_pm_ulp_wdt_closed_flag = false;

void pm_close_ulp_wdt_during_slp(bool close)
{
    g_pm_ulp_wdt_closed_flag = close;
}
#endif

// ulp wdt超时时间：32k时钟有误差，留10%余量喂狗。
static uint32_t g_pm_ulp_wdt_wkup_time[] = { 9000, 36000, 72000, 144000 };
static uint32_t pm_get_ulp_wdt_timeout_ms(void)
{
#ifdef CONFIG_SUPPORT_CLOSE_ULP_WDT_DURING_SLP
    if (g_pm_ulp_wdt_closed_flag) {
        return 0xFFFFFFFF;
    }
#endif
    return ((readw(0x5702C930) == 0) ? 0xFFFFFFFF : g_pm_ulp_wdt_wkup_time[readw(0x5702C938) & 0x3]);
}

#ifdef CONFIG_ULTRA_DEEP_SLEEP_ENABLE
static void pm_force_the_system_to_ultra_deep_sleep(void);
#endif

uint32_t pm_port_get_sleep_ms(void)
{
#ifdef CONFIG_ULTRA_DEEP_SLEEP_ENABLE
    if (pm_is_ultra_deep_sleep_mode()) {
        pm_force_the_system_to_ultra_deep_sleep();
        return 0;
    }
#endif

    uint32_t slp_ms = oal_get_sleep_ticks() / PM_OS_TICKS_PER_MS;
    uint32_t wdt_ms = pm_get_ulp_wdt_timeout_ms();
    slp_ms = min(slp_ms, wdt_ms);
    return slp_ms;
}

void pm_port_allow_deepsleep(bool allow)
{
    if (allow) {
#ifdef CONFIG_SUPPORT_CLOSE_ULP_WDT_DURING_SLP
        if (g_pm_ulp_wdt_closed_flag) { // 关ulp_wdt & 降低rc32k驱动
            writew(0x5702C930, 0x0);
            writew(0x5702C4C0, 0x101);
        }
#endif
        // config gm from 4 to 9.75 with 3 steps
        reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                      XO_CORE_TRIM_CTL_STEP1_VAL);
        reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                      XO_CORE_TRIM_CTL_DEFAULT_VAL);
        reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL4_REG, XO_CORE_TRIM_REV_CTL, XO_CORE_TRIM_REV_CTL_LEN,
                      XO_CORE_TRIM_REV_CTL_DEFAULT_VAL);
    }
    writew(PMU1_CTL_LPM_MCU_ALW_TO_SLP_REG, (uint16_t)allow);
    if (!allow) {
        // config gm from 9.75 to 4 with 3 steps
        reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL4_REG, XO_CORE_TRIM_REV_CTL, XO_CORE_TRIM_REV_CTL_LEN,
                      XO_CORE_TRIM_REV_CTL_STEP1_VAL);
        reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                      XO_CORE_TRIM_CTL_STEP1_VAL);
        reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                      XO_CORE_TRIM_CTL_STEP2_VAL);
#ifdef CONFIG_SUPPORT_CLOSE_ULP_WDT_DURING_SLP
        if (g_pm_ulp_wdt_closed_flag) { // 恢复rc32k驱动 & 开ulp_wdt并喂狗
            writew(0x5702C4C0, 0x1);
            writew(0x5702C930, 0x1);
            writew(0x5702C934, 0xC5);
        }
#endif
    }
}

void pm_port_enter_wfi(void)
{
#if defined(CONFIG_REDUCE_FREQ_DURING_WFI)
    uint16_t allow_to_slp = readw(PMU1_CTL_LPM_MCU_ALW_TO_SLP_REG);
    if (allow_to_slp == 0) {
        isb();                          // 保证前面指令执行完毕
        reg16_clrbit(0x52000548, 0x7);  // 关闭SFC时钟
        writew(0x57000100, 0x0);        // 主频时钟切到0通道（降频）
        writew(0x570000a8, 0x188);      // AON_BUS降频
    }
#endif
    dsb();
    wfi();
    isb();
    nop();
    nop();
    nop();
    nop();
#if defined(CONFIG_REDUCE_FREQ_DURING_WFI)
    if (allow_to_slp == 0) {
        writew(0x570000a8, 0x182);      // AON_BUS升频
        writew(0x57000100, 0x1);        // 主频时钟切到1通道（恢复）
        reg16_setbit(0x52000548, 0x7);  // 打开SFC时钟
        isb();                          // 保证前面指令执行完毕
    }
#endif
}

void pm_port_start_wakeup_timer(uint32_t sleep_ms)
{
    pm_wakeup_rtc_start(sleep_ms);
}

static void pm_ulp_gpio_int_sample_clk_sel_32k(bool enable)
{
    if (enable) {
        /* ULP_GPIO interrupt sampling clock: 32k. */
        reg16_setbit(ULP_AON_CTL_ULP_GPIO_CLK_CFG_REG, PM_PCLK_INTR_SEL_BIT);
    } else {
        /* ULP_GPIO interrupt sampling clock: pclk. */
        reg16_clrbit(ULP_AON_CTL_ULP_GPIO_CLK_CFG_REG, (uint16_t)PM_PCLK_INTR_SEL_BIT);
    }
}

static void pm_port_handle_before_enter_sleep(void)
{
    // Enable slp/wkup event.
    writew(PMU1_CTL_LPM_MCPU_SLP_EVT_CLR_REG, PM_LPM_MCPU_SLP_ALL_MASK);      // Clear status.
    writew(PMU1_CTL_LPM_MCPU_SLP_EVT_EN_REG, PM_LPM_MCPU_SLP_ALL_MASK);           // Enable.
    writew(PMU1_CTL_LPM_MCPU_WKUP_EVT_CLR_REG, PM_LPM_MCPU_WKUP_ALL_MASK);  // Clear status.
    writew(PMU1_CTL_LPM_MCPU_WKUP_EVT_EN_REG, PM_LPM_MCPU_WKUP_MASK);       // Enable.

#if defined(CONFIG_PM_ENABLE_WAKEUP_INTERRUPT)
    // Clear wkup int.
    writew(ULP_AON_CTL_ULP_WKUP_INT_CLR_REG, 0x17);
#endif

    // Enable ulp slp/wkup event.
    writew(ULP_AON_CTL_ULP_SLP_EVT_CLR_REG, 0x1); // Clear status.
    writew(ULP_AON_CTL_ULP_SLP_EVT_EN_REG, 0x1);  // Enable.
    writew(ULP_AON_CTL_ULP_WKUP_EVT_CLR_REG, PM_ULP_WKUP_ALL_MASK); // Clear status.
    writew(ULP_AON_CTL_ULP_WKUP_EVT_EN_REG, PM_ULP_WKUP_ALL_MASK);      // Enable.
}

static void pm_config_based_on_cldo_state(bool on)
{
    if (on) {
        /* Turn on cldo during sleep. */
        writew(PMU1_CTL_RST_MAN_REG, PM_RST_PWR_C1_FRC_ON_MASK);
        writew(ULP_AON_CTL_RST_BOOT_32K_CFG_REG, 0x1);
        writew(ULP_AON_CTL_PMU_CLDO_EN_CFG_REG, 0x1);
        writew(ULP_AON_CTL_PMU_REF1_IBG_EN_CFG_REG, 0x1);
        writew(ULP_AON_CTL_PMU_BUCK_EN_CFG_REG, 0x1);
        writew(ULP_AON_CTL_PMU_SYSLDO_ECO_EN_CFG_REG, 0x0);
        writew(ULP_AON_CTL_VDD0P7_TO_SYS_ISO_CFG_REG, 0);
        writew(ULP_AON_CTL_BUCK_VSET_ECO_CFG_REG, 0x1); // buck电压硬件控; 睡眠时间短的话这里要改成保持vset_nor
        writew(ULP_AON_CTL_PMU_UVLO_EN_CFG_REG, 0x1); // uvlo强制打开
    } else {
        /* Turn off cldo during sleep. */
        writew(PMU1_CTL_RST_MAN_REG, 0);
        writew(ULP_AON_CTL_RST_BOOT_32K_CFG_REG, 0);
        writew(ULP_AON_CTL_PMU_CLDO_EN_CFG_REG, 0);
        writew(ULP_AON_CTL_PMU_REF1_IBG_EN_CFG_REG, 0);
        writew(ULP_AON_CTL_PMU_BUCK_EN_CFG_REG, 0);
        writew(ULP_AON_CTL_PMU_SYSLDO_ECO_EN_CFG_REG, 0x1);
        writew(ULP_AON_CTL_VDD0P7_TO_SYS_ISO_CFG_REG, 0x1);
        writew(ULP_AON_CTL_BUCK_VSET_ECO_CFG_REG, 0x1);
        writew(ULP_AON_CTL_PMU_UVLO_EN_CFG_REG, 0x0); // uvlo硬件控
    }
}

void pm_port_lightsleep_config(void)
{
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
#if defined(PM_SUPPORT_LIGHT_SLEEP)
    pm_port_handle_before_enter_sleep();

    pm_config_based_on_cldo_state(true);

    pm_ulp_gpio_int_sample_clk_sel_32k(true);
    pm_port_allow_deepsleep(true);
#endif
#endif
    return;
}

void pm_port_light_wakeup_config(void)
{
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
#if defined(PM_SUPPORT_LIGHT_SLEEP)
    pm_port_allow_deepsleep(false);
    pm_ulp_gpio_int_sample_clk_sel_32k(false);
#endif
#endif
    return;
}

void pm_port_sleep_config_int(void)
{
#if defined(CONFIG_PM_ENABLE_WAKEUP_INTERRUPT)
    /* Aon wakeup interrupt. */
    writew(PMU1_CTL_LPM_MCPU_WKUP_INT_EN_REG, 0);                           // Disable.
    writew(PMU1_CTL_LPM_MCPU_WKUP_INT_CLR_REG, PM_LPM_MCPU_WKUP_ALL_MASK);  // Clear status.

    writew(ULP_AON_CTL_ULP_WKUP_INT_EN_REG, 0);                     // Disable.
    writew(ULP_AON_CTL_ULP_WKUP_INT_CLR_REG, PM_ULP_WKUP_ALL_MASK);     // Clear status.
    writew(ULP_AON_CTL_ULP_WKUP_INT_EN_REG, PM_ULP_WKUP_MASK);      // Enable.
#endif
    /* Ulp sleep event. */
    writew(ULP_AON_CTL_ULP_SLP_EVT_EN_REG, 0);  // Disable.
    writew(ULP_AON_CTL_ULP_SLP_EVT_CLR_REG, 1); // Clear status.
    writew(ULP_AON_CTL_ULP_SLP_EVT_EN_REG, 1);  // Enable.
    /* Ulp wakeup event. */
    writew(ULP_AON_CTL_ULP_WKUP_EVT_EN_REG, 0);                     // Disable.
    writew(ULP_AON_CTL_ULP_WKUP_EVT_CLR_REG, PM_ULP_WKUP_ALL_MASK); // Clear status.
    writew(ULP_AON_CTL_ULP_WKUP_EVT_EN_REG, PM_ULP_WKUP_ALL_MASK);      // Enable.
}

#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
static void pm_adc_ldo_check_off(void)
{
    if (reg16_getbit(0x5702C230, 0xA) == 0) {
        return;
    }
    writel(0x570363D4, 0x0);        // afe_vrefldo
    writel(0x570363E4, 0x0);        // afe_adcldo
    writel(0x570363E8, 0x1040);     // afe_adcldo
    writel(0x570363EC, 0x0);        // afe_afeldo
    writel(0x570363E8, 0x5040);     // afe_afeldo
    writel(0x57008708, 0x4);        // afe_gadc_cfg
    reg32_clrbit(0x57036000, 0x10); // cfg_rstn

    reg16_clrbit(0x5702C230, 0xA);  // afe iso
}
#endif

void pm_port_deepsleep_config(void)
{
    pm_sleep_wait_time_config();
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
    pm_adc_ldo_check_off();
    pm_config_based_on_cldo_state(false);
#endif
    pm_port_handle_before_enter_sleep();
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
    pm_aon_gpio_suspend();
#if defined(CONFIG_DISABLE_GPIO_INTERRUPT_DURING_PAD_SWITCHING)
    ulp_gpio_int_disable();
    /* Pad control by ulp. */
    writew(ULP_AON_CTL_PAD_CONTROL_REG, PM_PAD_CONTROL_BY_ULP);
    ulp_gpio_int_enable();
#else
    /* Pad control by ulp. */
    writew(ULP_AON_CTL_PAD_CONTROL_REG, PM_PAD_CONTROL_BY_ULP);
#endif
    // 修复低概率唤醒BUG：使用m_rtc3辅助唤醒。
    SET_CUSTOM_CSR(LOCIEN0, 0x100000);  // enable m_rtc3
    dsb();
    writew(0x57024410, 0x0);    // disable
    writew(0x57024400, 0x60);   // 3ms wkup
    writew(0x57024410, 0x1);    // enable
#else
    pm_config_based_on_cldo_state(true);
#endif
    pm_ulp_gpio_int_sample_clk_sel_32k(true);
}

void pm_port_deep_wakeup_config(void)
{
    pm_ulp_gpio_int_sample_clk_sel_32k(false);
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
    writew(0x57024410, 0x0);    // disable
    writew(0x57024414, 0x1);    // clear int
    WRITE_CUSTOM_CSR_VAL(0xBF0, 0x34);  // clear pending
    dsb();
    /* Pad control by aon. */
    pm_aon_gpio_resume();
    writew(ULP_AON_CTL_PAD_CONTROL_REG, PM_PAD_CONTROL_BY_AON);
    pm_clr_ulp_gpio();
#endif
}

#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
static void pm_suspend_regs(void)
{
    g_pm_m_clken0_val = readw(PM_M_CLDEN0_REG);
    g_pm_m_clken1_val = readw(PM_M_CLDEN1_REG);
    g_pm_m_soft_rst_n_val = readw(PM_M_SOFT_RST_N_REG);
    g_pm_m2b_h2h_sel_val = readw(PM_M2B_H2H_SEL_REG);
    g_pm_glb_clken_val = readw(PM_GLB_CLKEN_REG);
    g_pm_com_dll2_val = readw(PM_COM_DLL2_REG);
    g_pm_mcu_core_cr_ch1_val = readw(PM_MCU_CORE_CR_CH1_REG);
    g_pm_perp_bus_div_val = readw(PM_PERP_BUS_DIV_REG);
    g_pm_xo_clk_out_en_val = readw(PM_XO_CLK_OUT_EN_REG);
    g_pm_mcu_perp_qspi_cr_val = readw(PM_MCU_PERP_QSPI_CR_REG);
    g_pm_mcu_perp_ls_cr_val = readw(PM_MCU_PERP_LS_CR_REG);
    g_pm_mcu_perp_uart_cr_val = readw(PM_MCU_PERP_UART_CR_REG);
    g_pm_mcu_perp_spi_cr_val = readw(PM_MCU_PERP_SPI_CR_REG);
    g_pm_msub_nor_cfg_val = readw(PM_MSUB_NOR_CFG_REG);
    g_pm_msub_slp_cfg_val = readw(PM_MSUB_SLP_CFG_REG);
    g_pm_clk_1m_div_val = readw(PM_CLK_1M_DIV_REG);
    g_pm_clk_32k_div_val = readw(PM_CLK_32K_DIV_REG);
    g_pm_aon_bus_div_val = readw(PM_AON_BUS_DIV_REG);
    g_pm_m_clksel_val = readw(PM_M_CLKSEL_REG);
    g_m_ctl_dtcm_share = readw(M_CTL_DTCM_SHARE);
    g_b_ctl_b_crg_soft_rst_n = readw(B_CTL_B_CRG_SOFT_RST_N);
}

static void pm_suspend_device(void)
{
#if defined (CONFIG_SFC_SUPPORT_LPM)
    uapi_sfc_suspend(1);
#endif
#if defined(CONFIG_TCXO_SUPPORT_LPM)
    uapi_tcxo_suspend(0);
#endif
#if defined(CONFIG_TIMER_SUPPORT_LPM)
    uapi_timer_suspend(0);
#endif
}
#endif

void pm_port_cpu_suspend(void)
{
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
    pm_suspend_regs();
    pm_suspend_device();
    // pc = pc_h << 16 + pc_l
    g_cpu_reset_pc = (readl(ULP_AON_CTL_MCPU_POR_RST_PC_H_REG) << 16) | (readl(ULP_AON_CTL_MCPU_POR_RST_PC_L_REG));
    // pc_h: pc >> 16
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_H_REG, (uint16_t)((uint32_t)(uintptr_t)lowpower_cpu_resume >> 16));
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_L_REG, (uint16_t)((uint32_t)(uintptr_t)lowpower_cpu_resume));
    lowpower_cpu_suspend(); // Must be placed at the end of this function
#endif
}

#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
static void pm_port_timers_resume(void)
{
#if defined(CONFIG_TCXO_SUPPORT_LPM)
    // 1000000: 1s = 1000000us
    uint64_t tcxo_differential_value = (g_resume_current_count * 1000000) >> 0xF;
    uapi_tcxo_resume((uintptr_t)&tcxo_differential_value);
#endif
#if defined (CONFIG_SFC_SUPPORT_LPM)
    uapi_sfc_resume(1);
#endif
}
#endif

void pm_port_cpu_resume(void)
{
#if (CONFIG_PM_POWER_GATING_ENABLE == 1)
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_H_REG, (uint16_t)((uint32_t)(uintptr_t)g_cpu_reset_pc >> 16)); // pc_h: pc >> 16
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_L_REG, (uint16_t)((uint32_t)(uintptr_t)g_cpu_reset_pc));

    writew(PM_M_CLDEN0_REG, g_pm_m_clken0_val);
    writew(PM_M_CLDEN1_REG, g_pm_m_clken1_val);
    writew(PM_M_SOFT_RST_N_REG, g_pm_m_soft_rst_n_val);
    writew(PM_M2B_H2H_SEL_REG, g_pm_m2b_h2h_sel_val);
    writew(PM_COM_DLL2_REG, g_pm_com_dll2_val);
    writew(PM_MCU_CORE_CR_CH1_REG, g_pm_mcu_core_cr_ch1_val);
    writew(0x57000100, 0x1);    // switch to ch1
    writew(0x570000F8, 0x113);  // 主频ch0时钟通道降频
    writew(PM_PERP_BUS_DIV_REG, g_pm_perp_bus_div_val);
    writew(PM_XO_CLK_OUT_EN_REG, g_pm_xo_clk_out_en_val);

    writew(PM_GLB_CLKEN_REG, 0);
    writew(PM_MCU_PERP_QSPI_CR_REG, g_pm_mcu_perp_qspi_cr_val);
    writew(PM_MCU_PERP_LS_CR_REG, g_pm_mcu_perp_ls_cr_val);
    writew(PM_MCU_PERP_UART_CR_REG, g_pm_mcu_perp_uart_cr_val);
    writew(PM_MCU_PERP_SPI_CR_REG, g_pm_mcu_perp_spi_cr_val);
    writew(PM_GLB_CLKEN_REG, g_pm_glb_clken_val);

    writew(PM_MSUB_NOR_CFG_REG, g_pm_msub_nor_cfg_val);
    writew(PM_MSUB_SLP_CFG_REG, g_pm_msub_slp_cfg_val);
    writew(PM_CLK_1M_DIV_REG, g_pm_clk_1m_div_val);
    writew(PM_CLK_32K_DIV_REG, g_pm_clk_32k_div_val);
    writew(PM_AON_BUS_DIV_REG, g_pm_aon_bus_div_val);
    writew(PM_M_CLKSEL_REG, g_pm_m_clksel_val);

    // bt解复位晚会导致BT的唤醒中断不产生, cpu唤醒起来就解复位bt_sub, 防止bt解复位被阻塞
    writew(GLB_CTL_M_SOFT_RST_2, 0x3);
    // g_m_ctl_dtcm_share bit8~9的值只有1和3两种, 1表示16K EM模式, 3表示32K EM模式
    if (((g_m_ctl_dtcm_share & 0x300) >> 8) == 0x1) {
        writew(M_CTL_DTCM_SHARE, g_m_ctl_dtcm_share & 0x10FF); // 高位配0关闭时钟
    } else {
        writew(M_CTL_DTCM_SHARE, g_m_ctl_dtcm_share & 0xFF); // 高位配0关闭时钟
    }
    writew(M_CTL_DTCM_SHARE, g_m_ctl_dtcm_share & 0x3FF); // 切换itcm模式
    writew(M_CTL_DTCM_SHARE, g_m_ctl_dtcm_share); // 打开时钟
    writew(B_CTL_B_CRG_SOFT_RST_N, g_b_ctl_b_crg_soft_rst_n);
    writew(BT_CTL_BTCRG_SOFT_RESET, 0);
    writew(BT_CTL_BTCRG_SOFT_RESET, 0xFFFF);
    // 深睡唤醒读一次EM打开时钟
    readl(BT_EM_BASE_ADDR);
    writew(RF_CTL_RF_GP_REG0, 1); // 用于通知BT恢复bsoc和btsub相关配置
    pm_auto_cg_config();
    g_resume_current_count = ulp_rtc_get_count();
    pm_port_timers_resume();
    func_patch_init();
    /* 配置EFUSE0为读模式。 */
    uapi_reg_write32(0x57028030, 0x5a5a);
#endif

    return;
}

uint16_t pm_port_get_sleep_event_status(void)
{
    return readw(PMU1_CTL_LPM_MCPU_SLP_EVT_STS_REG);
}

uint16_t pm_port_get_wakeup_event_status(void)
{
    return readw(ULP_AON_CTL_ULP_WKUP_EVT_STS_REG);
}

#ifdef CONFIG_ULTRA_DEEP_SLEEP_ENABLE
static bool g_pm_ds_flag = false;
static bool g_pm_ultra_deep_sleep_mode = false;

bool pm_is_ultra_deep_sleep_mode(void)
{
    return g_pm_ultra_deep_sleep_mode;
}

void pm_enable_ultra_deep_sleep_mode(bool en)
{
    g_pm_ultra_deep_sleep_mode = en;
}

static void pm_uds_suspend_config(bool en)
{
    if (en) {
#if defined(CONFIG_KEYSCAN_SUPPORT_LPM)
        uapi_keyscan_suspend(0);
#endif
#if defined (CONFIG_SFC_SUPPORT_LPM)
        uapi_sfc_suspend(1);
#endif
#if defined(CONFIG_TCXO_SUPPORT_LPM)
        uapi_tcxo_suspend(0);
#endif
        pm_aon_gpio_suspend();
        /* Pad control by ulp. */
        writew(ULP_AON_CTL_PAD_CONTROL_REG, PM_PAD_CONTROL_BY_ULP);
        pm_ulp_gpio_int_sample_clk_sel_32k(true);
    } else {
        /* 配置EFUSE0为读模式。 */
        uapi_reg_write32(0x57028030, 0x5a5a);
        g_pm_ds_flag = false;
        pm_ulp_gpio_int_sample_clk_sel_32k(false);
        /* Pad control by aon. */
        pm_aon_gpio_resume();
        writew(ULP_AON_CTL_PAD_CONTROL_REG, PM_PAD_CONTROL_BY_AON);
        pm_clr_ulp_gpio();

#if defined(CONFIG_TCXO_SUPPORT_LPM)
        uint64_t tcxo_differential_value = 0;
        uapi_tcxo_resume((uintptr_t)&tcxo_differential_value);
#endif

#if defined (CONFIG_SFC_SUPPORT_LPM)
        uapi_sfc_resume(1);
#endif
        pm_clear_slp_wkup_event();
#if defined(CONFIG_KEYSCAN_SUPPORT_LPM)
        uapi_keyscan_resume(0);
#endif
    }
}

static void pm_partial_resume(void)
{
    // ArchICacheFlush
    write_custom_csr_val(0x7C2, 0x4);
    dsb();
    // ArchDCacheFlush
    write_custom_csr_val(0x7C3, 0xC);
    dsb();
    // ArchICacheEnable CACHE_8KB
    write_custom_csr_val(0x7C0, 0x3);
    dsb();
    // ArchICachePrefetchEnable
    write_custom_csr_val(0x7C6, 0x1);
    dsb();
    // ArchDCacheEnable CACHE_4KB
    write_custom_csr_val(0x7C1, 0x1);
    dsb();

    writew(PM_MCU_PERP_QSPI_CR_REG, g_pm_mcu_perp_qspi_cr_val);

    pm_uds_suspend_config(false);

    dsb();
    writeb(0x57030010, 0xFF);   // ULP_GPIO中断关闭
    writeb(0x5703000C, 0x0);
    writeb(0x5703002C, 0xFF);
    dsb();
    // 清pending
    WRITE_CSR(0xBF0, ULP_GPIO_IRQN);
    dsb();
    WRITE_CSR(0xBF0, ULP_INT_IRQN);
    dsb();

    void (*entry)(void) = (void*)(uintptr_t)(APP_PROGRAM_ORIGIN);
    entry();
}

static void pm_ultra_deep_sleep_suspend(void)
{
    // pc = pc_h << 16 + pc_l
    g_cpu_reset_pc = (readl(ULP_AON_CTL_MCPU_POR_RST_PC_H_REG) << 16) | (readl(ULP_AON_CTL_MCPU_POR_RST_PC_L_REG));
    // pc_h: pc >> 16
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_H_REG, (uint16_t)((uint32_t)(uintptr_t)lowpower_cpu_resume >> 16));
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_L_REG, (uint16_t)((uint32_t)(uintptr_t)lowpower_cpu_resume));
    lowpower_cpu_suspend(); // Must be placed at the end of this function
    // 未睡 or RTC唤醒喂狗
    if (!g_pm_ds_flag || (readw(ULP_AON_CTL_ULP_WKUP_EVT_STS_REG) & BIT(PM_ULP_RTC_WAKEUP)) != 0) {
        g_pm_ds_flag = false;
        return;
    }
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_H_REG, (uint16_t)((uint32_t)(uintptr_t)g_cpu_reset_pc >> 16)); // pc_h: pc >> 16
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_L_REG, (uint16_t)((uint32_t)(uintptr_t)g_cpu_reset_pc));
    writew(0x5702C4C0, 0x1); // RC32K resume
    pm_partial_resume();
}

static void pm_ultra_deep_sleep_config(void)
{
    // 喂狗
    writew(0x5702C934, 0xC5);
    // 定时唤醒&喂狗
    pm_wakeup_rtc_start(pm_get_ulp_wdt_timeout_ms());
    pm_sleep_wait_time_config();
    // 开睡眠事件
    writew(PMU1_CTL_LPM_MCPU_SLP_EVT_CLR_REG, PM_LPM_MCPU_SLP_ALL_MASK);
    writew(PMU1_CTL_LPM_MCPU_SLP_EVT_EN_REG, PM_LPM_MCPU_SLP_ALL_MASK);
    writew(ULP_AON_CTL_ULP_SLP_EVT_CLR_REG, 0x1);
    writew(ULP_AON_CTL_ULP_SLP_EVT_EN_REG, 0x1);
    // 关闭唤醒中断
    writew(PMU1_CTL_LPM_MCPU_WKUP_EVT_CLR_REG, PM_LPM_MCPU_WKUP_ALL_MASK);
    writew(PMU1_CTL_LPM_MCPU_WKUP_EVT_EN_REG, 0);
    writew(PMU1_CTL_LPM_MCPU_WKUP_INT_CLR_REG, PM_LPM_MCPU_WKUP_ALL_MASK);
    writew(PMU1_CTL_LPM_MCPU_WKUP_INT_EN_REG, 0);
    writew(ULP_AON_CTL_ULP_WKUP_EVT_CLR_REG, PM_ULP_WKUP_ALL_MASK);
    writew(ULP_AON_CTL_ULP_WKUP_EVT_EN_REG, 0x6);   // 需保留gpio和rtc事件唤醒
    writew(ULP_AON_CTL_ULP_WKUP_INT_CLR_REG, PM_ULP_WKUP_ALL_MASK);
    writew(ULP_AON_CTL_ULP_WKUP_INT_EN_REG, 0);
    // 关中断
    WRITE_CSR(0x304, 0x0);
    WRITE_CSR(0xBE0, 0x0);
    WRITE_CSR(0xBE1, 0x0);
    // 清pending
    for (uint32_t int_num = 0; int_num < 0x5A; int_num++) {
        WRITE_CSR(0xBF0, int_num);
        dsb();
    }
}

static void pm_force_the_system_to_ultra_deep_sleep(void)
{
    uint32_t irq_sts = osal_irq_lock();
    os_tick_timer_disable();
    // RAM retention disable (保留32KB RAM).
    writew(0x5702C0F8, 1);
    writew(0x5702C900, 0x13F);

    pm_uds_suspend_config(true);
    pm_config_based_on_cldo_state(false);
    // 关osTick
    // RC32k 降低驱动能力
    writew(0x5702C4C0, 0x101);

    pm_ultra_deep_sleep_suspend();
    pm_ultra_deep_sleep_config();

    g_pm_ds_flag = true;

    pm_port_allow_deepsleep(true);
    pm_port_enter_wfi();
    pm_port_allow_deepsleep(false);

    // 睡眠失败：也走重新加载流程
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_H_REG, (uint16_t)((uint32_t)(uintptr_t)g_cpu_reset_pc >> 16)); // pc_h: pc >> 16
    writew(ULP_AON_CTL_MCPU_POR_RST_PC_L_REG, (uint16_t)((uint32_t)(uintptr_t)g_cpu_reset_pc));
    writew(0x5702C4C0, 0x1); // RC32K resume
    pm_partial_resume();
    osal_irq_restore(irq_sts);
}

void hal_reboot_clear_ultra_deep_sleep_flag(void)
{
    writew(0x5702C700, 0);
    isb();
}
#endif