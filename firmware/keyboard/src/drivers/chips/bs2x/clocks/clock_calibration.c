/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: CLOCK CALIBRATION DRIVER.
 *
 * Create: 2020-03-17
 */

#include "clock_calibration.h"
#include "platform_core.h"
#include "hal_32k_clock.h"
#include "otp_map.h"
#include "efuse.h"
#include "hal_tcxo.h"
#include "chip_io.h"
#include "non_os.h"
#include "tcxo.h"
#include "nv.h"
#include "key_id.h"
#include "common.h"
#include "tcxo_porting.h"
#include "pm_sleep_porting.h"
#include "uart.h"
#include "debug_print.h"
#ifdef XO_32M_CALI
#include "pm_pmu.h"
#endif

#define CALIBRATION_XO_CORE_TRIM_REG XO_CORE_TRIM_REG
#define CALIBRATION_XO_CORE_CTRIM_REG XO_CORE_CTRIM_REG

#define CALIBRATION_XO_CORE_TRIM_DEFAULT 1

#define CALIBRATION_XO_CORE_CTRIM_BIT 0
#define CALIBRATION_XO_CORE_CTRIM_LEN 8
#define CALIBRATION_XO_CORE_CTRIM_MAX 0xFF
#define CALIBRATION_XO_CORE_CTRIM_MIN 0x0
#define CALIBRATION_XO_CORE_CTRIM_POS 0xFF

#define CALIBRATION_CLOCK_MUL 100
#define CALIBRATION_CLOCK_FREQ_32K_100 3276800

#define XO_CTRIM_CONFIG_WAIT_US 20ULL
#define XO_CTRIM_ADJUST_INTERVAL_VALUE 8
#define XO_CTRIM_LOW_BIT_ADJUST_MASK 7
#define XO_CTRIM_VALUE_DEFAULT 0x0

#define XO_CTRIM_NV_DEBUG 0x0

static uint32_t g_clock_32k = CALIBRATION_CLOCK_FREQ_32K_100;

uint32_t calibration_get_clock_frq(uint16_t cali_cycle)
{
    uint32_t result;
    hal_32k_clock_calibration_cycle_config(cali_cycle);
    result = hal_32k_clock_get_detect_result();
    if (result != 0) {
        g_clock_32k = (uint32_t)(((uint64_t)(cali_cycle)*HAL_TCXO_TICKS_PER_SECOND * CALIBRATION_CLOCK_MUL) / result);
    }

    return g_clock_32k; // return 32K clock
}

uint32_t calibration_get_clock_frq_result(void)
{
    return g_clock_32k;
}

#ifdef XO_32M_CALI
void calibration_xo_core_trim_init(void)
{
    // need use trim value from otp
    // the xo_trim and xo_ctrim should update at same time like this:
    // xo_trim:    0x1  0x03  ...  0x0F
    // xo_ctrim:   0x53 0x56  ...  0x76
    writew(CALIBRATION_XO_CORE_TRIM_REG, CALIBRATION_XO_CORE_TRIM_DEFAULT);
}

// 使用flash或efuse存储校准值时的初始化，优先判断flash
void calibration_xo_core_ctrim_init(void)
{
    uint8_t xo_ctrim_value = 0;
    calibration_xo_core_ctrim_read_flash(&xo_ctrim_value);

    if (xo_ctrim_value == 0) {
        // use default value if NO otp config
        calibration_read_xo_core_ctrim(&xo_ctrim_value);
        if (xo_ctrim_value == 0) {
        // use default value if NO otp config
            xo_ctrim_value = XO_CTRIM_VALUE_DEFAULT;
        }
    }
    calibration_set_xo_core_ctrim(xo_ctrim_value);
}

// 使用flash存储校准值时的初始化
void calibration_xo_core_ctrim_flash_init(void)
{
    uint8_t xo_ctrim_value = 0;
    calibration_xo_core_ctrim_read_flash(&xo_ctrim_value);

    if (xo_ctrim_value == 0) {
        // use default value if NO otp config
        xo_ctrim_value = XO_CTRIM_VALUE_DEFAULT;
    }
    calibration_set_xo_core_ctrim(xo_ctrim_value);
}

void calibration_set_xo_core_ctrim(uint8_t xo_ctrim_value)
{
    // config gm from 4 to 9.75 with 3 steps
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                  XO_CORE_TRIM_CTL_STEP1_VAL);
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                  XO_CORE_TRIM_CTL_DEFAULT_VAL);
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL4_REG, XO_CORE_TRIM_REV_CTL, XO_CORE_TRIM_REV_CTL_LEN,
                  XO_CORE_TRIM_REV_CTL_DEFAULT_VAL);
    reg16_setbits(ULP_AON_CTL_RB_ADDR + 0x1004, 0xF, 0x1, 0x1);
    // 由于gm降低影响ctrim校准结果，需要在gm = 9.75的情况下配置ctrim值
    reg16_setbits(CALIBRATION_XO_CORE_CTRIM_REG, CALIBRATION_XO_CORE_CTRIM_BIT, CALIBRATION_XO_CORE_CTRIM_LEN,
        xo_ctrim_value);
    // config gm from 9.75 to 4 with 3 steps
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL4_REG, XO_CORE_TRIM_REV_CTL, XO_CORE_TRIM_REV_CTL_LEN,
                  XO_CORE_TRIM_REV_CTL_STEP1_VAL);
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                  XO_CORE_TRIM_CTL_STEP1_VAL);
    reg16_setbits(ULP_AON_CTL_PMU_TRIM_CTL6_REG, XO_CORE_TRIM_CTL, XO_CORE_TRIM_CTL_LEN,
                  XO_CORE_TRIM_CTL_STEP2_VAL);
}

void calibration_save_xo_core_ctrim(uint8_t xo_ctrim_value)
{
    uint8_t xo_ctrim_value_rd = 0;
    errcode_t read_ret = uapi_efuse_read_buffer(&xo_ctrim_value_rd, XO_CORE_CTRIM, 0x1);
    if (read_ret != ERRCODE_SUCC) {
        osal_printk("[Acore] read efuse fail, ret = 0x%x\r\n", read_ret);
        return;
    }
    if (xo_ctrim_value_rd == 0) {
        pm_efuse_ldo_power(true);
        errcode_t write_ret = uapi_efuse_write_buffer(XO_CORE_CTRIM, &xo_ctrim_value, 0x1);
        if (write_ret != ERRCODE_SUCC) {
            osal_printk("write efuse fail, ret = 0x%x\r\n", write_ret);
        }
        pm_efuse_ldo_power(false);
    }
}

void calibration_read_xo_core_ctrim(uint8_t *xo_ctrim_value)
{
    if (xo_ctrim_value == NULL) {
        return;
    }
    uint8_t xo_ctrim_value_rd = 0;
    errcode_t read_ret = uapi_efuse_read_buffer(&xo_ctrim_value_rd, XO_CORE_CTRIM, 0x1);
    if (read_ret != ERRCODE_SUCC) {
        osal_printk("read efuse fail! ret = 0x%x\r\n", read_ret);
    }
    *xo_ctrim_value = xo_ctrim_value_rd & CALIBRATION_XO_CORE_CTRIM_POS;
}

void calibration_xo_core_ctrim_algorithm(bool increase, uint8_t step_num)
{
    reg16_setbits(ULP_AON_CTL_RB_ADDR + 0x1004, 0xF, 0x1, 0x1);
    uint8_t xo_ctrim_value =
        reg16_getbits(CALIBRATION_XO_CORE_CTRIM_REG, CALIBRATION_XO_CORE_CTRIM_BIT, CALIBRATION_XO_CORE_CTRIM_LEN);
    if (increase) {
        if ((xo_ctrim_value + step_num) > CALIBRATION_XO_CORE_CTRIM_MAX) {
            xo_ctrim_value = CALIBRATION_XO_CORE_CTRIM_MAX;
        } else {
            xo_ctrim_value = xo_ctrim_value + step_num;
        }
    } else {
        if (xo_ctrim_value < step_num) {
            xo_ctrim_value = CALIBRATION_XO_CORE_CTRIM_MIN;
        } else {
            xo_ctrim_value = xo_ctrim_value - step_num;
        }
    }
    reg16_setbits(CALIBRATION_XO_CORE_CTRIM_REG, CALIBRATION_XO_CORE_CTRIM_BIT, CALIBRATION_XO_CORE_CTRIM_LEN,
        xo_ctrim_value);
}

void calibration_get_xo_core_ctrim_reg(uint8_t *xo_ctrim_value)
{
    if (xo_ctrim_value == NULL) {
        return;
    }
    reg16_setbits(ULP_AON_CTL_RB_ADDR + 0x1004, 0xF, 0x1, 0x1);
    *xo_ctrim_value =
        reg16_getbits(CALIBRATION_XO_CORE_CTRIM_REG, CALIBRATION_XO_CORE_CTRIM_BIT, CALIBRATION_XO_CORE_CTRIM_LEN);
}

void calibration_set_xo_core_trim_reg(uint8_t xo_trim)
{
    non_os_enter_critical();
    writew(CALIBRATION_XO_CORE_TRIM_REG, xo_trim);
    non_os_exit_critical();
}

/**
  * ctrim校准值写flash方案
  */
void calibration_xo_core_ctrim_save_flash(uint8_t xo_ctrim_value)
{
    uint16_t key = BTH_BLE_NV_RESERVED_ID;
    size_t key_len = (uint16_t)sizeof(bth_ble_nv_reserved_struct_t);
    uint16_t real_len = 0;
    uint8_t *ctrim_nv = (uint8_t *)osal_vmalloc(key_len);
    if (ctrim_nv == NULL) {
        osal_printk("[ACore] malloc fail.\r\n");
        return;
    }
    errcode_t read_nv_ret = uapi_nv_read(key, (uint16_t)key_len, &real_len, ctrim_nv);
    if (read_nv_ret != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_printk("[ACore] nv read fail, ret = 0x%x\r\n", read_nv_ret);
        osal_vfree(ctrim_nv);
        ctrim_nv = NULL;
        return;
    }
    ctrim_nv[0x8] = 0x1; // ctrim flag
    ctrim_nv[0x9] = xo_ctrim_value; // ctrim value
    errcode_t nv_ret_value = uapi_nv_write(key, ctrim_nv, (uint16_t)key_len);
    if (nv_ret_value != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_printk("[ACore] nv write fail, ret = 0x%x\r\n", nv_ret_value);
        osal_vfree(ctrim_nv);
        ctrim_nv = NULL;
        return;
    }

#if XO_CTRIM_NV_DEBUG
    osal_printk("[INFO]flag = 0x%x, value = 0x%x\n", *(ctrim_nv), *(ctrim_nv + 1));
#endif
    if (ctrim_nv != NULL) {
        osal_vfree(ctrim_nv);
        ctrim_nv = NULL;
    }
}

// 从flash读出CTRIM校准值
void calibration_xo_core_ctrim_read_flash(uint8_t *xo_ctrim_value)
{
    uint16_t key = BTH_BLE_NV_RESERVED_ID;
    size_t key_len = (uint16_t)sizeof(bth_ble_nv_reserved_struct_t);
    uint16_t real_len = 0;
    uint8_t *ctrim_nv = (uint8_t *)osal_vmalloc(key_len);
    if (ctrim_nv == NULL) {
        osal_printk("[ACore]malloc fail!\r\n");
        return;
    }
    errcode_t read_nv_ret = uapi_nv_read(key, (uint16_t)key_len, &real_len, ctrim_nv);
    if (read_nv_ret != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_printk("[ACore]read nv fail!, ret = 0x%x\r\n", read_nv_ret);
        osal_vfree(ctrim_nv);
        ctrim_nv = NULL;
        return;
    }
    // ctrim flag = 1
    if (ctrim_nv[0x8] == 0x1) {
        *xo_ctrim_value = ctrim_nv[0x9]; // ctrim value
        osal_printk("[ACore] ctrim value from flash:0x%x\n", *xo_ctrim_value);
    }
#if XO_CTRIM_NV_DEBUG
    osal_printk("[INFO]flag = 0x%x, value = 0x%x\n", *(ctrim_nv), *(ctrim_nv + 1));
#endif
    if (ctrim_nv != NULL) {
        osal_vfree(ctrim_nv);
        ctrim_nv = NULL;
    }
}
#endif