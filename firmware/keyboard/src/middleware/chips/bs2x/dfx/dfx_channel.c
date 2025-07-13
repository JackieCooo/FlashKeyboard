/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: dfx channel
 * This file should be changed only infrequently and with great care.
 */
#include <stdint.h>
#include "dfx_adapt_layer.h"
#include "diag.h"
#include "diag_ind_src.h"
#include "diag_msg.h"
#include "diag_channel.h"
#include "diag_adapt_layer.h"
#include "log_uart.h"
#include "dfx_channel.h"
#if defined(CONFIG_BT_UPG_ENABLE) && (CONFIG_BT_UPG_ENABLE == 1)
#include "bts_def.h"
#endif
#if defined(CONFIG_SLE_UPG_ENABLE) && (CONFIG_SLE_UPG_ENABLE== 1)
#include "sle_ota.h"
#endif

#define DFX_DIAG_CHANNEL DIAG_CHANNEL_ID_0

#if defined(CONFIG_BT_UPG_ENABLE) && (CONFIG_BT_UPG_ENABLE == 1)
errcode_t bth_ota_reg_cbk(void *data_report, void *status_report);
errcode_t bth_ota_data_send(uint8_t *data_ptr, uint16_t data_len);
#endif

#if defined(CONFIG_SUPPORT_LOG_THREAD)
void diag_uart_rx_proc(uint8_t *buffer, uint16_t length)
{
    uapi_diag_channel_rx_mux_char_data(DFX_DIAG_CHANNEL, buffer, length);
}

static int32_t diag_channel_uart_output(void *fd, dfx_data_type_t data_type,
                                        uint8_t *data[], uint16_t len[], uint8_t cnt)
{
    unused(fd);
    unused(data_type);
    for (uint8_t i = 0; i < cnt; i++) {
#if defined(CONFIG_UART_SUPPORT_TX_INT) && (!defined(CONFIG_UART_LOG_WRITE_WITH_NOLOCK))
        if (osal_in_interrupt() || data_type == DFX_DATA_DIAG_PKT_CRITICAL) {
            log_uart_send_buffer(data[i], len[i]);
        } else {
            log_uart_write_blocking(data[i], len[i]);
        }
#else
        log_uart_send_buffer(data[i], len[i]);
#endif
    }
    return ERRCODE_SUCC;
}
#endif

#if defined(CONFIG_BT_UPG_ENABLE) && (CONFIG_BT_UPG_ENABLE == 1)
static int32_t diag_channel_bt_output(void *fd, dfx_data_type_t data_type, uint8_t *data[], uint16_t len[], uint8_t cnt)
{
    unused(fd);
    unused(data_type);
    unused(cnt);
    return (int32_t)bth_ota_data_send(data[0], len[0]);
}

static errcode_t diag_channel_rx_bt_data(uint8_t *data, uint16_t size)
{
    return uapi_diag_channel_rx_frame_data(DIAG_CHANNEL_ID_1, data, size);
}

static void diag_channel_register_bt_callback(void)
{
    bth_ota_reg_cbk(diag_channel_rx_bt_data, NULL);
}
#endif

#if defined(CONFIG_SLE_UPG_ENABLE) && (CONFIG_SLE_UPG_ENABLE== 1)
static int32_t diag_channel_sle_output(void *fd, dfx_data_type_t type, uint8_t *data[], uint16_t len[], uint8_t cnt)
{
    unused(fd);
    unused(type);
    unused(cnt);
    int32_t size = len[0];
    if (sle_ota_data_ack(len[0], (uint8_t *)data[0]) != ERRCODE_SUCC) {
        size = 0;
    }
    return size;
}

static void diag_channel_rx_sle_data(const uint8_t *data, const uint16_t size)
{
    (void)uapi_diag_channel_rx_frame_data(DIAG_CHANNEL_ID_2, (uint8_t *)data, (uint16_t)size);
}

static void diag_channel_register_sle_callback(void)
{
    sle_ota_reg_chan_data_report_cbk(diag_channel_rx_sle_data);
}
#endif

errcode_t diag_register_channel(void)
{
    if (uapi_diag_channel_init(DFX_DIAG_CHANNEL, DIAG_CHANNEL_ATTR_NEED_RX_BUF) != ERRCODE_SUCC) {
        return ERRCODE_FAIL;
    }
#if defined(CONFIG_SUPPORT_LOG_THREAD)
    if (uapi_diag_channel_set_connect_hso_addr(DFX_DIAG_CHANNEL, diag_adapt_get_default_dst()) !=
        (errcode_t)ERRCODE_SUCC) {
        return ERRCODE_FAIL;
    }
    if (uapi_diag_channel_set_tx_hook(DFX_DIAG_CHANNEL, diag_channel_uart_output) != ERRCODE_SUCC) {
        return ERRCODE_FAIL;
    }
#endif
#if defined(CONFIG_BT_UPG_ENABLE) && (CONFIG_BT_UPG_ENABLE == 1)
    if (uapi_diag_channel_init(DIAG_CHANNEL_ID_1, DIAG_CHANNEL_ATTR_NONE) != ERRCODE_SUCC) {
        return ERRCODE_FAIL;
    }

    if (uapi_diag_channel_set_tx_hook(DIAG_CHANNEL_ID_1, diag_channel_bt_output) != ERRCODE_SUCC) {
        return ERRCODE_FAIL;
    }
    diag_channel_register_bt_callback();
#endif
#if defined(CONFIG_SLE_UPG_ENABLE) && (CONFIG_SLE_UPG_ENABLE== 1)
    if (uapi_diag_channel_init(DIAG_CHANNEL_ID_2, DIAG_CHANNEL_ATTR_NONE) != (errcode_t)ERRCODE_SUCC) {
        return ERRCODE_FAIL;
    }

    if (uapi_diag_channel_set_tx_hook(DIAG_CHANNEL_ID_2, diag_channel_sle_output) != (errcode_t)ERRCODE_SUCC) {
        return ERRCODE_FAIL;
    }
    diag_channel_register_sle_callback();
#endif
    return ERRCODE_SUCC;
}

errcode_t dfx_msg_write(uint32_t msg_id, uint8_t *msg, uint16_t msg_len, bool wait)
{
    unused(wait);
    uint8_t msg_data[DFX_MSG_MAX_SIZE + DFX_MSG_ID_LEN];

    *(uint32_t*)msg_data = msg_id;

    if ((msg != NULL) && (memcpy_s(&msg_data[DFX_MSG_ID_LEN], DFX_MSG_MAX_SIZE, msg, msg_len) != EOK)) {
        return ERRCODE_FAIL;
    }
    uint32_t message_id = *((uint32_t*)&msg_data[0]);

    msg_process_proc(message_id, &msg_data[DFX_MSG_ID_LEN], DFX_MSG_MAX_SIZE);
    return ERRCODE_SUCC;
}

uint32_t diag_adapt_get_msg_local_time(void)
{
    time_t time_s = (time_t)dfx_get_cur_second();

    struct tm tm = { 0 };
    if (localtime_r(&time_s, &tm) == NULL) {
        return 0;
    }

    uint32_t local_time = (tm.tm_mon + 1) << 28; /* 28~31 bit: month */
    /* 1 day = 24 hours, 1 hour = 60 minutes, 1 minute = 60 seconds, 1 second = 100 * 10 milliseconds */
    local_time += (((tm.tm_mday * 24 + tm.tm_hour) * 60 + tm.tm_min) * 60 + tm.tm_sec) * 100;
    return local_time;
}