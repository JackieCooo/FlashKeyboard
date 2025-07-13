/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description:  common logging producer interface - need to change name of log.h in all the protocol core files
 *
 * Create:
 */

#ifndef _LOG_DEF_ALG_H_
#define _LOG_DEF_ALG_H_

typedef enum {
// only for alg extern part
    ALG_3IN1_POS_INOUT_FUS_C,
    PIPELINE_C,
    BASICMATH_C,
    ALG_SMOOTH_DIS_C,
    DISALG_C,
    XERBLA_C,
    SLAMCH_C,
    F77_ALOC_C,
    FMT_C,
    WRTFMT_C,
    ALG_FILE_ID_MAX = 1024,
} log_file_list_enum_alg_t;
#endif