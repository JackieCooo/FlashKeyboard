/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: dfx system init
 * This file should be changed only infrequently and with great care.
 */

#include <stdint.h>
#include "dfx_adapt_layer.h"
#include "diag.h"
#include "soc_diag_cmd_id.h"
#include "diag_cmd_connect.h"
#include "diag_cmd_filter.h"
#include "diag_cmd_password.h"
#include "diag_cmd_beat_heart.h"
#include "diag_cmd_get_mem_info.h"
#include "diag_cmd_get_task_info.h"
#include "diag_cmd_mem_read_write.h"
#include "diag_mocked_shell.h"
#include "diag_bt_sample_data.h"
#include "osal_task.h"
#include "osal_msgqueue.h"
#include "osal_event.h"
#include "diag_ind_src.h"
#include "diag_filter.h"
#include "diag_msg.h"
#include "diag_dfx.h"
#include "diag_service.h"
#include "diag_cmd_dispatch.h"
#include "diag_filter.h"
#include "diag_channel.h"
#include "transmit.h"
#include "diag_rom_api.h"
#include "diag_adapt_layer.h"
#include "diag_adapt_sdt.h"
#include "soc_log_uart_instance.h"
#include "sample_data_adapt.h"
#include "dfx_channel.h"
#include "nv.h"
#include "diag_cmd_offline_log.h"
#include "diag_dfx_cmd_init.h"
#if CONFIG_DFX_SUPPORT_OFFLINE_LOG_FILE == DFX_YES
#include "log_file.h"
#endif
#if defined(CONFIG_BT_UPG_ENABLE) || defined(CONFIG_SLE_UPG_ENABLE)
#include "ota_upgrade.h"
#endif
#include "log_common.h"

#if defined(CONFIG_SUPPORT_LOG_THREAD)
static const diag_cmd_reg_obj_t g_diag_default_cmd_tbl[] = {
#if (CONFIG_DFX_SUPPORT_DIAG_CONNECT_PASSWORD == DFX_YES)
    { DIAG_CMD_CONNECT_RANDOM, DIAG_CMD_CONNECT_M_CHECK, diag_cmd_password },
#endif
    { DIAG_CMD_HOST_CONNECT, DIAG_CMD_HOST_DISCONNECT, diag_cmd_hso_connect_disconnect },
    { DIAG_CMD_MSG_RPT_AIR, DIAG_CMD_MSG_RPT_USR, diag_cmd_filter_set },
    { DIAG_CMD_MSG_CFG_SET_AIR, DIAG_CMD_MSG_CFG_SET_LEVEL, diag_cmd_filter_set },
#if (CONFIG_DFX_SUPPORT_DIAG_BEAT_HEART == DFX_YES)
    { DIAG_CMD_HEART_BEAT, DIAG_CMD_HEART_BEAT, diag_cmd_beat_heart },
#endif
#if (CONFIG_DFX_MEMORY_OPERATE == DFX_YES)
    { DIAG_CMD_MEM_MEM32, DIAG_CMD_MEM_W4, diag_cmd_mem_operate },
#endif
#if (CONFIG_DFX_SUPPORT_OFFLINE_LOG_FILE == DFX_YES)
    { DIAG_CMD_SET_OFFLINE_LOG, DIAG_CMD_SET_OFFLINE_LOG, diag_cmd_set_offline_log },
#endif /* CONFIG_DFX_SUPPORT_OFFLINE_LOG_FILE */
};

unsigned long g_dfx_osal_queue_id;
static errcode_t register_default_diag_cmd(void)
{
    return uapi_diag_register_cmd(g_diag_default_cmd_tbl,
        sizeof(g_diag_default_cmd_tbl) / sizeof(g_diag_default_cmd_tbl[0]));
}
#endif

int32_t msg_process_proc(uint32_t msg_id, uint8_t *data, uint16_t size)
{
    zdiag_dfx_rev_msg();
    switch (msg_id) {
        case DFX_MSG_ID_DIAG_PKT:
            zdiag_dfx_rev_pkt_msg();
            diag_msg_proc((uint16_t)msg_id, data, size);
            break;
        case DFX_MSG_ID_SDT_MSG:
            zdiag_adapt_sdt_msg_dispatch(msg_id, data, size);
            break;
        case DFX_MSG_ID_BEAT_HEART:
#if (CONFIG_DFX_SUPPORT_DIAG_BEAT_HEART == DFX_YES)
            zdiag_dfx_rev_beat_herat_msg();
            diag_beat_heart_process();
#endif
            break;
#if CONFIG_DFX_SUPPORT_TRANSMIT_FILE == DFX_YES
        case DFX_MSG_ID_TRANSMIT_FILE:
            transmit_msg_proc(msg_id, data, size);
            break;
#endif
        default:
            break;
    }
    return ERRCODE_SUCC;
}

#if (CONFIG_DFX_SUPPORT_DIAG_VRTTUAL_SHELL == DFX_YES)
static void cmd_shell_proc(uint8_t *data, uint32_t data_len)
{
    diag_debug_cmd_proc(data, data_len);
    dfx_log_debug("cmd shell: %s", data);
}
#endif

#if defined(CONFIG_SUPPORT_LOG_THREAD)
unsigned long dfx_get_osal_queue_id(void)
{
    return g_dfx_osal_queue_id;
}
#endif

static errcode_t dfx_transmit_init(void)
{
    errcode_t ret = ERRCODE_SUCC;
#if CONFIG_DFX_SUPPORT_TRANSMIT_FILE == DFX_YES
    uapi_transmit_init();
#endif

#if defined(CONFIG_BT_UPG_ENABLE) || defined(CONFIG_SLE_UPG_ENABLE)
    ret = transmit_item_dst_result_hook_register(ota_upgrade_transmit_result_cb);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
#endif
    return ret;
}

errcode_t dfx_system_init(void)
{
    errcode_t ret;
    diag_rom_api_t rom_api;

    uapi_diag_service_init();

#if CONFIG_DFX_SUPPORT_OFFLINE_LOG_FILE == DFX_YES
    uapi_logfile_init();
#endif

#if defined(CONFIG_SUPPORT_LOG_THREAD)
    ret = register_default_diag_cmd();
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
#endif

    zdiag_filter_init();
#if !defined(CONFIG_SUPPORT_LOG_THREAD)
    log_set_local_log_level(LOG_LEVEL_NONE);
#endif

#ifndef FORBID_AUTO_LOG_REPORT
    diag_auto_log_report_enable();
#else
#if CONFIG_DFX_SUPPORT_DIAG_BEAT_HEART == DFX_YES
    ret = diag_beat_heart_init();
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
#endif
#endif
    ret = diag_register_channel();
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

#if (CONFIG_DFX_SUPPORT_DIAG_VRTTUAL_SHELL == DFX_YES)
    zdiag_mocked_shell_init();
    zdiag_mocked_shell_register_cmd_data_proc(cmd_shell_proc);
#endif /* CONFIG_DFX_SUPPORT_DIAG_VRTTUAL_SHELL */

    rom_api.report_sys_msg = uapi_zdiag_report_sys_msg_instance;
    diag_rom_api_register(&rom_api);

    ret = dfx_transmit_init();
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

#if defined(CONFIG_SUPPORT_LOG_THREAD)
    ret = diag_dfx_cmd_init();
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
#endif

    return ERRCODE_SUCC;
}

void dfx_set_log_enable(void)
{
#ifdef CONFIG_NV_FEATURE_SUPPORT
    uint8_t flag;
    uint16_t length;
    errcode_t ret = uapi_nv_read(NV_ID_OFFLINELOG_ENBALE_FLAG, sizeof(flag), &length, &flag);
    if (ret != ERRCODE_SUCC) {
        return;
    }
    if (flag != 0) {
        uapi_zdiag_set_offline_log_enable(true);
    } else {
        uapi_zdiag_set_offline_log_enable(false);
    }
#endif
}