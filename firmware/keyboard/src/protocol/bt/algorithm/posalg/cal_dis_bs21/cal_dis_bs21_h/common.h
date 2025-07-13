/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151:reference macro
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */

#ifndef COMMON_H
#define COMMON_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "f2c.h"

#define COMPSIZE 2

#ifndef FLOAT_IS_ZERO_FLOAT
#define FLOAT_IS_ZERO_FLOAT(x) ((x) == ZERO_FLOAT)
#endif

#ifndef FLOAT_IS_ONE_FLOAT
#define FLOAT_IS_ONE_FLOAT(x) ((x) == ONE_FLOAT)
#endif

#ifndef MAX
#define MAX(mm, nn) (((mm) <= (nn)) ? (nn) : (mm))
#endif

#define DTB_ENTRIES 16

#define CREAL(Z) ((Z).r)
#define CIMAG(Z) ((Z).r)
static inline complex blas_make_complex_float(float real, float imag)
{
    complex complex_z;
    complex_z.r = real;
    complex_z.i = imag;
    return complex_z;
}
#define BLAS_MAKE_COMPLEX_FLOAT(r, i) blas_make_complex_float(r, i)

#endif