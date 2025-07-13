/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100
 *              origin from spi_v151
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 * Notes: NA
 */

#ifndef SYMCOPY_H
#define SYMCOPY_H
#include "common.h"

#define SYMV_SMALLEST_BLOCK_SIZE 2
#define SYMV_BLOCK_2x2_COMPSIZE 4

#define REAL_BLOCK_2x2_ALL_COPY(m, tmp_a, aa, bb, cc) \
    do {                                              \
        REAL_TMPA_ASSIGNMENT(tmp_a, aa);              \
        REAL_CUR_BLOCK_COPY(tmp_a, bb);               \
        REAL_SYM_BLOCK_COPY(m, tmp_a, cc);            \
    } while (0)

#define REAL_TMPA_ASSIGNMENT(tmp_a, aa) \
    do {                                \
        (tmp_a)[0][0] = *((aa)[0]);     \
        (tmp_a)[1][0] = *((aa)[0] + 1); \
        (tmp_a)[0][1] = *((aa)[1]);     \
        (tmp_a)[1][1] = *((aa)[1] + 1); \
        CUR_OFFSET(aa);                 \
    } while (0)

#define REAL_CUR_BLOCK_COPY(tmp_a, bb)  \
    do {                                \
        *((bb)[0]) = (tmp_a)[0][0];     \
        *((bb)[0] + 1) = (tmp_a)[1][0]; \
        *((bb)[1]) = (tmp_a)[0][1];     \
        *((bb)[1] + 1) = (tmp_a)[1][1]; \
        CUR_OFFSET(bb);                 \
    } while (0)

#define REAL_SYM_BLOCK_COPY(m, tmp_a, cc) \
    do {                                  \
        *((cc)[0]) = (tmp_a)[0][0];       \
        *((cc)[0] + 1) = (tmp_a)[0][1];   \
        *((cc)[1]) = (tmp_a)[1][0];       \
        *((cc)[1] + 1) = (tmp_a)[1][1];   \
        SYM_OFFSET(m, cc);                \
    } while (0)

#define COMPLEX_BLOCK_2x2_ALL_COPY(FLAG1, FLAG2, m, tmp_a, aa, bb, cc) \
    do {                                                               \
        COMPLEX_TMPA_ASSIGNMENT(tmp_a, aa);                            \
        COMPLEX_CUR_BLOCK_COPY_##FLAG1(tmp_a, bb);                     \
        COMPLEX_SYM_BLOCK_COPY_##FLAG2(m, tmp_a, cc);                  \
    } while (0)

#define COMPLEX_TMPA_ASSIGNMENT(tmp_a, aa) \
    do {                                   \
        (tmp_a)[0][0] = *((aa)[0]);        \
        (tmp_a)[1][0] = *((aa)[0] + 1);    \
        (tmp_a)[2][0] = *((aa)[0] + 2);    \
        (tmp_a)[3][0] = *((aa)[0] + 3);    \
        (tmp_a)[0][1] = *((aa)[1]);        \
        (tmp_a)[1][1] = *((aa)[1] + 1);    \
        (tmp_a)[2][1] = *((aa)[1] + 2);    \
        (tmp_a)[3][1] = *((aa)[1] + 3);    \
        CUR_OFFSET(aa);                    \
    } while (0)

#define COMPLEX_CUR_BLOCK_COPY_EQ(tmp_a, bb) \
    do {                                     \
        *((bb)[0]) = (tmp_a)[0][0];          \
        *((bb)[0] + 1) = (tmp_a)[1][0];      \
        *((bb)[0] + 2) = (tmp_a)[2][0];      \
        *((bb)[0] + 3) = (tmp_a)[3][0];      \
        *((bb)[1]) = (tmp_a)[0][1];          \
        *((bb)[1] + 1) = (tmp_a)[1][1];      \
        *((bb)[1] + 2) = (tmp_a)[2][1];      \
        *((bb)[1] + 3) = (tmp_a)[3][1];      \
        CUR_OFFSET(bb);                      \
    } while (0)

#define COMPLEX_CUR_BLOCK_COPY_HE(tmp_a, bb) \
    do {                                     \
        *((bb)[0]) = (tmp_a)[0][0];          \
        *((bb)[0] + 1) = -(tmp_a)[1][0];     \
        *((bb)[0] + 2) = (tmp_a)[2][0];      \
        *((bb)[0] + 3) = -(tmp_a)[3][0];     \
        *((bb)[1]) = (tmp_a)[0][1];          \
        *((bb)[1] + 1) = -(tmp_a)[1][1];     \
        *((bb)[1] + 2) = (tmp_a)[2][1];      \
        *((bb)[1] + 3) = -(tmp_a)[3][1];     \
        CUR_OFFSET(bb);                      \
    } while (0)

#define COMPLEX_SYM_BLOCK_COPY_EQ(m, tmp_a, cc) \
    do {                                        \
        *((cc)[0]) = (tmp_a)[0][0];             \
        *((cc)[0] + 1) = (tmp_a)[1][0];         \
        *((cc)[0] + 2) = (tmp_a)[0][1];         \
        *((cc)[0] + 3) = (tmp_a)[1][1];         \
        *((cc)[1]) = (tmp_a)[2][0];             \
        *((cc)[1] + 1) = (tmp_a)[3][0];         \
        *((cc)[1] + 2) = (tmp_a)[2][1];         \
        *((cc)[1] + 3) = (tmp_a)[3][1];         \
        SYM_OFFSET(m, cc);                      \
    } while (0)

#define COMPLEX_SYM_BLOCK_COPY_HE(m, tmp_a, cc) \
    do {                                        \
        *((cc)[0]) = (tmp_a)[0][0];             \
        *((cc)[0] + 1) = -(tmp_a)[1][0];        \
        *((cc)[0] + 2) = (tmp_a)[0][1];         \
        *((cc)[0] + 3) = -(tmp_a)[1][1];        \
        *((cc)[1]) = (tmp_a)[2][0];             \
        *((cc)[1] + 1) = -(tmp_a)[3][0];        \
        *((cc)[1] + 2) = (tmp_a)[2][1];         \
        *((cc)[1] + 3) = -(tmp_a)[3][1];        \
        SYM_OFFSET(m, cc);                      \
    } while (0)

#define COMPLEX_SYMV_COPY_L_1(FLAG1, FLAG2, m, tmp_a, aa, bb, cc) \
    do {                                                          \
        if ((uint32_t)(m) & 1) {                                  \
            (tmp_a)[0][0] = *((aa)[0]);                           \
            (tmp_a)[1][0] = *((aa)[0] + 1);                       \
            (tmp_a)[0][1] = *((aa)[1]);                           \
            (tmp_a)[1][1] = *((aa)[1] + 1);                       \
            COMPLEX_CUR_COPY_L_1_##FLAG1(tmp_a, bb);              \
            COMPLEX_SYM_COPY_L_1_##FLAG2(tmp_a, cc);              \
        }                                                         \
    } while (0)

#define COMPLEX_CUR_COPY_L_1_EQ(tmp_a, bb) \
    do {                                   \
        *((bb)[0]) = (tmp_a)[0][0];        \
        *((bb)[0] + 1) = (tmp_a)[1][0];    \
        *((bb)[1]) = (tmp_a)[0][1];        \
        *((bb)[1] + 1) = (tmp_a)[1][1];    \
    } while (0)

#define COMPLEX_CUR_COPY_L_1_HE(tmp_a, bb) \
    do {                                   \
        *((bb)[0]) = (tmp_a)[0][0];        \
        *((bb)[0] + 1) = -(tmp_a)[1][0];   \
        *((bb)[1]) = (tmp_a)[0][1];        \
        *((bb)[1] + 1) = -(tmp_a)[1][1];   \
    } while (0)

#define COMPLEX_SYM_COPY_L_1_EQ(tmp_a, cc) \
    do {                                   \
        *((cc)[0]) = (tmp_a)[0][0];        \
        *((cc)[0] + 1) = (tmp_a)[1][0];    \
        *((cc)[0] + 2) = (tmp_a)[0][1];    \
        *((cc)[0] + 3) = (tmp_a)[1][1];    \
    } while (0)

#define COMPLEX_SYM_COPY_L_1_HE(tmp_a, cc) \
    do {                                   \
        *((cc)[0]) = (tmp_a)[0][0];        \
        *((cc)[0] + 1) = -(tmp_a)[1][0];   \
        *((cc)[0] + 2) = (tmp_a)[0][1];    \
        *((cc)[0] + 3) = -(tmp_a)[1][1];   \
    } while (0)

#define COMPLEX_SYMV_COPY_U_1(FLAG1, FLAG2, m, tmp_a, aa, bb, cc) \
    do {                                                          \
        (tmp_a)[0][0] = *((aa)[0]);                               \
        (tmp_a)[1][0] = *((aa)[0] + 1);                           \
        (tmp_a)[2][0] = *((aa)[0] + 2);                           \
        (tmp_a)[3][0] = *((aa)[0] + 3);                           \
        (aa)[0] += SYMV_BLOCK_2x2_COMPSIZE;                       \
        COMPLEX_CUR_COPY_U_1_##FLAG1(tmp_a, bb);                  \
        COMPLEX_SYM_COPY_U_1_##FLAG2(m, tmp_a, cc);               \
    } while (0)

#define COMPLEX_CUR_COPY_U_1_EQ(tmp_a, bb)  \
    do {                                    \
        *((bb)[0]) = (tmp_a)[0][0];         \
        *((bb)[0] + 1) = (tmp_a)[1][0];     \
        *((bb)[0] + 2) = (tmp_a)[2][0];     \
        *((bb)[0] + 3) = (tmp_a)[3][0];     \
        (bb)[0] += SYMV_BLOCK_2x2_COMPSIZE; \
    } while (0)

#define COMPLEX_CUR_COPY_U_1_HE(tmp_a, bb)  \
    do {                                    \
        *((bb)[0]) = (tmp_a)[0][0];         \
        *((bb)[0] + 1) = -(tmp_a)[1][0];    \
        *((bb)[0] + 2) = (tmp_a)[2][0];     \
        *((bb)[0] + 3) = -(tmp_a)[3][0];    \
        (bb)[0] += SYMV_BLOCK_2x2_COMPSIZE; \
    } while (0)

#define COMPLEX_SYM_COPY_U_1_EQ(m, tmp_a, cc)     \
    do {                                          \
        *((cc)[0]) = (tmp_a)[0][0];               \
        *((cc)[0] + 1) = (tmp_a)[1][0];           \
        *((cc)[1]) = (tmp_a)[2][0];               \
        *((cc)[1] + 1) = (tmp_a)[3][0];           \
        (cc)[0] += SYMV_BLOCK_2x2_COMPSIZE * (m); \
        (cc)[1] += SYMV_BLOCK_2x2_COMPSIZE * (m); \
    } while (0)

#define COMPLEX_SYM_COPY_U_1_HE(m, tmp_a, cc)     \
    do {                                          \
        *((cc)[0]) = (tmp_a)[0][0];               \
        *((cc)[0] + 1) = -(tmp_a)[1][0];          \
        *((cc)[1]) = (tmp_a)[2][0];               \
        *((cc)[1] + 1) = -(tmp_a)[3][0];          \
        (cc)[0] += SYMV_BLOCK_2x2_COMPSIZE * (m); \
        (cc)[1] += SYMV_BLOCK_2x2_COMPSIZE * (m); \
    } while (0)

#define CUR_OFFSET(aa)                      \
    do {                                    \
        (aa)[0] += SYMV_BLOCK_2x2_COMPSIZE; \
        (aa)[1] += SYMV_BLOCK_2x2_COMPSIZE; \
    } while (0)

#define SYM_OFFSET(m, cc)                         \
    do {                                          \
        (cc)[0] += SYMV_BLOCK_2x2_COMPSIZE * (m); \
        (cc)[1] += SYMV_BLOCK_2x2_COMPSIZE * (m); \
    } while (0)

#define ADDR_OFFSET_L(m, a, lda, tmpB)                                        \
    do {                                                                      \
        a += SYMV_SMALLEST_BLOCK_SIZE * (lda) + SYMV_BLOCK_2x2_COMPSIZE;      \
        (tmpB)[0] += SYMV_BLOCK_2x2_COMPSIZE * (m) + SYMV_BLOCK_2x2_COMPSIZE; \
        (tmpB)[1] += SYMV_BLOCK_2x2_COMPSIZE * (m) + SYMV_BLOCK_2x2_COMPSIZE; \
    } while (0)

#define ADDR_OFFSET_U(m, a, lda, tmpB)              \
    do {                                            \
        a += SYMV_SMALLEST_BLOCK_SIZE * (lda);      \
        (tmpB)[0] += SYMV_BLOCK_2x2_COMPSIZE * (m); \
        (tmpB)[1] += SYMV_BLOCK_2x2_COMPSIZE;       \
    } while (0)

#define ADDR_INIT(X, m, a, lda, aa, bb, cc, tmpB) \
    do {                                          \
        (aa)[0] = a;                              \
        (aa)[1] = (a) + 1 * (lda);                \
        (bb)[0] = (tmpB)[0];                      \
        (bb)[1] = (tmpB)[0] + COMPSIZE * (m);     \
        (cc)[0] = (tmpB)[1];                      \
        (cc)[1] = (tmpB)[1] + COMPSIZE * (m);     \
        ADDR_OFFSET_##X(m, a, lda, tmpB);         \
    } while (0)

#ifndef HEMVREV
#define ARRAY_LEN 2
#define TMPA_ARRAY_LEN 4

#ifndef SIMPLE_MATRIX_CALCULATION
static void ZHEMCOPY_L(int32_t m, float *a_, int32_t lda_, float *b)
{
    int32_t is, js;
    int32_t lda = lda_;

    float *a = a_;
    float *aa[ARRAY_LEN] = { NULL, NULL };
    float *tmpB[ARRAY_LEN] = { b, b };
    float *bb[ARRAY_LEN] = { NULL, NULL };
    float *cc[ARRAY_LEN] = { NULL, NULL };
    float tmp_a[TMPA_ARRAY_LEN][ARRAY_LEN];

    lda *= COMPSIZE;

    for (js = 0; js < m; js += SYMV_SMALLEST_BLOCK_SIZE) {
        ADDR_INIT(L, m, a, lda, aa, bb, cc, tmpB);

        if (m - js >= SYMV_SMALLEST_BLOCK_SIZE) {
            tmp_a[0][0] = *(aa[0]);
            tmp_a[2][0] = *(aa[0] + 2);  // 2 is the index
            tmp_a[3][0] = *(aa[0] + 3);  // 3 is the index

            tmp_a[0][1] = *(aa[1] + 2);  // 2 is the index

            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;           // 0. is float zero
            *(bb[0] + 2) = tmp_a[2][0];  // 2 is the index
            *(bb[0] + 3) = tmp_a[3][0];  // 3 is the index

            *(bb[1]) = tmp_a[2][0];       // 2 is the index
            *(bb[1] + 1) = -tmp_a[3][0];  // 3 is the index
            *(bb[1] + 2) = tmp_a[0][1];   // 2 is the index
            *(bb[1] + 3) = 0.;            // 0. is float zero, 3 is the index

            aa[0] += SYMV_BLOCK_2x2_COMPSIZE;
            aa[1] += SYMV_BLOCK_2x2_COMPSIZE;
            bb[0] += SYMV_BLOCK_2x2_COMPSIZE;
            bb[1] += SYMV_BLOCK_2x2_COMPSIZE;

            cc[0] += SYMV_BLOCK_2x2_COMPSIZE * m;
            cc[1] += SYMV_BLOCK_2x2_COMPSIZE * m;

            is = ((uint32_t)(m - js - SYMV_SMALLEST_BLOCK_SIZE) >> 1);

            while (is > 0) {
                COMPLEX_BLOCK_2x2_ALL_COPY(EQ, HE, m, tmp_a, aa, bb, cc);
                is--;
            }
            COMPLEX_SYMV_COPY_L_1(EQ, HE, m, tmp_a, aa, bb, cc);
        }

        if (m - js == 1) {
            tmp_a[0][0] = *(aa[0]);
            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;  // 0. is float zero
        }
    }
}

static void ZHEMCOPY_U(int32_t m, float *a_, int32_t lda_, float *b)
{
    int32_t is, js;
    int32_t lda = lda_;

    float *a = a_;

    float *aa[ARRAY_LEN] = { NULL, NULL };
    float *tmpB[ARRAY_LEN] = { b, b };
    float *bb[ARRAY_LEN] = { NULL, NULL };
    float *cc[ARRAY_LEN] = { NULL, NULL };
    float tmp_a[TMPA_ARRAY_LEN][ARRAY_LEN];

    lda *= COMPSIZE;

    for (js = 0; js < m; js += SYMV_SMALLEST_BLOCK_SIZE) {
        ADDR_INIT(U, m, a, lda, aa, bb, cc, tmpB);

        if (m - js >= SYMV_SMALLEST_BLOCK_SIZE) {
            for (is = 0; is < js; is += SYMV_SMALLEST_BLOCK_SIZE) {
                COMPLEX_BLOCK_2x2_ALL_COPY(EQ, HE, m, tmp_a, aa, bb, cc);
            }

            tmp_a[0][0] = *(aa[0]);

            tmp_a[0][1] = *(aa[1]);
            tmp_a[1][1] = *(aa[1] + 1);
            tmp_a[2][1] = *(aa[1] + 2);  // 2 is the index

            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;            // 0. is float zero
            *(bb[0] + 2) = tmp_a[0][1];   // 2 is the index
            *(bb[0] + 3) = -tmp_a[1][1];  // 3 is the index

            *(bb[1]) = tmp_a[0][1];
            *(bb[1] + 1) = tmp_a[1][1];
            *(bb[1] + 2) = tmp_a[2][1];  // 2 is the index
            *(bb[1] + 3) = 0.;           // 3 is the index, 0. is float zero
        }

        if (m - js == 1) {
            for (is = 0; is < js; is += SYMV_SMALLEST_BLOCK_SIZE) {
                COMPLEX_SYMV_COPY_U_1(EQ, HE, m, tmp_a, aa, bb, cc);
            }
            tmp_a[0][0] = *(aa[0]);
            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;  // 0. is float zero
        }
    }
}
#endif /* SIMPLE_MATRIX_CALCULATION */

#else /* HEMVREV */

static void ZHEMCOPY_M(int32_t m, float *a_, int32_t lda_, float *b)
{
    int32_t is, js;
    int32_t lda = lda_;

    float *a = a_;
    float *aa[ARRAY_LEN] = { NULL, NULL };
    float *tmpB[ARRAY_LEN] = { b, b };
    float *bb[ARRAY_LEN] = { NULL, NULL };
    float *cc[ARRAY_LEN] = { NULL, NULL };
    float tmp_a[TMPA_ARRAY_LEN][ARRAY_LEN];

    lda *= COMPSIZE;

    for (js = 0; js < m; js += SYMV_SMALLEST_BLOCK_SIZE) {
        ADDR_INIT(L, m, a, lda, aa, bb, cc, tmpB);

        if (m - js >= SYMV_SMALLEST_BLOCK_SIZE) {
            tmp_a[0][0] = *(aa[0]);
            tmp_a[2][0] = *(aa[0] + 2);  // 2 is the index
            tmp_a[3][0] = *(aa[0] + 3);  // 3 is the index

            tmp_a[0][1] = *(aa[1] + 2);  // 2 is the index

            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;            // 0. is float zero
            *(bb[0] + 2) = tmp_a[2][0];   // 2 is the index
            *(bb[0] + 3) = -tmp_a[3][0];  // 3 is the index

            *(bb[1]) = tmp_a[2][0];      // 2 is the index
            *(bb[1] + 1) = tmp_a[3][0];  // 3 is the index
            *(bb[1] + 2) = tmp_a[0][1];  // 2 is the index
            *(bb[1] + 3) = 0.;           // 3 is the index, 0.is float zero

            aa[0] += SYMV_BLOCK_2x2_COMPSIZE;
            aa[1] += SYMV_BLOCK_2x2_COMPSIZE;
            bb[0] += SYMV_BLOCK_2x2_COMPSIZE;
            bb[1] += SYMV_BLOCK_2x2_COMPSIZE;

            cc[0] += SYMV_BLOCK_2x2_COMPSIZE * m;
            cc[1] += SYMV_BLOCK_2x2_COMPSIZE * m;

            is = ((uint32_t)(m - js - SYMV_SMALLEST_BLOCK_SIZE) >> 1);

            while (is > 0) {
                COMPLEX_BLOCK_2x2_ALL_COPY(HE, EQ, m, tmp_a, aa, bb, cc);
                is--;
            }
            COMPLEX_SYMV_COPY_L_1(HE, EQ, m, tmp_a, aa, bb, cc);
        }

        if (m - js == 1) {
            tmp_a[0][0] = *(aa[0]);
            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;  // 0.is float zero
        }
    }
}

static void ZHEMCOPY_V(int32_t m, float *a_, int32_t lda_, float *b)
{
    int32_t is, js;
    int32_t lda = lda_;

    float *a = a_;
    float *aa[ARRAY_LEN] = { NULL, NULL };
    float *tmpB[ARRAY_LEN] = { b, b };
    float *bb[ARRAY_LEN] = { NULL, NULL };
    float *cc[ARRAY_LEN] = { NULL, NULL };
    float tmp_a[TMPA_ARRAY_LEN][ARRAY_LEN];

    lda *= COMPSIZE;

    for (js = 0; js < m; js += SYMV_SMALLEST_BLOCK_SIZE) {
        ADDR_INIT(U, m, a, lda, aa, bb, cc, tmpB);

        if (m - js >= SYMV_SMALLEST_BLOCK_SIZE) {
            for (is = 0; is < js; is += SYMV_SMALLEST_BLOCK_SIZE) {
                COMPLEX_BLOCK_2x2_ALL_COPY(HE, EQ, m, tmp_a, aa, bb, cc);
            }

            tmp_a[0][0] = *(aa[0]);

            tmp_a[0][1] = *(aa[1]);
            tmp_a[1][1] = *(aa[1] + 1);
            tmp_a[2][1] = *(aa[1] + 2);  // 2 is the index

            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;           // 0. is float zero
            *(bb[0] + 2) = tmp_a[0][1];  // 2 is the index
            *(bb[0] + 3) = tmp_a[1][1];  // 3 is the index

            *(bb[1]) = tmp_a[0][1];
            *(bb[1] + 1) = -tmp_a[1][1];
            *(bb[1] + 2) = tmp_a[2][1];  // 2 is the index
            *(bb[1] + 3) = 0.;           // 3 is the index, 0. is float zero
        }

        if (m - js == 1) {
            for (is = 0; is < js; is += SYMV_SMALLEST_BLOCK_SIZE) {
                COMPLEX_SYMV_COPY_U_1(HE, EQ, m, tmp_a, aa, bb, cc);
            }
            tmp_a[0][0] = *(aa[0]);
            *(bb[0]) = tmp_a[0][0];
            *(bb[0] + 1) = 0.;  // 0. is float zero
        }
    }
}

#endif /* HEMVREV */

#endif