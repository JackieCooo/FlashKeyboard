/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: ota upgrade.
 * This file should be changed only infrequently and with great care.
 */

#include "ota_upgrade.h"
#include "ota_upgrade_handle.h"
#include "diag_service.h"
#include "osal_timer.h"
#include "chip_io.h"
#include "tcxo.h"
#include "transmit.h"

#define OTA_UPGRADE_PREPARE  0x1
#define OTA_UPGRADE_REQUEST  0x2
#define OTA_UPGRADE_START    0x3
#define OTA_UPGRADE_STOP     0x4
#define OTA_UPGRADE_GETINFO  0x5

#define OTA_UPGRADE_TIMER_PERIOD  1000 /* 毫秒 */

#define OTA_PREPARE_TRANS_TIMEOUT   (60 * 10) /* 秒 */
#define OTA_UPGRADE_TIMEOUT         5  /* 秒 */
#define OTA_UPGRADE_1000MS          1000

typedef struct {
    uint8_t cmd_id;
    upgrade_pkt_recv_hook handler;
} upgrade_cmd_ind_item_t;

static upgrade_cmd_ind_item_t g_upgrade_cmd_id_tbl[] = {
    { OTA_UPGRADE_PREPARE,      ota_upgrade_prepare},
    { OTA_UPGRADE_REQUEST,  ota_upgrade_request },
    { OTA_UPGRADE_START,    ota_upgrade_start },
    { OTA_UPGRADE_STOP,      NULL },
    { OTA_UPGRADE_GETINFO,     ota_upgrade_getinfo },
};

typedef struct {
    uint32_t ota_start_sec;
    errcode_t ota_trans_result;
    uint32_t ota_trans_finish_sec;
} ota_upgrade_ctrl_t;

static ota_upgrade_ctrl_t g_ota_upgrade_ctrl = { 0 };
static ota_upgrade_state_changed_callback g_ota_upgrade_state_cb;
static osal_timer g_ota_upgrade_timer;

static void ota_upgrade_timeout_process(uint8_t cmd_id, errcode_t result);
static void ota_upgrade_ctl_init(void);
static void ota_upgrade_timer_handler(unsigned long data);
static uint32_t ota_get_cur_second(void);

static errcode_t ota_upgrade_cmd_receiver(uint8_t cmd_id, uint8_t *cmd_param, uint16_t cmd_param_size, uint8_t dst)
{
    uint8_t i;
    for (i = 0; i < sizeof(g_upgrade_cmd_id_tbl) / sizeof(g_upgrade_cmd_id_tbl[0]); i++) {
        upgrade_cmd_ind_item_t *item = &g_upgrade_cmd_id_tbl[i];
        if (item->cmd_id == cmd_id && item->handler != NULL) {
            errcode_t ret = item->handler(cmd_id, cmd_param, cmd_param_size, dst);
            ota_upgrade_timeout_process(cmd_id, ret);
            return ERRCODE_SUCC;
        }
    }
    return ERRCODE_NOT_SUPPORT;
}

errcode_t ota_upgrade_service_process(diag_ser_data_t *data)
{
    diag_ser_frame_t *request = (diag_ser_frame_t *)((uint8_t *)data + sizeof(diag_ser_data_t));
    tlv_t *tlv_data = (tlv_t *)((uint8_t *)request + sizeof(diag_ser_frame_t));
    uint16_t size = (uint16_t)(data->header.length - sizeof(diag_ser_frame_t));
    uint8_t dst = data->header.src;
    return ota_upgrade_cmd_receiver(request->cmd_id, (uint8_t *)tlv_data, size, dst);
}

errcode_t uapi_upgrade_init(void)
{
    errcode_t ret;
    ota_upgrade_ctl_init();
    g_ota_upgrade_timer.data = 0;
    g_ota_upgrade_timer.interval = OTA_UPGRADE_TIMER_PERIOD;
    g_ota_upgrade_timer.handler = ota_upgrade_timer_handler;
    int ret_timer = osal_timer_init(&g_ota_upgrade_timer);
    if (ret_timer < 0) {
        ota_log_print("[ota] timer init failed ret = %#x.\r\n", ret_timer);
        return ERRCODE_FAIL;
    }
    ret = uapi_diag_service_register(DIAG_SER_OTA, ota_upgrade_service_process);
    if (ret != ERRCODE_SUCC) {
        osal_timer_destroy(&g_ota_upgrade_timer);
        return ret;
    }
    return ERRCODE_SUCC;
}

errcode_t ota_upgrade_callback_register(ota_upgrade_state_changed_callback func)
{
    if (g_ota_upgrade_state_cb != NULL) {
        ota_log_print("[OTA] ota upgrade state changed callback already registered\r\n");
        return ERRCODE_FAIL;
    }
    g_ota_upgrade_state_cb = func;
    return ERRCODE_SUCC;
}

void ota_upgrade_callback_unregister(void)
{
    g_ota_upgrade_state_cb = NULL;
}

ota_upgrade_state_changed_callback ota_upgrade_get_callback(void)
{
    return g_ota_upgrade_state_cb;
}

errcode_t ota_upgrade_transmit_result_cb(errcode_t result, uintptr_t usr_data)
{
    UNUSED(usr_data);
    g_ota_upgrade_ctrl.ota_trans_finish_sec = ota_get_cur_second();
    g_ota_upgrade_ctrl.ota_trans_result = result;
    ota_log_print("[OTA] transmit result 0x%x\r\n", g_ota_upgrade_ctrl.ota_trans_result);
    return ERRCODE_SUCC;
}

void ota_upgrade_ctl_init(void)
{
    g_ota_upgrade_ctrl.ota_start_sec = 0;
    g_ota_upgrade_ctrl.ota_trans_result = ERRCODE_SUCC;
    g_ota_upgrade_ctrl.ota_trans_finish_sec = 0;
}

void ota_upgrade_timeout_process(uint8_t cmd_id, errcode_t result)
{
    if (cmd_id == OTA_UPGRADE_PREPARE) {
        if (result != ERRCODE_SUCC) {
            return;
        }
        osal_timer_stop(&g_ota_upgrade_timer);
        ota_upgrade_ctl_init();
        g_ota_upgrade_ctrl.ota_start_sec = ota_get_cur_second();
        if (g_ota_upgrade_state_cb != NULL) {
            g_ota_upgrade_state_cb(OTA_UPGRADE_STATE_START, result);
        }
        osal_timer_start(&g_ota_upgrade_timer);
    }
    if ((cmd_id == OTA_UPGRADE_REQUEST) || (cmd_id == OTA_UPGRADE_START)) {
        ota_upgrade_ctl_init();
        if (g_ota_upgrade_state_cb != NULL) {
            g_ota_upgrade_state_cb(OTA_UPGRADE_STATE_END, result);
        }
    }
}

void ota_upgrade_timer_handler(unsigned long data)
{
    UNUSED(data);
    if (g_ota_upgrade_ctrl.ota_start_sec == 0) {
        ota_log_print("[OTA] upgrade not start yet\r\n");
        return;
    }
    uint32_t cur_sec = ota_get_cur_second();
    uint32_t start_sec = g_ota_upgrade_ctrl.ota_start_sec;
    uint32_t trans_finish_sec = g_ota_upgrade_ctrl.ota_trans_finish_sec;

    if (trans_finish_sec != 0) {
        if (g_ota_upgrade_ctrl.ota_trans_result != ERRCODE_SUCC) {
            if (g_ota_upgrade_state_cb != NULL) {
                g_ota_upgrade_state_cb(OTA_UPGRADE_STATE_END, g_ota_upgrade_ctrl.ota_trans_result);
            }
            ota_upgrade_ctl_init();
            return;
        }
        if ((cur_sec - trans_finish_sec) > OTA_UPGRADE_TIMEOUT) {
            ota_log_print("[OTA] transmit finish but not receive upgrade cmd in %d seconds\r\n", OTA_UPGRADE_TIMEOUT);
            if (g_ota_upgrade_state_cb != NULL) {
                g_ota_upgrade_state_cb(OTA_UPGRADE_STATE_END, ERRCODE_UPG_TIMEOUT);
            }
            ota_upgrade_ctl_init();
            return;
        }
        osal_timer_start(&g_ota_upgrade_timer);
        return;
    }
    if ((cur_sec - start_sec) > OTA_PREPARE_TRANS_TIMEOUT) {
        ota_log_print("[OTA] not trans finish in %d seconds,stop upgrade\r\n", OTA_PREPARE_TRANS_TIMEOUT);
        uapi_transmit_device_stop();
        if (g_ota_upgrade_state_cb != NULL) {
            g_ota_upgrade_state_cb(OTA_UPGRADE_STATE_END, ERRCODE_TRNG_TIMEOUT);
        }
        ota_upgrade_ctl_init();
        return;
    }
    osal_timer_start(&g_ota_upgrade_timer);
}

uint32_t ota_get_cur_second(void)
{
    osal_timeval cur_time = { 0 };
    cur_time.tv_sec = uapi_tcxo_get_ms() / OTA_UPGRADE_1000MS;
    cur_time.tv_usec = 0;
    return cur_time.tv_sec;
}
