/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151:macro defined for interface
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */

#ifndef MACRO_INTERFACE_H
#define MACRO_INTERFACE_H

#define CONV_INV 200

#define ERROR_INDEX1 1
#define ERROR_INDEX2 2
#define ERROR_INDEX3 3
#define ERROR_INDEX4 4
#define ERROR_INDEX5 5
#define ERROR_INDEX6 6
#define ERROR_INDEX7 7
#define ERROR_INDEX8 8
#define ERROR_INDEX9 9
#define ERROR_INDEX10 10
#define ERROR_INDEX11 11
#define ERROR_INDEX12 12
#define ERROR_INDEX13 13
#define ERROR_INDEX14 14
#define ERROR_INDEX15 15
#define ERROR_INDEX16 16
#define ERROR_INDEX17 17
#define ERROR_INDEX18 18
#define ERROR_MEMORY_ALLOC 19

#define CACHELINE_SIZE 128

#define TRANS_N 0
#define TRANS_T 1
#define TRANS_C 2
#define TRANS_R 3

#define TRANSN 0
#define TRANST 1
#define TRANSR 2
#define TRANSC 3

#define UPLO_U 0
#define UPLO_L 1
#define UPLO_V 2
#define UPLO_M 3

#define DIAG_U 0
#define DIAG_N 1

#define SIDE_L 0
#define SIDE_R 1

#define BITMASK3 3
#define BITMASK4 4

#define INFO_INIT 0

#define UPDATE_DELTA(x, n, incx)                    \
    do {                                            \
        if ((incx) < 0) {                           \
            (x) -= (1 * ((n)-1) * (incx)*COMPSIZE); \
        }                                           \
    } while (0)

#define CBLAS_COL_TRANS_CONV(transA, trans) \
    do {                                    \
        if (*(transA) == 'N') {             \
            trans = TRANS_N;                \
        } else if (*(transA) == ('C')) {    \
            trans = TRANS_C;                \
        } else if (*(transA) == ('T')) {    \
            trans = TRANS_T;                \
        } else if (*(transA) == ('R')) {    \
            trans = TRANS_R;                \
        }                                   \
    } while (0)

#define CBLAS_COL_UPLO_CONV(extUplo, InUplo) \
    do {                                     \
        if (*(extUplo) == ('U')) {           \
            InUplo = UPLO_U;                 \
        } else if (*(extUplo) == ('L')) {    \
            InUplo = UPLO_L;                 \
        }                                    \
    } while (0)

#define CBLAS_DIAG_CONV(ExtDiag, InDiag)  \
    do {                                  \
        if (*(ExtDiag) == ('U')) {        \
            InDiag = DIAG_U;              \
        } else if (*(ExtDiag) == ('L')) { \
            InDiag = DIAG_N;              \
        }                                 \
    } while (0)

/* below macroes are only used for trmv/trsv */
#define TRXV_VALIDATE_PARAM(info, incx, lda, n, unit, trans, uplo) \
    do {                                                           \
        if ((incx) == 0) {                                         \
            info = ERROR_INDEX8;                                   \
        }                                                          \
        if ((lda) < MAX(1, (n))) {                                 \
            info = ERROR_INDEX6;                                   \
        }                                                          \
        if ((n) < 0) {                                             \
            info = ERROR_INDEX4;                                   \
        }                                                          \
        if ((unit) == CONV_INV) {                                  \
            info = ERROR_INDEX3;                                   \
        }                                                          \
        if ((trans) == CONV_INV) {                                 \
            info = ERROR_INDEX2;                                   \
        }                                                          \
        if ((uplo) == CONV_INV) {                                  \
            info = ERROR_INDEX1;                                   \
        }                                                          \
    } while (0)

#define TRXX_CBLAS_CONV(info, lda, n, trans, uplo, unit, TransA, diagArg, uploArg) \
    do {                                                                           \
        unit = (uplo) = (trans) = CONV_INV;                                        \
        CBLAS_DIAG_CONV(diagArg, unit);                                            \
        CBLAS_COL_UPLO_CONV(uploArg, uplo);                                        \
        CBLAS_COL_TRANS_CONV(TransA, trans);                                       \
        info = -1;                                                                 \
        TRXV_VALIDATE_PARAM(info, incx, lda, n, unit, trans, uplo);                \
    } while (0)

#define GET_ERRCODE_N2(info, n, uplo) \
    do {                              \
        if ((n) < 0) {                \
            info = ERROR_INDEX2;      \
        }                             \
        if ((uplo) == CONV_INV) {     \
            info = ERROR_INDEX1;      \
        }                             \
    } while (0)

#define GET_ERRCODE_X5(info, incx, n, uplo) \
    do {                                    \
        if ((incx) == 0) {                  \
            info = ERROR_INDEX5;            \
        }                                   \
        GET_ERRCODE_N2(info, n, uplo);      \
    } while (0)

#define GET_ERRCODE_YX7(info, incy, incx, n, uplo) \
    do {                                           \
        if ((incy) == 0) {                         \
            info = ERROR_INDEX7;                   \
        }                                          \
        GET_ERRCODE_X5(info, incx, n, uplo);       \
    } while (0)

#define GET_ERRCODE_YXA9(info, incy, incx, lda, n, uplo) \
    do {                                                 \
        if ((lda) < MAX(1, (n))) {                       \
            info = ERROR_INDEX9;                         \
        }                                                \
        GET_ERRCODE_YX7(info, incy, incx, n, uplo);      \
    } while (0)

#define GET_ERRCODE_YXA10(info, incy, incx, lda, n, uplo) \
    do {                                                  \
        if ((incy) == 0) {                                \
            info = ERROR_INDEX10;                         \
        }                                                 \
        if ((incx) == 0) {                                \
            info = ERROR_INDEX7;                          \
        }                                                 \
        if ((lda) < MAX(1, (n))) {                        \
            info = ERROR_INDEX5;                          \
        }                                                 \
        GET_ERRCODE_N2(info, n, uplo);                    \
    } while (0)

#define CHECK_CBLAS_COL_UAXY(info, uploArg, uplo, incx, incy, lda, n) \
    do {                                                              \
        CBLAS_COL_UPLO_CONV(uploArg, uplo);                           \
        info = -1;                                                    \
        GET_ERRCODE(info, incy, incx, lda, n, uplo);                  \
    } while (0)

#define UPDATE_XY(x, y, n, incx, incy) \
    do {                               \
        UPDATE_DELTA(x, n, incx);      \
        UPDATE_DELTA(y, n, incy);      \
    } while (0)

#define SET_SIDE_VAL_WITH_COLMAJ(sideArg, side) \
    do {                                        \
        if (*(sideArg) == ('L')) {              \
            side = SIDE_L;                      \
        } else if (*(sideArg) == ('R')) {       \
            side = SIDE_R;                      \
        } else {                                \
            side = CONV_INV;                    \
        }                                       \
    } while (0)
#define SET_UPLO_VAL_WITH_COLMAJ(uploArg, uplo) \
    do {                                        \
        if (*(uploArg) == ('U')) {              \
            uplo = UPLO_U;                      \
        } else if (*(uploArg) == ('L')) {       \
            uplo = UPLO_L;                      \
        } else {                                \
            uplo = CONV_INV;                    \
        }                                       \
    } while (0)
#define SET_SIDE_AND_UPLO_WITH_COLMAJ(sideArg, side, uploArg, uplo) \
    do {                                                            \
        SET_SIDE_VAL_WITH_COLMAJ(sideArg, side);                    \
        SET_UPLO_VAL_WITH_COLMAJ(uploArg, uplo);                    \
    } while (0)

// level2
#define CALC_COMPLEX_R(aReal, aImag)                                             \
    do {                                                                         \
        float ratio_, den_;                                                      \
        if (fabs((aReal)) >= fabs((aImag))) {                                    \
            ratio_ = ((aReal) != ZERO_FLOAT) ? ((aImag) / (aReal)) : ZERO_FLOAT; \
            den_ = ONE_FLOAT / ((aReal) * (1 + ratio_ * ratio_));                \
            (aReal) = den_;                                                      \
            (aImag) = -ratio_ * den_;                                            \
        } else {                                                                 \
            ratio_ = ((aImag) != ZERO_FLOAT) ? ((aReal) / (aImag)) : ZERO_FLOAT; \
            den_ = ONE_FLOAT / ((aImag) * (1 + ratio_ * ratio_));                \
            (aReal) = ratio_ * den_;                                             \
            (aImag) = -den_;                                                     \
        }                                                                        \
    } while (0)

#define CALC_COMPLEX_C(aReal, aImag)                                             \
    do {                                                                         \
        float ratio_, den_;                                                      \
        if (fabs((aReal)) >= fabs((aImag))) {                                    \
            ratio_ = ((aReal) != ZERO_FLOAT) ? ((aImag) / (aReal)) : ZERO_FLOAT; \
            den_ = ONE_FLOAT / ((aReal) * (1 + ratio_ * ratio_));                \
            (aReal) = den_;                                                      \
            (aImag) = ratio_ * den_;                                             \
        } else {                                                                 \
            ratio_ = ((aImag) != ZERO_FLOAT) ? ((aReal) / (aImag)) : ZERO_FLOAT; \
            den_ = ONE_FLOAT / ((aImag) * (1 + ratio_ * ratio_));                \
            (aReal) = ratio_ * den_;                                             \
            (aImag) = den_;                                                      \
        }                                                                        \
    } while (0)
#endif