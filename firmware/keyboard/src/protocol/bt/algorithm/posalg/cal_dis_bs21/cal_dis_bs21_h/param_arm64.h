/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151:param setting for arm64 kernel
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */

#ifndef PARAM_ARM64_H
#define PARAM_ARM64_H

#define COMPLEX_OFFSET 2

#define UNROLL_SHIFT5 5
#define UNROLL_TIME32 32
#define UNROLL_OFFSET31 31
#define UNROLL_BITMASK31 31

#define UNROLL_SHIFT4 4
#define UNROLL_TIME16 16
#define UNROLL_OFFSET15 15
#define UNROLL_BITMASK15 15

// trsm kernel params
#if defined(GEMM_DEFAULT_UNROLL_M) && GEMM_DEFAULT_UNROLL_M == 16
#define GEMM_UNROLL_M_SHIFT 4
#endif

#if defined(GEMM_DEFAULT_UNROLL_M) && GEMM_DEFAULT_UNROLL_M == 8
#define GEMM_UNROLL_M_SHIFT 3
#endif

#if defined(GEMM_DEFAULT_UNROLL_M) && GEMM_DEFAULT_UNROLL_M == 6
#define GEMM_UNROLL_M_SHIFT 2
#endif

#if defined(GEMM_DEFAULT_UNROLL_M) && GEMM_DEFAULT_UNROLL_M == 4
#define GEMM_UNROLL_M_SHIFT 2
#endif

#if defined(GEMM_DEFAULT_UNROLL_M) && GEMM_DEFAULT_UNROLL_M == 2
#define GEMM_UNROLL_M_SHIFT 1
#endif

#if defined(GEMM_DEFAULT_UNROLL_M) && GEMM_DEFAULT_UNROLL_M == 1
#define GEMM_UNROLL_M_SHIFT 0
#endif

#if defined(GEMM_DEFAULT_UNROLL_N) && GEMM_DEFAULT_UNROLL_N == 16
#define GEMM_UNROLL_N_SHIFT 4
#endif

#if defined(GEMM_DEFAULT_UNROLL_M) && GEMM_DEFAULT_UNROLL_M == 32
#define GEMM_UNROLL_M_SHIFT 5
#endif

#if defined(GEMM_DEFAULT_UNROLL_N) && GEMM_DEFAULT_UNROLL_N == 32
#define GEMM_UNROLL_N_SHIFT 5
#endif

#if defined(GEMM_DEFAULT_UNROLL_N) && GEMM_DEFAULT_UNROLL_N == 8
#define GEMM_UNROLL_N_SHIFT 3
#endif

#if defined(GEMM_DEFAULT_UNROLL_N) && GEMM_DEFAULT_UNROLL_N == 4
#define GEMM_UNROLL_N_SHIFT 2
#endif

#if defined(GEMM_DEFAULT_UNROLL_N) && GEMM_DEFAULT_UNROLL_N == 2
#define GEMM_UNROLL_N_SHIFT 1
#endif

#if defined(GEMM_DEFAULT_UNROLL_N) && GEMM_DEFAULT_UNROLL_N == 1
#define GEMM_UNROLL_N_SHIFT 0
#endif

#endif