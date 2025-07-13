/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: diag cmd dispatch
 * This file should be changed only infrequently and with great care.
 */

#include "diag_cmd_dispatch.h"
#include "diag.h"
#include "diag_adapt_layer.h"
#include "dfx_adapt_layer.h"
#include "errcode.h"
#include "diag_cmd_dst.h"

errcode_t uapi_zdiag_cmd_process(diag_ser_data_t *data)
{
    errcode_t ret;
    diag_option_t option = DIAG_OPTION_INIT_VAL;
    option.peer_addr = data->header.src;

    ret = diag_pkt_router_run_cmd(data, &option);
    return ret;
}