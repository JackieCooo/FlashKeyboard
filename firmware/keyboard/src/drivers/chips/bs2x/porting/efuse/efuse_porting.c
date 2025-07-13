/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides efuse port template \n
 *
 * History: \n
 * 2023-3-4， Create file. \n
 */
#include "chip_io.h"
#include "securec.h"
#ifdef EFUSE_BIT_OPERATION
#include "boot_serial.h"
#endif
#if defined(BUILD_APPLICATION_STANDARD)
#include "osal_debug.h"
#include "pm_pmu.h"
#endif
#include "hal_efuse_v151.h"
#include "efuse.h"
#include "efuse_porting.h"

#define EFUSE0_BASE_ADDR             0x57028000
#define HAL_EFUSE0_WRITE_BASE_ADDR   (EFUSE0_BASE_ADDR + 0x800)
#define HAL_EFUSE0_READ_BASE_ADDR    (EFUSE0_BASE_ADDR + 0x800)

#define EFUSE_IDX_NRW 0x0
#define EFUSE_IDX_RO  0x1
#define EFUSE_IDX_WO  0x2
#define EFUSE_IDX_RW  0x3
#define BIT_SIZE_OF_BYTE 8

uint32_t g_efuse_boot_done_addr = EFUSE0_BASE_ADDR + 0x2c;
#if defined(CONFIG_EFUSE_SWITCH_EN)
uint32_t g_efuse_switch_en_addr = 0x5702C258;
#endif
uint32_t g_efuse_base_addr[CONFIG_EFUSE_REGION_NUM] = {EFUSE0_BASE_ADDR + 0x30};
uint32_t g_efuse_region_read_address[CONFIG_EFUSE_REGION_NUM] = {HAL_EFUSE0_READ_BASE_ADDR};
uint32_t g_efuse_region_write_address[CONFIG_EFUSE_REGION_NUM] = {HAL_EFUSE0_WRITE_BASE_ADDR};
typedef struct {
    uint16_t id_start_bit;
    uint16_t id_size;
    uint8_t attr;
} efuse_config_t;
#ifdef EFUSE_BIT_OPERATION
static const efuse_config_t g_efuse_cfg[EFUSE_IDX_MAX] = {
    { 0,   144, EFUSE_IDX_RW }, // die id
    { 600,  424,  EFUSE_IDX_RW }, // reserved id
};
#endif

void efuse_port_register_hal_funcs(void)
{
    hal_efuse_register_funcs(hal_efuse_funcs_get());
}

void efuse_port_unregister_hal_funcs(void)
{
    hal_efuse_unregister_funcs();
}

hal_efuse_region_t hal_efuse_get_region(uint32_t byte_addr)
{
    return (hal_efuse_region_t)(byte_addr / EFUSE_REGION_MAX_BYTES);
}

uint16_t hal_efuse_get_byte_offset(uint32_t byte_addr)
{
    return byte_addr % EFUSE_REGION_MAX_BYTES;
}

#ifdef EFUSE_BIT_OPERATION
uint32_t efuse_read_item(efuse_idx_t efuse_id, uint8_t *data, uint16_t data_len)
{
    uint16_t item_data_len;
    uint16_t bit_index;
    uint32_t byte_index;
    uint8_t bit_offset;
    uint8_t value;
    uint16_t id_start_bit;
    uint16_t id_size;
    uint16_t result_index;
    uint32_t ret;
    if ((data == NULL) || (efuse_id >= EFUSE_IDX_MAX)) {
        return ERRCODE_FAIL;
    }
    id_start_bit = g_efuse_cfg[efuse_id].id_start_bit;
    id_size = g_efuse_cfg[efuse_id].id_size;
    item_data_len = (id_size + BIT_SIZE_OF_BYTE - 1) / BIT_SIZE_OF_BYTE;
    if (item_data_len > data_len) {
        return ERRCODE_FAIL;
    }
    (void)memset_s(data, data_len, 0, data_len);
    if ((id_size % BIT_SIZE_OF_BYTE) == 0 && (id_start_bit % BIT_SIZE_OF_BYTE) == 0) {
        ret = uapi_efuse_read_buffer(data, (id_start_bit / BIT_SIZE_OF_BYTE), item_data_len);
        if (ret != ERRCODE_SUCC) {
            return ret;
        }
    } else {
        for (bit_index = id_start_bit; bit_index < (id_start_bit + id_size); bit_index++) {
            byte_index = bit_index / BIT_SIZE_OF_BYTE;
            bit_offset = bit_index % BIT_SIZE_OF_BYTE;
            ret = uapi_efuse_read_bit(&value, byte_index, bit_offset);
            if (ret != ERRCODE_SUCC) {
                return ret;
            }
            if (value != 0) {
                result_index = (bit_index - id_start_bit) / BIT_SIZE_OF_BYTE;
                data[result_index] = data[result_index] |
                    (1 << ((uint16_t)((bit_index - id_start_bit) % BIT_SIZE_OF_BYTE)));
            }
        }
    }
    return ERRCODE_SUCC;
}

void efuse_port_item_puts(efuse_idx_t efuse_id, uint8_t *data)
{
    uint16_t data_len = (g_efuse_cfg[efuse_id].id_size + BIT_SIZE_OF_BYTE - 1) / BIT_SIZE_OF_BYTE;
    if (data_len > EFUSE_READ_MAX_BYTE) {
        data_len = EFUSE_READ_MAX_BYTE;
    }

    for (uint16_t i = 0; i < data_len; i++) {
        serial_puthex(data[i], 1);
        serial_puts("  ");
    }
}
#endif

#if defined(BUILD_APPLICATION_STANDARD)
void efuse_close_swd_debug_function(swd_fusing_t type)
{
    uint8_t buffer[0x2], bit;
    if (type == SWD_HW_FUSING) {
        bit = 0x6;  // swd_fusing_bit
    } else {
        bit = 0x5;  // swd_sw_enable_bit
    }

    errcode_t ret = uapi_efuse_read_buffer(buffer, 0x25, 0x2);
    if (ret != ERRCODE_SUCC) {
        osal_printk("Efuse read buffer is failed, ret = 0x%x\r\n", ret);
        return;
    }

    buffer[0] = buffer[0] | (1 << bit);
    buffer[1] = buffer[1] | (1 << bit);
    pm_efuse_ldo_power(true);
    ret = uapi_efuse_write_buffer(0x25, buffer, 0x2);
    pm_efuse_ldo_power(false);
    if (ret != ERRCODE_SUCC) {
        osal_printk("Efuse write buffer is failed, ret = 0x%x\r\n", ret);
    }
}

void efuse_swd_debug_enable(bool en)
{
    if (en) {
        writew(0x57000420, 0x8AB3);
    } else {
        writew(0x57000420, 0x0);
    }
}
#endif