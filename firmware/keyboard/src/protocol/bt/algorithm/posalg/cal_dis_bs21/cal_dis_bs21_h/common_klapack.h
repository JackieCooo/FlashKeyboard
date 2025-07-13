/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151:common macro header file
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */
#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include "f2c.h"
#include "blaswrap.h"
#include "clapack.h"

// REVICE ZCY
#define COMPLEX

#define SLAMCH_P_PREC 1.19209289550781250000e-07f   // eps*base
#define SLAMCH_E_EPS 5.96046447753906250000e-08f    // relative machine precision
#define SLAMCH_S_SFMIN 1.17549435082228750797e-38f  // safe minimum, such that 1/sfmin does not overflow
#define DLAMCH_P_PREC 2.22044604925031308085e-16    // eps*base
#define DLAMCH_E_EPS 1.11022302462515654042e-16     // relative machine precision
#define DLAMCH_S_SFMIN 2.22507385850720138309e-308  // safe minimum, such that 1/sfmin does not overflow

// transpose param
#define NO_TRANSPOSE "N"
#define TRANSPOSE "T"
#define TRANSPOSE_CONJUGATE_COM "C"

// jobz param
#define EIGENVALUES "N"
#define EIGENVECTORS "V"
#define EIGENVECTORS_TRIDIAG "I"
#define EIGENVECTORS_SYEVD "U"

// uplo param
#define UPPER "U"
#define LOWER "L"
#define FULL "F"

// diag param
#define UNIT "U"
#define NON_UNIT "N"

// side param
#define LEFT "L"
#define RIGHT "R"

// direct param
#define FORWARD_DIR "F"
#define BACKWARD_DIR "B"

// wise param
#define COLUMNWISE "C"
#define ROWWISE "R"

#define MAX_ABS_NORM "M"

#define LQUERY (-1)

#define CONCAT2(a, b) CONCA_2(a, b)
#define CONCA_2(a, b) a##b

#define AUTOTYPE_MIN(a, b, _a, _b)                   \
    ({                                               \
        __auto_type (_a) = (a);                        \
        __auto_type (_b) = (b);                        \
        ((_a) != (_a) || (_a) < (_b)) ? (_a) : (_b); \
    })

#define AUTOTYPE_MAX(a, b, _a, _b)                   \
    ({                                               \
        __auto_type (_a) = (a);                        \
        __auto_type (_b) = (b);                        \
        ((_a) != (_a) || (_a) > (_b)) ? (_a) : (_b); \
    })

static inline size_t calc_index(size_t i, size_t j, size_t ld)
{
    return i + j * ld;
}

#ifndef LIKELY
#define LIKELY(x) (!!__builtin_expect(!!(x), 1))
#endif

#ifndef UNLIKELY
#define UNLIKELY(x) (!!__builtin_expect(!!(x), 0))
#endif

#ifdef COMPLEX

#define TRANSPOSE_CONJUGATE "C"
#define NO_TRANSPOSE_CONJUGATE "R"
// REVISE ZCY
static inline complex CONJ(complex z)
{
    complex a = { z.r, -z.i };
    return a;
}
#define REAL(x) ((x).r)
#define IMAG(x) ((x).r)
static inline complex MINUS(complex z)
{
    complex a = { -z.r, -z.i };
    return a;
}
static inline complex DOT(complex x, complex y)
{
    complex a = { x.r * x.r - x.i * x.i, x.r * y.i + x.i * y.r };
    return a;
}

#define RSIGN copysignf
#define RSQRT sqrtf

#define HALF KML_CMPLX(0.5f, 0.f)
#define ONE_KML_TYPE KML_CMPLX(1.f, 0.f)
#define TWO_KML_TYPE KML_CMPLX(2.f, 0.f)
#define MONE KML_CMPLX(-1.f, -0.f)
#define ZERO_KML_TYPE KML_CMPLX(0.f, 0.f)

#define RHALF 0.5f
#define RONE 1.f
#define MRONE (-1.f)
#define RTWO 2.f
#define RZERO 0.f

#define FABS cabsf
#define RFABS fabsf

#define KML_CMPLX CMPLXF
#ifndef CMPLXF
#define CMPLXF CMPLXF
// static inline float _Complex CMPLXF(float a, float b)
static inline complex CMPLXF(float a, float b)
{
#ifndef __cplusplus
    complex c = { a, b };
    return c;
#else
    std::complex<float> val(a, b);
    return *reinterpret_cast<float _Complex *>(&val);
#endif  // __cplusplus
}
#endif  // CMPLXF

#endif  // COMPLEX

static inline float Int2FloatCeil(size_t i)
{
    float rv = (float)i;
    if (i > (size_t)rv) {
        uint32_t *irv = (uint32_t *)&rv;
        ++*(irv);
    }
    return rv;
}

#define LAMCH_EPS SLAMCH_E_EPS
#define LAMCH_SAFEMIN SLAMCH_S_SFMIN
#define LAMCH_PREC SLAMCH_P_PREC
#define LWORK_MAX 2147483520
#define INT_CONVERT_TO_FTYPE(x) Int2FloatCeil(x)

#define CONVERT_TO_INT(x) ((int32_t)(*((float *)&(x))))

typedef struct work_size_status {
    float work_size;
    bool is_converted;
} work_size_status_t;

#endif  // COMMON_H
