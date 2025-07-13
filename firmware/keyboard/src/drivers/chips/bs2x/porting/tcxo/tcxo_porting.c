/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides tcxo port \n
 *
 * History: \n
 * 2022-08-16， Create file. \n
 */

#include "tcxo_porting.h"
#include "hal_tcxo.h"
#include "hal_tcxo_v150.h"

static uintptr_t g_tcxo_base_addr =  (uintptr_t)TCXO_COUNT_BASE_ADDR;

uintptr_t tcxo_porting_base_addr_get(void)
{
    return g_tcxo_base_addr;
}