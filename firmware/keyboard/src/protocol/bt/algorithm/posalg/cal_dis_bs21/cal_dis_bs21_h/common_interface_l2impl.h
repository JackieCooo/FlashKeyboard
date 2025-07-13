/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151:level2 interface common implentations
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */

#ifndef COMMON_INTERFACE_LEVEL2_IMPL_H
#define COMMON_INTERFACE_LEVEL2_IMPL_H
#include "common.h"
#include "clapack.h"
#include "blaswrap.h"
#define PARAM_ERROR (-1)
#define PARAM_OK 0

static inline int32_t CheckInfoError(int32_t info, char *errname, int32_t sz)
{
    if (info >= 0) {
        xerbla_(errname, &info, (int16_t)sz);
        return PARAM_ERROR;
    }

    return PARAM_OK;
}
static inline int32_t DoBaseOperationAz(int32_t n, float *x, float *y, float *a, float *alpha)
{
    bool skip = (n == 0) || (x == NULL) || (y == NULL) || (a == NULL);
    if (skip == true) {
        return PARAM_ERROR;
    }

    if (FLOAT_IS_ZERO_FLOAT(alpha[0]) && FLOAT_IS_ZERO_FLOAT(alpha[1])) {
        return PARAM_ERROR;
    }

    return PARAM_OK;
}

static inline int32_t DoBaseOperationBaz(int32_t n, float *alpha, float *y, int32_t incy, float *beta)
{
    if (FLOAT_IS_ZERO_FLOAT((beta)[0]) && FLOAT_IS_ZERO_FLOAT((beta)[1])) {
        cscal_k_b0(&n, y, &incy);
    } else if ((!FLOAT_IS_ONE_FLOAT((beta)[0])) || (!FLOAT_IS_ZERO_FLOAT((beta)[1]))) {
        cscal_k(&n, &(beta)[0], &(beta)[1], y, &incy, NULL, NULL);
    }

    if (FLOAT_IS_ZERO_FLOAT(alpha[0]) && FLOAT_IS_ZERO_FLOAT(alpha[1])) {
        return PARAM_ERROR;
    }

    return PARAM_OK;
}
#endif  // COMMON_INTERFACE_LEVEL2_IMPL_H