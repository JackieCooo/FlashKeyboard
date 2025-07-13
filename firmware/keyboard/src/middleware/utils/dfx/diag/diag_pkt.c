/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: zdiag pkt
 * This file should be changed only infrequently and with great care.
 */
#include "diag_pkt.h"
#include "diag_adapt_layer.h"
#include "diag.h"
#include "diag_filter.h"
#include "errcode.h"
#include "common_def.h"

errcode_t diag_check_mux_pkt(const diag_router_frame_t *frame, uint16_t size)
{
    unused(size);
    if (frame->sof != DIAG_ROUTER_FRAME_START_FLAG) {
        return ERRCODE_FAIL;
    }

    return ERRCODE_SUCC;
}

void diag_pkt_handle_init(diag_pkt_handle_t *pkt, uint8_t data_cnt)
{
    memset_s(pkt, sizeof(diag_pkt_handle_t), 0, sizeof(diag_pkt_handle_t));
    pkt->data_cnt = data_cnt;
}

void diag_pkt_handle_set_data(diag_pkt_handle_t *pkt, uint8_t idx, uint8_t *data, uint16_t data_len,
    diag_pkt_data_t attribute)
{
    unused(attribute);
    pkt->data[idx] = data;
    pkt->data_len[idx] = data_len;

    if (((uint32_t)attribute & DIAG_PKT_DATA_ATTRIBUTE_SINGLE_TASK) != 0) {
        pkt->single_task = true;
    }

    if (((uint32_t)attribute & DIAG_PKT_DATA_ATTRIBUTE_DYN_MEM) != 0) {
        pkt->need_free = true;
    }
    return;
}

#if (defined CONFIG_DFX_SUPPORT_CUSTOM_LOG) && (CONFIG_DFX_SUPPORT_CUSTOM_LOG == DFX_YES)
void diag_mk_log_pkt_custom(diag_cmd_log_layer_stru_t *log_pkt, uint32_t module_id, uint32_t dst_id, uint32_t msg_id)
{
    STATIC uint32_t msg_sn = 0;
    log_pkt->module = module_id;
    log_pkt->dest_mod = dst_id;
    log_pkt->no = msg_sn++;
    log_pkt->id = msg_id;
    log_pkt->time = diag_adapt_get_msg_time();
}
#endif

void diag_mk_log_pkt(diag_cmd_log_layer_t *log_pkt, uint32_t module_id, uint32_t msg_id, uint32_t sn, bool sn_enable)
{
    log_pkt->id = msg_id;
#if (CONFIG_DFX_SUPPORT_DIAG_MINIMAL_MSG == DFX_YES)
    unused(module_id);
    STATIC uint16_t msg_sn = 0;
    log_pkt->no = (sn_enable) ? (uint16_t)sn : (msg_sn++);
#elif (CONFIG_DFX_SUPPORT_DIAG_NORMAL_MSG == DFX_YES)
    STATIC uint16_t msg_sn = 0;
    uint64_t time_ms = diag_adapt_get_msg_time_ms();
    uint16_t tmp_sn = (sn_enable) ? (uint16_t)sn : (msg_sn++);

    log_pkt->module = (uint8_t)module_id;
    log_pkt->no_lo = (uint8_t)(tmp_sn & 0xFF);          /* 0xFF： get lower 8 bits */
    log_pkt->no_hi = (uint8_t)((tmp_sn >> 8) & 0x3F);   /* 8, 0x3F： get upper 6 bits */
    log_pkt->time = (uint32_t)(time_ms / 1000);     /* 1 second = 1000 milliseconds */
    log_pkt->time_ms = (uint16_t)(time_ms % 1000);  /* 1 second = 1000 milliseconds */
#elif (CONFIG_DFX_SUPPORT_DIAG_EXTEND_MSG == DFX_YES)
    STATIC uint8_t msg_sn = 0;
    uint8_t sub_mod_id = (uint8_t)((module_id >> SUB_MOD_ID_OFFSET) & SUB_MOD_ID_MASK);
    log_pkt->sub_mod = sub_mod_id;
    log_pkt->no = (sn_enable) ? (uint8_t)sn : (msg_sn++);
    log_pkt->id = msg_id;
    log_pkt->time = diag_adapt_get_msg_local_time();
#else
    STATIC uint32_t msg_sn = 0;
    log_pkt->module = module_id;
    log_pkt->dest_mod = diag_adapt_get_local_addr();
    log_pkt->no = (sn_enable) ? sn : (msg_sn++);
    log_pkt->time = diag_adapt_get_msg_time();
#endif
}

void diag_mk_frame_header_1(diag_router_frame_t *frame, uint16_t pkt_size)
{
    diag_router_inner_frame_t *inner = (diag_router_inner_frame_t *)frame->inner;
    frame->sof      = DIAG_ROUTER_FRAME_START_FLAG;
    frame->ctrl     = 0;
    inner->len_msb  = (uint8_t)(pkt_size >> DIAG_FRAME_SHIFT_8);
    inner->len_lsb  = (uint8_t)(pkt_size);
}

uint32_t diag_pkt_handle_get_total_size(diag_pkt_handle_t *pkt)
{
    uint32_t total_size = 0;
    for (int i = 0; i < pkt->data_cnt; i++) {
        total_size += (uint32_t)pkt->data_len[i];
    }
    return total_size;
}

void diag_pkt_handle_clean(diag_pkt_handle_t *pkt)
{
    pkt->need_free = false;
    for (int i = 0; i < pkt->data_cnt; i++) {
        pkt->data[i] = NULL;
        pkt->data_len[i] = 0;
    }
    pkt->data_cnt = 0;
}