/*================================================================
* Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
*
* Description: NV on different flash
*
* History:
* 2023-10-11， Create file.
================================================================*/

#include "nv_porting.h"
#include "sfc.h"
#include "soc_osal.h"
#include "std_def.h"
#include "uapi_crc.h"
#include "memory_config_common.h"

#define FLASH_WORD_ALIGN 4
#define NV_START_OFFSET ((NV_STATR_ADDR) - (SFC_FLASH_START))
#define NV_END_OFFSET (((NV_STATR_ADDR) + (NV_LENGTH)) - (SFC_FLASH_START))

static uint32_t flash_read_data_align(const uint32_t *addr, uint32_t *length, uint8_t *first_cover_count,
                                      uint8_t *last_cover_count)
{
    uint32_t dest_addr;
    /* Check if the address is 4 bytes aligned. */
    if (((*addr) & 0x3) != 0) {
        /* The actual send address requires 4 byte alignment. */
        dest_addr = ((*addr) & 0xFFFFFFFC);

        /* Fill the first 4 bytes of data. */
        *first_cover_count = (*addr) & 0x3;
        (*length) += (*first_cover_count);
    } else {
        dest_addr = (*addr);
    }
    /* Check if the last 4 bytes are aligned. */
    if (((*length) & 0x3) != 0) {
        /* Fill the first 4 bytes of data. */
        *last_cover_count = FLASH_WORD_ALIGN - (uint8_t)((*length) & 0x3);
        (*length) += (*last_cover_count);
    }
    return dest_addr;
}

errcode_t kv_flash_read(const uint32_t flash_offset, const uint32_t size, uint8_t *ram_data)
{
    if ((flash_offset < NV_START_OFFSET) || ((flash_offset + size) > NV_END_OFFSET)) {
        return ERRCODE_INVALID_PARAM;
    }
    errcode_t ret = ERRCODE_SUCC;
    uint32_t read_size = size;
    uint8_t first_cover_count = 0;
    uint8_t last_cover_count = 0;
    uint32_t dest_addr = flash_read_data_align(&flash_offset, &read_size, &first_cover_count, &last_cover_count);
    uint8_t *align_read_buff = (uint8_t *)kv_malloc(read_size);
    if (align_read_buff == NULL) { return ERRCODE_MALLOC; }
    uint32_t mstatus = osal_irq_lock();
    if (uapi_sfc_reg_read(dest_addr, align_read_buff, read_size) != ERRCODE_SUCC) {
        osal_irq_restore(mstatus);
        kv_free(align_read_buff);
        return ERRCODE_FAIL;
    }
    osal_irq_restore(mstatus);
    if (memcpy_s((uint8_t *)ram_data, size, (uint8_t *)(align_read_buff + first_cover_count), size) != EOK) {
        ret = ERRCODE_FAIL;
    }
    kv_free(align_read_buff);
    return ret;
}

errcode_t kv_flash_write(const uint32_t flash_offset, uint32_t size, const uint8_t *ram_data, bool do_erase)
{
    if ((flash_offset < NV_START_OFFSET) || ((flash_offset + size) > NV_END_OFFSET)) {
        return ERRCODE_INVALID_PARAM;
    }
    
    UNUSED(do_erase);
    errcode_t ret = ERRCODE_SUCC;

    uint32_t write_size = size;
    uint8_t first_cover_count = 0;
    uint8_t last_cover_count = 0;
    uint32_t dest_addr = flash_read_data_align(&flash_offset, &write_size, &first_cover_count, &last_cover_count);

    uint8_t *align_write_buff = (uint8_t *)kv_malloc(write_size);
    if (align_write_buff == NULL) { return ERRCODE_MALLOC; }
    uint32_t mstatus = osal_irq_lock();
    if (uapi_sfc_reg_read(dest_addr, align_write_buff, write_size) != ERRCODE_SUCC) {
        osal_irq_restore(mstatus);
        kv_free(align_write_buff);
        return ERRCODE_FAIL;
    }
    osal_irq_restore(mstatus);
    if (memcpy_s((uint8_t *)(align_write_buff + first_cover_count), write_size, (uint8_t *)ram_data, size) != EOK) {
        kv_free(align_write_buff);
        return ERRCODE_FAIL;
    }

    mstatus = osal_irq_lock();
    if (uapi_sfc_reg_write(dest_addr, (uint8_t *)align_write_buff, write_size) != ERRCODE_SUCC) {
        ret = ERRCODE_FAIL;
    }
    osal_irq_restore(mstatus);
    kv_free(align_write_buff);
    return ret;
}

errcode_t kv_flash_erase(const uint32_t flash_offset, uint32_t size)
{
    if ((flash_offset < NV_START_OFFSET) || ((flash_offset + size) > NV_END_OFFSET)) {
        return ERRCODE_INVALID_PARAM;
    }

    errcode_t ret = ERRCODE_SUCC;
    uint32_t mstatus = osal_irq_lock();
    if (uapi_sfc_reg_erase(flash_offset, FLASH_PAGE_SIZE) != ERRCODE_SUCC) {
        ret = ERRCODE_FAIL;
    }
    osal_irq_restore(mstatus);
    return ret;
}