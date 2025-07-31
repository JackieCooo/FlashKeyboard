/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2025-2025. All rights reserved.
 *
 * Description:  w33 firmware file arrays.
 *
 * History:
 * 2025-06-12, Create file.
 */

#include "tiot_types.h"
#include "errcode.h"
#include "partition.h"
#include "tiot_board_log.h"
#include "w33_fw_file_array.h"

tiot_file g_w33_fw_files[] = {
    { "cfg", 8, 0 }
};

const tiot_file_path g_w33_fw_file_path = {
    g_w33_fw_files, sizeof(g_w33_fw_files) / sizeof(tiot_file)
};

static const char *get_img_addr(void)
{
    uint32_t addr;
    const uint32_t base_addr = 0x90100000;
    partition_information_t info = { .type = PARTITION_BY_ADDRESS };

    if (uapi_partition_get_info(PARTITION_SLP_IMAGE, &info) != ERRCODE_SUCC) {
        return NULL;
    }
    addr = base_addr + info.part_info.addr_info.addr;
    return (const char*)addr;
}

const tiot_file_path *w33_fw_file_path_get(void)
{
    const char *addr = get_img_addr();
    if (addr == NULL) {
        tiot_print_err("[TIoT]get img addr fail.\r\n");
        return NULL;
    }
    g_w33_fw_files[0].data = addr;
    tiot_print_info("flash addr = 0x%x\r\n", addr);
    return &g_w33_fw_file_path;
}
