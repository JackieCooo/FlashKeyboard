/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151:cheev/sheev interface
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */
#ifndef LIB_CONFIG_H
#define LIB_CONFIG_H

typedef void *(*alg_malloc_cbk)(size_t size);
typedef void *(*alg_free_cbk)(void *ptr);

void set_alg_space_alloc_func(alg_malloc_cbk malloc_cbk, alg_free_cbk free_cbk);
#endif