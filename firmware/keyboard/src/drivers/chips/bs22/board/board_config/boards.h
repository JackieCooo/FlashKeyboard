/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description:  board config
 *
 * Create: 2023-08-23
 */
#ifndef BOARDS_H
#define BOARDS_H

#ifdef BOARD_EVB
#include "board_evb.h"
#elif defined(BOARD_FPGA)
#include "board_fpga.h"
#endif

#endif