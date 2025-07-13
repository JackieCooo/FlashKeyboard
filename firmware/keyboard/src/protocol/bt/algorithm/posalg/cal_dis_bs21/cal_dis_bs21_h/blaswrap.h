/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100 origin from spi_v151
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 */
#ifndef __BLASWRAP_H
#define __BLASWRAP_H

#include <stdint.h>
#include <stdbool.h>

int32_t cscal_k(int32_t *n, float *da_r, float *da_i, float *x, int32_t *incx, float *y, int32_t *incy);
void cscal_k_b0(int32_t *n, float *, int32_t *incy);
int32_t caxpy_k(int32_t n, float da_r, float da_i, float *x, int32_t incx, float *y, int32_t incy, bool conj_flag);
int32_t ccopy_k(int32_t n, float *x, int32_t incx, float *y, int32_t incy);
int32_t cgemv_t(int32_t m, int32_t n, float alpha_r, float alpha_i, float *a, int32_t lda, float *x, int32_t inc_x,
                float *y, int32_t inc_y, float *buffer, bool conj_flag, bool xconj_flag);
int32_t cgemv_n(int32_t m, int32_t n, float alpha_r, float alpha_i, float *a, int32_t lda, float *x, int32_t inc_x,
                float *y, int32_t inc_y, float *buffer, bool conj_flag, bool xconj_flag);
void cdot_kernel_16(int32_t n, float *x, float *y, float *d);

#endif /* __BLASWRAP_H */
