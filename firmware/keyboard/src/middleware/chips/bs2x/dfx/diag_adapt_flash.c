/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: diag adapt flash
 * This file should be changed only infrequently and with great care.
 */

#include "debug_print.h"
#include "diag.h"
#include "dfx_adapt_layer.h"
#include "sfc.h"
#if defined(SUPPORT_EXTERN_FLASH)
#include "flash.h"
#endif
#include "memory_config_common.h"
#include "partition.h"
#include "diag_adapt_flash.h"

#define SIZE_4K                         0x1000
#define SECTOR_ERASE_DELAY_MS           0x200

#define EXTERN_FLASH_OTA_START_ADDRESS    0x80000
static int32_t diag_adapt_flash_addr_get(uint8_t opt_type, uint32_t offset)
{
    uint32_t addr = offset;
    if (opt_type == FLASH_OP_TYPE_OTA) {
        // 读取分区表获取flash存储地址
        partition_information_t info;
        if (uapi_partition_get_info(PARTITION_FOTA_DATA, &info) != ERRCODE_SUCC) {
            return -1;
        }
        addr += info.part_info.addr_info.addr;
    } else if (opt_type == FLASH_OP_TYPE_LOG_FILE) {
        addr += LOG_FILE_FLASH_START_ADDRESS;
    } else if (opt_type == FLASH_OP_TYPE_FLASH_DATA) {
        addr -= FLASH_START;
    } else {
        return -1;
    }
    return addr;
}

/* flash 读写 */
int32_t diag_adapt_flash_read(uint8_t opt_type, uint32_t offset, uint8_t *buf, uint32_t size)
{
    int32_t addr = diag_adapt_flash_addr_get(opt_type, offset);
    if (addr == -1) { return -1; }
    uint32_t length = size;
    uint32_t irq = osal_irq_lock();
#if defined(SUPPORT_EXTERN_FLASH)
    length = uapi_flash_read_data(0, (addr - EXTERN_FLASH_PARTITION_OFFSET), buf, size);
#else
    if (uapi_sfc_reg_read((uint32_t)addr, buf, size) != ERRCODE_SUCC) {
        length = 0;
    }
#endif
    osal_irq_restore(irq);
    return (int32_t)length;
}

int32_t diag_adapt_flash_write(uint8_t opt_type, uint32_t offset, const uint8_t *buf, uint32_t size,
    bool do_erase)
{
    unused(opt_type);
    unused(do_erase);
    int32_t addr = diag_adapt_flash_addr_get(opt_type, offset);
    if (addr == -1) { return -1; }
    uint32_t length = size;
    uint32_t irq = osal_irq_lock();
#if defined(SUPPORT_EXTERN_FLASH)
    length = uapi_flash_write_data(0, (addr - EXTERN_FLASH_PARTITION_OFFSET), buf, size);
#else
    if (uapi_sfc_reg_write((uint32_t)addr, (uint8_t *)buf, size) != ERRCODE_SUCC) {
        length = 0;
    }
#endif
    osal_irq_restore(irq);
    return (int32_t)length;
}

int32_t diag_adapt_flash_info_write(uint8_t opt_type, uint32_t offset, const uint8_t *buf, uint32_t size,
    bool do_erase)
{
    unused(opt_type);
    unused(do_erase);
    int32_t addr = offset;
    if (opt_type == FLASH_OP_TYPE_OTA) {
        partition_information_t info;
        if (uapi_partition_get_info(PARTITION_FOTA_DATA, &info) != ERRCODE_SUCC) {
            return -1;
        }
        addr += info.part_info.addr_info.addr;
    } else if (opt_type == FLASH_OP_TYPE_LOG_FILE) {
        addr += LOG_FILE_FLASH_START_ADDRESS;
    } else {
        return -1;
    }
    uint32_t length = size;
    uint32_t irq = osal_irq_lock();
#if defined(SUPPORT_EXTERN_FLASH)
    length = uapi_flash_write_data(0, (addr - EXTERN_FLASH_PARTITION_OFFSET), buf, size);
#else
    if (uapi_sfc_reg_write((uint32_t)addr, (uint8_t *)buf, size) != ERRCODE_SUCC) {
        length = 0;
    }
#endif
    osal_irq_restore(irq);
    return (int32_t)length;
}

/*
 * flash擦除数据接口
 */
errcode_t diag_adapt_flash_erase(uint8_t opt_type, const uint32_t offset, const uint32_t size)
{
    unused(opt_type);
    unused(size);

    errcode_t ret = ERRCODE_SUCC;
    int32_t addr = diag_adapt_flash_addr_get(opt_type, offset);
    if (addr == -1) { return ERRCODE_FAIL; }

#if defined(SUPPORT_EXTERN_FLASH)
    for (uint32_t i = addr; i < (addr + size); i += SIZE_4K) {
        ret = uapi_flash_sector_erase(0, (i - EXTERN_FLASH_PARTITION_OFFSET), false);
        while (uapi_flash_is_processing(0)) {
            osal_msleep(SECTOR_ERASE_DELAY_MS);
        }
        if (ret) { return ret; }
    }
#else
    for (uint32_t i = addr; i < (addr + size); i += SIZE_4K) {
        ret = uapi_sfc_reg_erase(i, SIZE_4K);
        if (ret) { return ret; }
    }
#endif
    return ret;
}