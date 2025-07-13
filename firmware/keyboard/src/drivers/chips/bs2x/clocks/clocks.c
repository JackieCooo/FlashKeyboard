/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2023. All rights reserved.
 * Description:   CLOCKS DRIVER.
 *
 * Create:
 */

#include "chip_io.h"
#include "soc_osal.h"
#include "product.h"
#include "tcxo.h"
#include "clocks_switch.h"
#include "clock_calibration.h"
#include "pm_pmu.h"
#include "pm_clock.h"

static const clocks_clk_cfg_t g_system_clocks_cfg[CLOCKS_CCRG_MODULE_MAX] = {
#if (BS21_DLL2_ENABLE == YES)
    {CLOCKS_CLK_SRC_TCXO_2X, CLOCK_DIV_1},      // CLOCKS_CCRG_MODULE_MCU_CORE
#else
    {CLOCKS_CLK_SRC_TCXO, CLOCK_DIV_1},         // CLOCKS_CCRG_MODULE_MCU_CORE
#endif
#if defined(CONFIG_REDUCE_PERP_LS_FREQ)
    {CLOCKS_CLK_SRC_TCXO, CLOCK_DIV_4},         // CLOCKS_CCRG_MODULE_MCU_PERP_LS
#else
    {CLOCKS_CLK_SRC_TCXO, CLOCK_DIV_1},         // CLOCKS_CCRG_MODULE_MCU_PERP_LS
#endif
    {CLOCKS_CLK_SRC_TCXO, CLOCK_DIV_1},         // CLOCKS_CCRG_MODULE_MCU_PERP_UART
#if defined(SUPPORT_EXTERN_FLASH) || defined(SUPPORT_SPI_BUS_2)
    {CLOCKS_CLK_SRC_TCXO, CLOCK_DIV_1},         // CLOCKS_CCRG_MODULE_MCU_PERP_SPI
#else
    {CLOCKS_CLK_SRC_NONE, CLOCK_DIV_1},         // CLOCKS_CCRG_MODULE_MCU_PERP_SPI
#endif
#if defined(CONFIG_SUPPORT_NFC_SERVICE)
    {CLOCKS_CLK_SRC_RC, CLOCK_DIV_2},           // CLOCKS_CCRG_MODULE_NFC_SUB
#else
    {CLOCKS_CLK_SRC_NONE, CLOCK_DIV_2},         // CLOCKS_CCRG_MODULE_NFC_SUB
#endif
    {CLOCKS_CLK_SRC_NONE, CLOCK_DIV_4},         // CLOCKS_CCRG_MODULE_I2S
#if (BS21_DLL2_ENABLE == YES)
    {CLOCKS_CLK_SRC_TCXO_2X, CLOCK_DIV_1},
#else
    {CLOCKS_CLK_SRC_TCXO, CLOCK_DIV_1},         // CLOCKS_CCRG_MODULE_XIP_QSPI
#endif
};

#define ULP_AON_CTL_XO_REG0_H_REG              (ULP_AON_CTL_RB_ADDR + 0x280)
#define XO_REG0_H_VAL                          0x2709
#define ULP_AON_CTL_XO_REG0_L_REG              (ULP_AON_CTL_RB_ADDR + 0x284)
#define XO_REG0_L_VAL                          0xE1E1
#define ULP_AON_CTL_XO_REG1_H_REG              (ULP_AON_CTL_RB_ADDR + 0x288)
#define XO_REG1_H_VAL                          0x3200
#define ULP_AON_CTL_XO_REG1_L_REG              (ULP_AON_CTL_RB_ADDR + 0x28C)
#define XO_REG1_L_VAL                          0xCA0C
#define ULP_AON_CTL_EFUSE_ULP_SEL0_REG         (ULP_AON_CTL_RB_ADDR + 0x1000)
#define XO_CORE_TRIM_SEL                       0x0
#define XO_CORE_TRIM_SEL_BIT_LEN               0x2
#define XO_CORE_TRIM_SEL_RES_VAL               0x3
#define ULP_AON_CTL_PMU_TRIM_CTL4_REG          (ULP_AON_CTL_RB_ADDR + 0x1020)
#define XO_CORE_TRIM_REV_CTL                   0x1
#define XO_CORE_TRIM_REV_CTL_LEN               0x5
#define ULP_AON_CTL_PMU_TRIM_CTL6_REG          (ULP_AON_CTL_RB_ADDR + 0x1028)
#define XO_CORE_TRIM_CTL                       0x8
#define XO_CORE_TRIM_CTL_LEN                   0x5
#define XO_CORE_TRIM_REV_CTL_DEFAULT_VAL       0x4
#define XO_CORE_TRIM_REV_CTL_STEP1_VAL         0x0
#define XO_CORE_TRIM_CTL_DEFAULT_VAL           0x1f
#define XO_CORE_TRIM_CTL_STEP1_VAL             0x17
#define XO_CORE_TRIM_CTL_STEP2_VAL             0x11
#define FUSE_CTL_RB_PMU2_TRIM_REG              (FUSE_CTL_RB_ADDR + 0x834)
#define XO_CORE_TRIM_GET                       0x1
#define XO_CORE_TRIM_GET_LEN                   0x5
#define FUSE_CTL_RB_CMU_TRIM0_REG              (FUSE_CTL_RB_ADDR + 0x83C)
#define XO_CORE_TRIM_REV_GET                   0x8
#define XO_CORE_TRIM_REV_GET_LEN               0x5

#ifndef CONFIG_XO_32K_ENABLE
#define HAL_CALIBRATION_PMU_TRIM_CTL2          (ULP_AON_CTL_RB_ADDR + 0x1018)
#define HAL_CALIBRATION_PMU_RC32_RES_TRIM      8
#define HAL_CALIBRATION_PMU_RC32_RES_TRIM_LEN  6

#define HAL_CALIBRATION_PMU_VOLT_TEST          (ULP_AON_CTL_RB_ADDR + 0x5CC)
#define HAL_CALIBRATION_PMU_VOLT_SEL           0
#define HAL_CALIBRATION_PMU_VOLT_SEL_LEN       5
#define HAL_CALIBRATION_PMU_VOLT_FRQ0_SEL      0x8
#define HAL_CALIBRATION_PMU_VOLT_FRQ1_SEL      0xE
#define HAL_CALIBRATION_PMU_VOLT_EN            5

#define HAL_CALIBRATION_PMU2_CFG0              (PMU2_CMU_CTL_RB_BASE + 0x138)
#define HAL_CALIBRATION_VOLT_TEST_BUF_EN       6

#define HAL_CALIBRATION_EFUSE_ULP_SEL0         (ULP_AON_CTL_RB_ADDR + 0x1000)
#define HAL_CALIBRATION_RC32K_RES_TRIM_SEL     11

// 32K时钟周期数，0x40即2ms
#define PM_CLK_32K_DET_VAL                     0x40
#define PM_RC32K_MAX_TRIM_VAL                  0x3F
#define PM_RC32K_MIN_CALI_FREQ                 3104000
#define PM_RC32K_MAX_CALI_FREQ                 3296000

static void clocks_rc32k_calibration(void)
{
    volatile uint32_t freq = calibration_get_clock_frq(PM_CLK_32K_DET_VAL);
    if ((freq >= PM_RC32K_MIN_CALI_FREQ) && (freq <= PM_RC32K_MAX_CALI_FREQ)) { // 误差范围内
        return;
    }
    volatile uint16_t trim_val = 0;
    volatile uint16_t trim_val_record = reg16_getbits(HAL_CALIBRATION_PMU_TRIM_CTL2, HAL_CALIBRATION_PMU_RC32_RES_TRIM,
                                                      HAL_CALIBRATION_PMU_RC32_RES_TRIM_LEN);

    reg16_setbit(HAL_CALIBRATION_PMU_VOLT_TEST, HAL_CALIBRATION_PMU_VOLT_EN); // PMU电源电压测试MUX通路选择使能
    reg16_setbits(HAL_CALIBRATION_PMU_VOLT_TEST, HAL_CALIBRATION_PMU_VOLT_SEL, HAL_CALIBRATION_PMU_VOLT_SEL_LEN,
                  HAL_CALIBRATION_PMU_VOLT_FRQ0_SEL); // PMU电源电压测试MUX通路选择
    reg16_clrbit(HAL_CALIBRATION_PMU2_CFG0, HAL_CALIBRATION_VOLT_TEST_BUF_EN); // 仅trim电流时关闭buffer
    reg16_setbits(HAL_CALIBRATION_PMU_VOLT_TEST, HAL_CALIBRATION_PMU_VOLT_SEL, HAL_CALIBRATION_PMU_VOLT_SEL_LEN,
                  HAL_CALIBRATION_PMU_VOLT_FRQ1_SEL); // PMU电源电压测试MUX通路选择
    reg16_setbit(HAL_CALIBRATION_EFUSE_ULP_SEL0, HAL_CALIBRATION_RC32K_RES_TRIM_SEL); // PMU1基准trim配置成manual模式
    do { // 手动校准
        reg16_setbits(HAL_CALIBRATION_PMU_TRIM_CTL2, HAL_CALIBRATION_PMU_RC32_RES_TRIM,
                      HAL_CALIBRATION_PMU_RC32_RES_TRIM_LEN, trim_val++); // 调节时钟频率
        freq = calibration_get_clock_frq(PM_CLK_32K_DET_VAL);
        if ((freq >= PM_RC32K_MIN_CALI_FREQ) && (freq <= PM_RC32K_MAX_CALI_FREQ)) { // 误差范围内
            writew(HAL_CALIBRATION_PMU_VOLT_TEST, 0);
            return;
        }
    } while (trim_val <= PM_RC32K_MAX_TRIM_VAL);
    // 没找到正确校准值
    reg16_setbits(HAL_CALIBRATION_PMU_TRIM_CTL2, HAL_CALIBRATION_PMU_RC32_RES_TRIM,
                  HAL_CALIBRATION_PMU_RC32_RES_TRIM_LEN, trim_val_record);
    writew(HAL_CALIBRATION_PMU_VOLT_TEST, 0);
}
#endif

static void clocks_32k_sel_config(void)
{
#ifndef CONFIG_XO_32K_ENABLE
    writew(0x5702C4C0, 0x1); // rc_32k enable
    writew(0x5702C4B0, 0x0); // xo_32k disable
    clocks_rc32k_calibration(); // trim rc_32k clock
#else
    writew(0x5702C4B0, 0x1); // xo_32k enable
    writew(0x5702C4C0, 0x0); // rc_32k disable
#endif
}

static void clocks_system_init(void)
{
    uint32_t irq_sts = osal_irq_lock();
    for (uint8_t module = CLOCKS_CCRG_MODULE_MCU_CORE; module < CLOCKS_CCRG_MODULE_MAX; module++) {
        system_ccrg_clock_config(module, g_system_clocks_cfg[module].clk_src, g_system_clocks_cfg[module].clk_div);
    }
#if (BS21_DLL2_ENABLE == NO)
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_CLDO, PMU_CLDO_VSET_1V05);
#ifdef CONFIG_POWER_SUPPLY_BY_LDO
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_1V1);
#else
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_BUCK_1P1, PMU_BUCK_1P1_VSET_1V1);
#endif
#endif
    osal_irq_restore(irq_sts);
}

void clocks_hardware_sub_init(void)
{
    writew(0x520003E0, 0x3);    // MSUB_NOR_CFG
    uapi_clock_control(CLOCK_CONTROL_GLB_CLKEN_DISABLE, CLOCK_GLB_CLKEN_USB_PHY);
    uapi_clock_control(CLOCK_CONTROL_GLB_CLKEN_DISABLE, CLOCK_GLB_CLKEN_USB_BUS);
    uapi_clock_control(CLOCK_CONTROL_GLB_CLKEN_DISABLE, CLOCK_GLB_CLKEN_CAN_BUS);
    uapi_clock_control(CLOCK_CONTROL_GLB_CLKEN_DISABLE, CLOCK_GLB_CLKEN_PDM);
}

#if defined(BUILD_APPLICATION_STANDARD)
static void clocks_mclken_config_init(void)
{
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_MTOP_GLUE_TRIGGER_CLKEN);
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_PULSE_CAPTURE_CLKEN);
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_BT_TGTWS_CLKEN);

    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_TRNG_CLKEN);
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_I2S_CLKEN);
    // SPI.
#if defined(SUPPORT_EXTERN_FLASH)
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_ENABLE, CLOCK_APERP_SPI0_M_CLKEN);
#else
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_SPI0_M_CLKEN);
#endif
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_SPI1_M_CLKEN);
#if defined(SUPPORT_SPI_BUS_2)
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_ENABLE, CLOCK_APERP_SPI2_M_CLKEN);
#else
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_SPI2_M_CLKEN);
#endif
    // I2C.
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_I2C0_CLKEN);
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_I2C1_CLKEN);

#ifdef SUPPORT_SLP_CLIENT
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_ENABLE, CLOCK_APERP_UART_L1_CLKEN);
#else
    uapi_clock_control(CLOCK_CONTROL_MCLKEN_DISABLE, CLOCK_APERP_UART_L1_CLKEN);
#endif
}

static void clocks_xo_32m_trim(void)
{
    // 将EFUSE内的初始GM值，即9.75配置到XO_CORE_TRIM_REV和XO_CORE_TRIM中
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL4_REG, XO_CORE_TRIM_REV_CTL, XO_CORE_TRIM_REV_CTL_LEN,
                  XO_CORE_TRIM_REV_CTL_DEFAULT_VAL);
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                  XO_CORE_TRIM_CTL_DEFAULT_VAL);

    // 快启值配置
    writew(ULP_AON_CTL_XO_REG0_H_REG, XO_REG0_H_VAL);
    writew(ULP_AON_CTL_XO_REG0_L_REG, XO_REG0_L_VAL);
    writew(ULP_AON_CTL_XO_REG1_H_REG, XO_REG1_H_VAL);
    writew(ULP_AON_CTL_XO_REG1_L_REG, XO_REG1_L_VAL);

    // 将XO_CORE_TRIM_REV和XO_CORE_TRIM的控制由EFUSE切换为寄存器控制
    reg16_setbits(ULP_AON_CTL_EFUSE_ULP_SEL0_REG, XO_CORE_TRIM_SEL, XO_CORE_TRIM_SEL_BIT_LEN, XO_CORE_TRIM_SEL_RES_VAL);
    // 逐步将GM总值降为4，XO_CORE_TRIM_REV位域最终值为0，XO_CORE_TRIM位域最终值为0x11
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL4_REG, XO_CORE_TRIM_REV_CTL, XO_CORE_TRIM_REV_CTL_LEN,
                  XO_CORE_TRIM_REV_CTL_STEP1_VAL);
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN, XO_CORE_TRIM_CTL_STEP1_VAL);
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN, XO_CORE_TRIM_CTL_STEP2_VAL);
}

void clocks_init(void)
{
#if (BS21_DLL2_ENABLE == YES)
    writew(0x52000540, 0x7);
#else
    writew(0x52000540, 0x6);
#endif
    clocks_32k_sel_config();
    clocks_system_init();
    clocks_hardware_sub_init();
    clocks_mclken_config_init();
    clocks_xo_32m_trim();
    writew(0x5200004C, 0x3); // pdm&trng时钟源选RC（RC关闭->降低功耗）
}
#endif