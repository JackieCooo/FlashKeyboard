/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: last dump
 * This file should be changed only infrequently and with great care.
 */

#include "errcode.h"
#if defined(CONFIG_WATCHDOG_SUPPORT_ULP_WDT) && (CONFIG_WATCHDOG_SUPPORT_ULP_WDT == 1)
#include "watchdog_porting.h"
#endif
#include "diag.h"
#include "soc_diag_cmd_id.h"
#include "dfx_adapt_layer.h"
#include "diag_adapt_layer.h"
#include "last_dump_st.h"
#include "last_dump.h"

#define LAST_DUMP_PER_SIZE (4 * 1024)

static void dfx_last_dump_report(uint32_t file_num, uint16_t cmd_id)
{
    last_dump_start_ind_t ind = {0};
    ind.file_num = file_num;

    uint8_t *data[1];
    uint16_t len[1];

    data[0] = (uint8_t *)&ind;
    len[0] = (uint16_t)sizeof(last_dump_start_ind_t);
    (void)uapi_diag_report_packets_critical(cmd_id, NULL, data, len, 1); /* pkt_cnt 为1 */
}

void dfx_last_dump_start(uint32_t file_num)
{
    dfx_last_dump_report(file_num, DIAG_CMD_ID_LAST_DUMP_START);
}

void dfx_last_dump_end(uint32_t file_num)
{
    dfx_last_dump_report(file_num, DIAG_CMD_ID_LAST_DUMP_END);
}

void dfx_last_dump_data(const char *name, uintptr_t addr, uint32_t size)
{
    last_dump_data_ind_t ind = { 0 };
    uint8_t *data_addr = (uint8_t *)addr;
    uint32_t offset = 0;
    uint8_t *data[2];
    uint16_t len[2];
    uint32_t size_tmp = size;

    if (strncpy_s(ind.name, LAST_DUMP_NAME_LEN, name, strlen(name)) != EOK) {
        dfx_log_err("dfx_last_dump_data(%s) error \r\n", name);
        return;
    }
    ind.total_size = size_tmp;
    data[0] = (uint8_t *)&ind;
    len[0] = (uint16_t)sizeof(last_dump_data_ind_t);
    while (size_tmp != 0) {
#if defined(CONFIG_WATCHDOG_SUPPORT_ULP_WDT) && (CONFIG_WATCHDOG_SUPPORT_ULP_WDT == 1)
        ulp_wdt_kick();
#endif
        uint16_t per_size = (uint16_t)uapi_min(LAST_DUMP_PER_SIZE, size_tmp);
        data[1] = (uint8_t *)data_addr;
        len[1] = per_size;
        ind.offset = offset;
        ind.size = per_size;
        (void)uapi_diag_report_packets_critical(DIAG_CMD_ID_LAST_DUMP, NULL, data, len, 2); /* pkt_cnt 为2 */
        data_addr += per_size;
        size_tmp -= per_size;
        offset += per_size;
    }

    last_dump_data_ind_finish_t *finish_ind = (last_dump_data_ind_finish_t *)(uintptr_t)&ind;
    finish_ind->start_address = addr;
    data[0] = (uint8_t *)finish_ind;
    len[0] = (uint16_t)sizeof(last_dump_data_ind_finish_t);
    (void)uapi_diag_report_packets_critical(DIAG_CMD_ID_LAST_DUMP_FINISH, NULL, data, len, 1); /* pkt_cnt 为1 */
}

void dfx_last_word_send(uint8_t *buf, uint32_t size)
{
    uint8_t *data[1];
    uint16_t len[1];

    data[0] = buf;
    len[0] = (uint16_t)size;
    (void)uapi_diag_report_packets_critical(DIAG_CMD_ID_LAST_WORD, NULL, data, len, 1); /* pkt_cnt 为1 */
}
