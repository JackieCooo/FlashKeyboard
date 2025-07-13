/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides sfc port template \n
 *
 * History: \n
 * 2022-11-30， Create file. \n
 */
#include "hal_sfc_v150.h"
#include "hal_sfc_v150_regs_op.h"
#ifndef BUILD_NOOSAL
#include "soc_osal.h"
#endif
#include "sfc.h"
#include "sfc_porting.h"

static uint32_t g_sfc_delay_once_us = SFC_DELAY_ONCE_US;
static uint32_t g_sfc_delay_times = SFC_DELAY_TIMES;

#ifndef BUILD_NOOSAL
static osal_mutex g_sfc_mutex = { NULL };
static bool g_sfc_mutex_inited = false;
#endif

#define WRITE_VOLATILE_ENABLE              0x50
#define WRITE_ALL_PROTECT_MASK             0x7C
#ifdef FLASH_1M
#define WRITE_PROTECT_MASK                 0x30
#define FLASH_HALF_ADDR                    0x80000
#else
#define WRITE_PROTECT_MASK                 0x2a
#define FLASH_HALF_ADDR                    0x40000
#endif
#define READ_FLASH_STATUS_0                0x5
#define READ_FLASH_STATUS_1                0x35
#define SET_FLASH_STATUS                   0x1

uint32_t sfc_port_get_delay_times(void)
{
    return (uintptr_t)g_sfc_delay_times;
}

void sfc_port_set_delay_times(uint32_t delay_times)
{
    g_sfc_delay_times = delay_times;
}

void sfc_port_set_delay_once_time(uint32_t delay_us)
{
    g_sfc_delay_once_us = delay_us;
}

uint32_t sfc_port_get_delay_once_time(void)
{
    return (uintptr_t)g_sfc_delay_once_us;
}

void sfc_port_lock_init(void)
{
#ifndef BUILD_NOOSAL
    if (!g_sfc_mutex_inited) {
        osal_mutex_init(&g_sfc_mutex);
        g_sfc_mutex_inited = true;
    }
#endif
}

uint32_t sfc_port_lock(void)
{
#ifdef SUPPORT_SFC_IRQ_LOCK
    return osal_irq_lock();
#else
#ifndef BUILD_NOOSAL
    if (g_sfc_mutex_inited && !osal_in_interrupt()) {
        return osal_mutex_lock_timeout(&g_sfc_mutex, OSAL_MUTEX_WAIT_FOREVER);
    }
#endif
    return ERRCODE_SUCC;
#endif
}

void sfc_port_unlock(uint32_t lock_sts)
{
#ifdef SUPPORT_SFC_IRQ_LOCK
    osal_irq_restore(lock_sts);
#else
    unused(lock_sts);
#ifndef BUILD_NOOSAL
    if (g_sfc_mutex_inited && !osal_in_interrupt()) {
        osal_mutex_unlock(&g_sfc_mutex);
    }
#endif
#endif
}

#define STANDARD_SPI             0x0

#define ENABLE                   0x1
#define DISABLE                  0x0

#define READ_MODE                0x1
#define WRITE_MODE               0x0

static void sfc_port_set_flash_sts_in_volatile(uint8_t sts0, uint8_t sts1)
{
    hal_spi_opreation_t hal_opreation;
    // 50H: Write Enable for Volatile Status Register
    // The 50H must be issued prior to a 01H
    hal_opreation.opt.cmd_support = SPI_CMD_SUPPORT;
    hal_opreation.opt.cmd = WRITE_VOLATILE_ENABLE;
    hal_opreation.opt.iftype = STANDARD_SPI;
    hal_opreation.opt.size = 0x0;
    hal_opreation.data_size = 0x0;
    hal_opreation.dummy_byte = 0x0;

    // ROM化后，函数参数有变化，flashboot和ssb中用的是非ROM的接口
    hal_sfc_regs_set_opt(&hal_opreation);
    hal_sfc_regs_set_opt_attr(WRITE_MODE, DISABLE, DISABLE); // cmd starts to be sent here
    hal_sfc_regs_wait_config();

    cmd_databuf_t databuf;
    // 01H: Write Status Register 1&2 (WRSRn)
    hal_opreation.opt.cmd_support = SPI_CMD_SUPPORT;
    hal_opreation.opt.cmd = SET_FLASH_STATUS;
    hal_opreation.opt.iftype = STANDARD_SPI;
    hal_opreation.opt.size = 0x0;
    hal_opreation.data_size = 0x2;
    hal_opreation.dummy_byte = 0x0;

    databuf.d32 = 0;
    databuf.b.databyte[0] = sts0;
    databuf.b.databyte[1] = sts1;
    hal_sfc_regs_set_databuf(0, databuf.d32);
    hal_sfc_regs_set_opt(&hal_opreation);

    hal_sfc_regs_set_opt_attr(WRITE_MODE, ENABLE, DISABLE); // cmd starts to be sent here
    hal_sfc_regs_wait_config();
}

uint32_t sfc_port_write_lock(uint32_t start_addr, uint32_t end_addr)
{
    unused(start_addr);
    unused(end_addr);
    uint32_t lock_sts = sfc_port_lock();
    uint8_t flash_sts0 = 0;
    uint8_t flash_sts1 = 0;
    uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_0, &flash_sts0, 1);
    uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_1, &flash_sts1, 1);
    flash_sts0 &= (~WRITE_ALL_PROTECT_MASK);
    if (start_addr >= FLASH_HALF_ADDR) {
        flash_sts0 = flash_sts0 | WRITE_PROTECT_MASK;
    }
    sfc_port_set_flash_sts_in_volatile(flash_sts0, flash_sts1);
    return lock_sts;
}

void sfc_port_write_unlock(uint32_t lock_sts)
{
    uint8_t flash_sts0 = 0;
    uint8_t flash_sts1 = 0;
    uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_0, &flash_sts0, 1);
    uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_1, &flash_sts1, 1);
    flash_sts0 |= WRITE_ALL_PROTECT_MASK;
    sfc_port_set_flash_sts_in_volatile(flash_sts0, flash_sts1);
    sfc_port_unlock(lock_sts);
}

#if defined(CONFIG_SFC_SUPPORT_WRITE_PROTECT)
errcode_t sfc_port_lock_write_protect(sfc_flash_protect_region_t region)
{
    if (region != SFC_FLASH_REGION_LOWER_1_2) {
        return ERRCODE_FAIL;
    }
    uint8_t flash_sts0 = 0;
    uint8_t flash_sts1 = 0;
    errcode_t ret = uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_0, &flash_sts0, 1);
    ret |= uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_1, &flash_sts1, 1);
    flash_sts0 = flash_sts0 | WRITE_PROTECT_MASK;
    sfc_port_set_flash_sts_in_volatile(flash_sts0, flash_sts1);
    return ret;
}

errcode_t sfc_port_unlock_write_protect(void)
{
    uint8_t flash_sts0 = 0;
    uint8_t flash_sts1 = 0;
    errcode_t ret = uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_0, &flash_sts0, 1);
    ret |= uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_1, &flash_sts1, 1);
    flash_sts0 = flash_sts0 & (~WRITE_PROTECT_MASK);
    sfc_port_set_flash_sts_in_volatile(flash_sts0, flash_sts1);
    return ret;
}
#endif