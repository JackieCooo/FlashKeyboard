/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Provides SFC patch source \n
 *
 * History: \n
 * 2024-07-03, Create file. \n
 */
#include "sfc.h"
#include "hal_sfc.h"
#include "hal_sfc_v150_regs_op.h"
#include "sfc_porting.h"
#include "patch.h"
#include "soc_osal.h"

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
#define FLASH_CHIP_PROTECT_END             0x400000

#define STANDARD_SPI             0x0
#define ENABLE                   0x1
#define DISABLE                  0x0
#define READ_MODE                0x1
#define WRITE_MODE               0x0
#define BYTES_64                 0x40
#define BYTES_64_MASK            0x3F
#define BYTES_4K                 0x1000
#define BYTES_4K_MASK            0xFFF
#define BYTES_4                  0x4
#define SFC_READ_TIME_SHIFT_NUM  6
#define ERASE_CHIP               0

#define _4K                             0x1000
#define _32K                            0x8000
#define _64K                            0x10000
#define CHIP_SIZE                       0x3ffff

extern flash_spi_ctrl_t g_flash_ctrl;
extern errcode_t check_opt_param(uint32_t addr, uint32_t size);

static errcode_t do_greedy_erase(uint32_t hal_erase_size, uint32_t start_sector)
{
    uint32_t erase_opt_index;
    uint32_t temp_size = 0;
    errcode_t ret = ERRCODE_FAIL;
    /* All parameters are aligned in 4KB. */
    uint32_t remain_size = hal_erase_size;
    uint32_t current_addr = start_sector;
    spi_opreation_t current_erase_opt = {0};
    spi_opreation_t flash_erase_cmds[] = {
        {SPI_CMD_SUPPORT, 0xC7, 0x0, CHIP_SIZE},
        {SPI_CMD_SUPPORT, 0xD8, 0x0, _64K},
        {SPI_CMD_SUPPORT, 0x52, 0x0, _32K},
        {SPI_CMD_SUPPORT, 0x20, 0x0, _4K}
    };
    while (remain_size > 0) {
        uint8_t erase_opt_num = 4;
        for (erase_opt_index = 1; erase_opt_index < erase_opt_num; erase_opt_index++) {
            current_erase_opt = flash_erase_cmds[erase_opt_index];
            temp_size = current_erase_opt.size;
            if ((remain_size >= temp_size) && ((current_addr & (temp_size - 1)) == 0)) {
                break;
            }
        }
        /* Generally, the 4K erase is not configured for this branch. Check the erase array at the port layer. */
        if (erase_opt_index == erase_opt_num) {
            return ERRCODE_SFC_ERASE_FORM_ERROR;
        }
        ret = hal_sfc_reg_erase(current_addr, current_erase_opt, false);
        if (ret != ERRCODE_SUCC) {
            return ret;
        }
        remain_size -= temp_size;
        current_addr += temp_size;
    }
    return ERRCODE_SUCC;
}

errcode_t uapi_sfc_reg_other_flash_opt_patch(sfc_flash_op_t cmd_type, uint8_t cmd, uint8_t *buffer, uint32_t length)
{
    if (unlikely(length > BYTES_4)) {
        return ERRCODE_INVALID_PARAM;
    }
    uint32_t lock_sts = osal_irq_lock();
    errcode_t ret = hal_sfc_reg_flash_opreations(cmd_type, cmd, buffer, length);
    osal_irq_restore(lock_sts);
    return ret;
}

errcode_t uapi_sfc_reg_read_patch(uint32_t flash_addr, uint8_t *read_buffer, uint32_t read_size)
{
    errcode_t ret = check_opt_param(flash_addr, read_size);
    if (unlikely(ret != ERRCODE_SUCC)) {
        return ret;
    }

    /* Unaligned length */
    uint32_t temp_addr = flash_addr;
    uint8_t *buffer_ptr = read_buffer;
    uint32_t current_size = read_size & BYTES_64_MASK;
    uint16_t read_times = (read_size >> SFC_READ_TIME_SHIFT_NUM) + 1;
    /* Cyclic read. After the first read, 64 bytes are read. */
    uint32_t lock_sts = osal_irq_lock();
    for (uint32_t i = 0; i < read_times; i++) {
        if (current_size == 0) {
            /* Read_size is already 64-bytes aligned, skip one read. */
            current_size = BYTES_64;
            continue;
        }
        ret = hal_sfc_reg_read(temp_addr, buffer_ptr, current_size, g_flash_ctrl.read_opreation);
        if (unlikely(ret != ERRCODE_SUCC)) {
            osal_irq_restore(lock_sts);
            return ret;
        }
        temp_addr += current_size;
        buffer_ptr += current_size;
        current_size = BYTES_64;
    }
    osal_irq_restore(lock_sts);
    return ERRCODE_SUCC;
}

errcode_t hal_sfc_execute_cmds_patch(flash_cmd_execute_t *command)
{
    unused(command);
    return ERRCODE_SUCC;
}

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
    hal_sfc_regs_set_opt(hal_opreation);
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
    hal_sfc_regs_set_opt(hal_opreation);

    hal_sfc_regs_set_opt_attr(WRITE_MODE, ENABLE, DISABLE); // cmd starts to be sent here
    hal_sfc_regs_wait_config();
}

static uint32_t sfc_port_write_lock(uint32_t start_addr, uint32_t end_addr)
{
    unused(start_addr);
    unused(end_addr);
    uint32_t lock_sts = osal_irq_lock();
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

static void sfc_port_write_unlock(uint32_t lock_sts)
{
    uint8_t flash_sts0 = 0;
    uint8_t flash_sts1 = 0;
    uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_0, &flash_sts0, 1);
    uapi_sfc_reg_other_flash_opt(READ_TYPE, READ_FLASH_STATUS_1, &flash_sts1, 1);
    flash_sts0 |= WRITE_ALL_PROTECT_MASK;
    sfc_port_set_flash_sts_in_volatile(flash_sts0, flash_sts1);
    osal_irq_restore(lock_sts);
}

errcode_t uapi_sfc_reg_write_patch(uint32_t flash_addr, uint8_t *write_data, uint32_t write_size)
{
    errcode_t ret = check_opt_param(flash_addr, write_size);
    if (unlikely(ret != ERRCODE_SUCC)) {
        return ret;
    }
    spi_opreation_t write_opt = {SPI_CMD_SUPPORT, 0x02, 0x0, 0};
    uint32_t unaligned_size = BYTES_64 - (flash_addr & BYTES_64_MASK);
    if (write_size < unaligned_size) {
        unaligned_size = write_size;
    }
    uint32_t temp_addr = flash_addr;
    uint8_t *buffer_ptr = write_data;
    /* Part 1: The start address is not aligned by 64 Byte. */
    uint32_t lock_sts = sfc_port_write_lock(flash_addr, flash_addr + write_size);
    ret = hal_sfc_reg_write(temp_addr, buffer_ptr, unaligned_size, write_opt);
    if (unlikely(ret != ERRCODE_SUCC)) {
        sfc_port_write_unlock(lock_sts);
        return ret;
    }
    uint32_t remained_size = write_size - unaligned_size;
    buffer_ptr += unaligned_size;
    temp_addr += unaligned_size;
    /* Part 2: Follow-up data processing. The cross-page problem does not need to be considered.
       Any flash memory larger than 64 bytes/page can be used. */
    while (remained_size > 0) {
        uint32_t current_size = remained_size <= BYTES_64 ? remained_size : BYTES_64;
        ret = hal_sfc_reg_write(temp_addr, buffer_ptr, current_size, write_opt);
        if (unlikely(ret != ERRCODE_SUCC)) {
            sfc_port_write_unlock(lock_sts);
            return ret;
        }
        buffer_ptr += current_size;
        temp_addr += current_size;
        remained_size -= current_size;
    }
    sfc_port_write_unlock(lock_sts);
    return ERRCODE_SUCC;
}

errcode_t uapi_sfc_reg_erase_patch(uint32_t flash_addr, uint32_t erase_size)
{
    errcode_t ret = check_opt_param(flash_addr, erase_size);
    if (unlikely(ret != ERRCODE_SUCC)) {
        return ret;
    }
    uint32_t end_addr = flash_addr + erase_size;
    uint32_t start_sector = flash_addr & ~BYTES_4K_MASK;
    uint32_t end_sector = (end_addr & BYTES_4K_MASK) == 0 ? end_addr : (end_addr & ~BYTES_4K_MASK) + BYTES_4K;
    uint32_t hal_erase_size = end_sector - start_sector;

    if (flash_addr != start_sector || end_addr != end_sector) {
        return ERRCODE_INVALID_PARAM;
    }
    /* Erasing with greedy algorithms */
    uint32_t lock_sts = sfc_port_write_lock(start_sector, end_sector);
    ret = do_greedy_erase(hal_erase_size, start_sector);
    sfc_port_write_unlock(lock_sts);
    return ret;
}

errcode_t uapi_sfc_reg_erase_chip_patch(void)
{
    errcode_t ret = check_opt_param(0, 0);
    if (unlikely(ret != ERRCODE_SUCC)) {
        return ret;
    }
    spi_opreation_t erase_chip_opt = {SPI_CMD_SUPPORT, 0xC7, 0x0, CHIP_SIZE};
    uint32_t lock_sts = sfc_port_write_lock(0x0, FLASH_CHIP_PROTECT_END);
    ret = hal_sfc_reg_erase(0x0, erase_chip_opt, true);
    sfc_port_write_unlock(lock_sts);
    return ret;
}

EXPORT_HARD_PATCH(hal_sfc_execute_cmds, hal_sfc_execute_cmds_patch);
EXPORT_HARD_PATCH(uapi_sfc_reg_write, uapi_sfc_reg_write_patch);
EXPORT_HARD_PATCH(uapi_sfc_reg_erase, uapi_sfc_reg_erase_patch);
EXPORT_HARD_PATCH(uapi_sfc_reg_erase_chip, uapi_sfc_reg_erase_chip_patch);
EXPORT_HARD_PATCH(uapi_sfc_reg_read, uapi_sfc_reg_read_patch);
EXPORT_HARD_PATCH(uapi_sfc_reg_other_flash_opt, uapi_sfc_reg_other_flash_opt_patch);