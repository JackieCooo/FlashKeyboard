/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151:cheev/sheev header file
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */
#ifndef LAPACK_SYEV_H
#define LAPACK_SYEV_H

#include "common_klapack.h"

void cheev_check(char *jobz, char *uplo, int32_t *n, complex *a, int32_t *lda, float *w, complex *work, int32_t *lwork,
                 float *rwork, int32_t *info);

#endif  // LAPACK_SYEV_H
