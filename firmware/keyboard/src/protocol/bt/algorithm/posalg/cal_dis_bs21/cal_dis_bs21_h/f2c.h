/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Provides v151_100 spi HAL spi, qspi_v151_100 origin from spi_v100, spi_v151_100 origin from spi_v151
 * Author: HiSilicon (Shanghai) Technologies Co., Ltd
 * Create: 2024
 */
#ifndef F2C_INCLUDE
#define F2C_INCLUDE

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "securec.h"

#include "alg_dts_log.h"

#ifndef SIMPLE_MATRIX_CALCULATION
#define SIMPLE_MATRIX_CALCULATION
#endif
#ifndef GLE_CS_MODE3_SUPPORT
#define GLE_CS_MODE3_SUPPORT
#endif

typedef int integer;
typedef unsigned long int uinteger;
typedef char *address;
typedef short int shortint;
typedef float real;
typedef float doublereal;
typedef struct {
    real r, i;
} complex;
typedef struct {
    real r, i;
} doublecomplex;
typedef int logical;
typedef short int shortlogical;
typedef char logical1;
typedef char integer1;
#ifdef INTEGER_STAR_8                /* Adjust for integer*8. */
typedef long long longint;           /* system-dependent */
typedef unsigned long long ulongint; /* system-dependent */
#define qbit_clear(a, b) ((a) & ~((uinteger)1 << (b)))
#define qbit_set(a, b) ((a) | ((uinteger)1 << (b)))
#endif

#define TRUE_ (1)
#define FALSE_ (0)

/* Extern is for use with -E */
#ifndef Extern
#define Extern extern
#endif

/* external read, write */
typedef struct {
    int16_t cierr;
    int16_t ciunit;
    int16_t ciend;
    char *cifmt;
    int16_t cirec;
} cilist;

/* internal read, write */
typedef struct {
    int16_t icierr;
    char *iciunit;
    int16_t iciend;
    char *icifmt;
    int16_t icirlen;
    int16_t icirnum;
} icilist;

/* open */
typedef struct {
    int16_t oerr;
    int16_t ounit;
    char *ofnm;
    int16_t ofnmlen;
    char *osta;
    char *oacc;
    char *ofm;
    int16_t orl;
    char *oblnk;
} olist;

/* close */
typedef struct {
    int16_t cerr;
    int16_t cunit;
    char *csta;
} cllist;

/* rewind, backspace, endfile */
typedef struct {
    int16_t aerr;
    int16_t aunit;
} alist;

/* inquire */
typedef struct {
    int16_t inerr;
    int16_t inunit;
    char *infile;
    int16_t infilen;
    int16_t *inex; /* parameters in standard's order */
    int16_t *inopen;
    int16_t *innum;
    int16_t *innamed;
    char *inname;
    int16_t innamlen;
    char *inacc;
    int16_t inacclen;
    char *inseq;
    int16_t inseqlen;
    char *indir;
    int16_t indirlen;
    char *infmt;
    int16_t infmtlen;
    char *inform;
    int16_t informlen;
    char *inunf;
    int16_t inunflen;
    int16_t *inrecl;
    int16_t *innrec;
    char *inblank;
    int16_t inblanklen;
} inlist;

#define VOID void

union Multitype { /* for multiple entry points */
    integer1 g;
    shortint h;
    integer i;
#ifndef SIMPLE_MATRIX_CALCULATION
    long j;
#endif
    real r;
    real d;
    complex c;
    doublecomplex z;
};

typedef union Multitype Multitype;

/* typedef int Long; No longer used; formerly in Namelist */

struct Vardesc { /* for Namelist */
    char *name;
    char *addr;
    int16_t *dims;
    int type;
};
typedef struct Vardesc Vardesc;

struct Namelist {
    char *name;
    Vardesc **vars;
    int nvars;
};
typedef struct Namelist Namelist;

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define dabs(x) (real) ABS(x)

#ifndef min
#define min(a, b) ((a) <= (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) >= (b) ? (a) : (b))
#endif

static inline int32_t matrix_max(int32_t a, int32_t b)
{
    return (a) >= (b) ? (a) : (b);
}

static inline float matrix_max_f(float a, float b)
{
    return (a) >= (b) ? (a) : (b);
}

static inline float matrix_min_f(float a, float b)
{
    return (a) <= (b) ? (a) : (b);
}

#define dmin(a, b) (real) min(a, b)
#define dmax(a, b) (real) max(a, b)
#define bit_test(a, b) ((a) >> (b)&1)
#define bit_clear(a, b) ((a) & ~((uinteger)1 << (b)))
#define bit_set(a, b) ((a) | ((uinteger)1 << (b)))

/* procedure parameter types for -A and -C++ */

#define F2C_proc_par_types 1

typedef int (*U_fp)(void);
typedef shortint (*J_fp)(void);
typedef integer (*I_fp)(void);
typedef real (*R_fp)(void);
typedef real (*D_fp)(void), (*E_fp)(void);
typedef VOID (*C_fp)(void);
typedef VOID (*Z_fp)(void);
typedef shortlogical (*L_fp)(void);
typedef shortlogical (*K_fp)(void);
typedef VOID (*H_fp)(void);
typedef int (*S_fp)(void);

/* E_fp is for real functions when -R is not specified */
typedef VOID C_f; /* complex function */
typedef VOID H_f; /* character function */
typedef VOID Z_f; /* double complex function */
typedef real E_f; /* real function with -R not specified */

/* undef any lower-case symbols that your C compiler predefines, e.g.: */

#ifndef Skip_f2c_Undefs
#undef cray
#undef gcos
#undef mc68010
#undef mc68020
#undef mips
#undef pdp11
#undef sgi
#undef sparc
#undef sun
#undef sun2
#undef sun3
#undef sun4
#undef u370
#undef u3b
#undef u3b2
#undef u3b5
#undef unix
#undef vax
#endif

#define TWO 2
#define THREE 3
#define FOUR 4
#define FIVE 5
#define SIX 6
#define SEVEN 7
#define EIGHT 8
#define NINE 9
#define TEN 10

#define HALF_FLOAT 0.5f
#define ZERO_FLOAT 0.f
#define ONE_FLOAT 1.f
#define TWO_FLOAT 2.f
#define THREE_FLOAT 3.f
#define FOUR_FLOAT 4.f
#define FIVE_FLOAT 5.f
#define SIX_FLOAT 6.f
#define SEVEN_FLOAT 7.f
#define EIGHT_FLOAT 8.f
#define NINE_FLOAT 9.f
#define MATRIX_EPSILON (0.0001)

static inline bool matrix_equal_float(float a, float b)
{
    float diff = a - b;
    return ((diff > -MATRIX_EPSILON) && (diff < MATRIX_EPSILON));
}

#define A_TAG "A"
#define B_TAG "B"
#define C_TAG "C"
#define D_TAG "D"
#define E_TAG "E"
#define F_TAG "F"
#define G_TAG "G"
#define H_TAG "H"
#define I_TAG "I"
#define J_TAG "J"
#define K_TAG "K"
#define L_TAG "L"
#define M_TAG "M"
#define N_TAG "N"
#define O_TAG "O"
#define P_TAG "P"
#define Q_TAG "Q"
#define R_TAG "R"
#define S_TAG "S"
#define T_TAG "T"
#define U_TAG "U"
#define V_TAG "V"
#define W_TAG "W"
#define X_TAG "X"
#define Y_TAG "Y"
#define Z_TAG "Z"

float sqrtf(float);
float powf(float, float);
float expf(float);
float logf(float);
// f2c.h声明
void c_div(complex *, complex *, complex *);
float f__cabs(float real_num, float imag);
float c_abs(complex *z);
void c_sqrt(complex *r, complex *z);
float copysign_ri(float r, float z);
int32_t i_len(char *s, int16_t n);
int32_t i_nint(float *x);
void pow_ci(complex *p, complex *a, int32_t *b);
float pow_ri(float *ap, int32_t *bp);
void r_cnjg(complex *r, complex *z);
float r_imag(complex *z);
float r_lg10(float *x);
float r_max(float a, float b, float c);
float r_sign(float *a, float *b);
void s_cat(char *lp, char *rpp[], int32_t rnp[], int32_t *np, int16_t ll);
int32_t s_cmp(char *a0, char *b0, int16_t la, int16_t lb);
void s_copy(register char *a, register char *b, int16_t la, int16_t lb);

// clapack.h声明
int32_t cgebak_(char *job, char *side, int32_t *n, int32_t *ilo, int32_t *ihi, float *scale, int32_t *m, complex *v,
                int32_t *ldv, int32_t *info, int16_t, int16_t);
int32_t cgebal_(char *job, int32_t *n, complex *a, int32_t *lda, int32_t *ilo, int32_t *ihi, float *scale,
                int32_t *info, int16_t);
int32_t cgeev_(char *jobvl, char *jobvr, int32_t *n, complex *a, int32_t *lda, complex *w, complex *vl, int32_t *ldvl,
               complex *vr, int32_t *ldvr, complex *work, int32_t *lwork, float *rwork, int32_t *info);
int32_t cgehd2_(int32_t *n, int32_t *ilo, int32_t *ihi, complex *a, int32_t *lda, complex *tau, complex *work,
                int32_t *info);
int32_t cgehrd_(int32_t *n, int32_t *ilo, int32_t *ihi, complex *a, int32_t *lda, complex *tau, complex *work,
                int32_t *lwork, int32_t *info);
int32_t cheev_(char *jobz, char *uplo, int32_t *n, complex *a, int32_t *lda, float *w, complex *work, int32_t *lwork,
               float *rwork, int32_t *info);
int32_t chetd2_(char *uplo, int32_t *n, complex *a, int32_t *lda, float *d__, float *e, complex *tau, int32_t *info,
                int16_t);
int32_t chetrd_(char *uplo, int32_t *n, complex *a, int32_t *lda, float *d__, float *e, complex *tau, complex *work,
                int32_t *lwork, int32_t *info, int16_t);
int32_t chseqr_(char *job, char *compz, int32_t *n, int32_t *ilo, int32_t *ihi, complex *h__, int32_t *ldh, complex *w,
                complex *z__, int32_t *ldz, complex *work, int32_t *lwork, int32_t *info, int16_t job_len,
                int16_t compz_len);
int32_t clacgv_(int32_t *n, complex *x, int32_t *incx);
int32_t clacpy_(char *uplo, int32_t *m, int32_t *n, complex *a, int32_t *lda, complex *b, int32_t *ldb,
                int16_t uplo_len);
VOID cladiv_(complex *ret_val, complex *x, complex *y);
int32_t clahqr_(int16_t *wantt, int16_t *wantz, int32_t *n, int32_t *ilo, int32_t *ihi, complex *h__, int32_t *ldh,
                complex *w, int32_t *iloz, int32_t *ihiz, complex *z__, int32_t *ldz, int32_t *info);
int32_t clahr2_(int32_t *n, int32_t *k, int32_t *nb, complex *a, int32_t *lda, complex *tau, complex *t, int32_t *ldt,
                complex *y, int32_t *ldy);
float clange_(char *norm, int32_t *m, int32_t *n, complex *a, int32_t *lda, float *work, int16_t);
float clanhe_(char *norm, char *uplo, int32_t *n, complex *a, int32_t *lda, float *work, int16_t, int16_t);
int32_t claqr0_(int16_t *wantt, int16_t *wantz, int32_t *n, int32_t *ilo, int32_t *ihi, complex *h__, int32_t *ldh,
                complex *w, int32_t *iloz, int32_t *ihiz, complex *z__, int32_t *ldz, complex *work, int32_t *lwork,
                int32_t *info);
int32_t claqr1_(int32_t *n, complex *h__, int32_t *ldh, complex *s1, complex *s2, complex *v);
int32_t claqr2_(int16_t *wantt, int16_t *wantz, int32_t *n, int32_t *ktop, int32_t *kbot, int32_t *nw, complex *h__,
                int32_t *ldh, int32_t *iloz, int32_t *ihiz, complex *z__, int32_t *ldz, int32_t *ns, int32_t *nd,
                complex *sh, complex *v, int32_t *ldv, int32_t *nh, complex *t, int32_t *ldt, int32_t *nv, complex *wv,
                int32_t *ldwv, complex *work, int32_t *lwork);

int32_t claqr3_(int16_t *wantt, int16_t *wantz, int32_t *n, int32_t *ktop, int32_t *kbot, int32_t *nw, complex *h__,
                int32_t *ldh, int32_t *iloz, int32_t *ihiz, complex *z__, int32_t *ldz, int32_t *ns, int32_t *nd,
                complex *sh, complex *v, int32_t *ldv, int32_t *nh, complex *t, int32_t *ldt, int32_t *nv, complex *wv,
                int32_t *ldwv, complex *work, int32_t *lwork);

int32_t claqr4_(int16_t *wantt, int16_t *wantz, int32_t *n, int32_t *ilo, int32_t *ihi, complex *h__, int32_t *ldh,
                complex *w, int32_t *iloz, int32_t *ihiz, complex *z__, int32_t *ldz, complex *work, int32_t *lwork,
                int32_t *info);

int32_t claqr5_(int16_t *wantt, int16_t *wantz, int32_t *kacc22, int32_t *n, int32_t *ktop, int32_t *kbot,
                int32_t *nshfts, complex *s, complex *h__, int32_t *ldh, int32_t *iloz, int32_t *ihiz, complex *z__,
                int32_t *ldz, complex *v, int32_t *ldv, complex *u, int32_t *ldu, int32_t *nv, complex *wv,
                int32_t *ldwv, int32_t *nh, complex *wh, int32_t *ldwh);
int32_t clarf_(char *side, int32_t *m, int32_t *n, complex *v, int32_t *incv, complex *tau, complex *c__, int32_t *ldc,
               complex *work, int16_t);
int32_t clarfb_(char *side, char *trans, char *direct, char *storev, int32_t *m, int32_t *n, int32_t *k, complex *v,
                int32_t *ldv, complex *t, int32_t *ldt, complex *c__, int32_t *ldc, complex *work, int32_t *ldwork,
                int16_t, int16_t, int16_t, int16_t);
int32_t clarfg_(int32_t *n, complex *alpha, complex *x, int32_t *incx, complex *tau);
int32_t clarft_(char *direct, char *storev, int32_t *n, int32_t *k, complex *v, int32_t *ldv, complex *tau, complex *t,
                int32_t *ldt, int16_t, int16_t);
int32_t clartg_(complex *f, complex *g, float *cs, complex *sn, complex *r__);
int32_t clascl_(char *type__, int32_t *kl, int32_t *ku, float *cfrom, float *cto, int32_t *m, int32_t *n, complex *a,
                int32_t *lda, int32_t *info, int16_t ftn_len);
int32_t claset_(char *uplo, int32_t *m, int32_t *n, complex *alpha, complex *beta, complex *a, int32_t *lda, int16_t);
int32_t clasr_(char *side, char *pivot, char *direct, int32_t *m, int32_t *n, float *c__, float *s, complex *a,
               int32_t *lda, int16_t, int16_t, int16_t);
int32_t classq_(int32_t *n, complex *x, int32_t *incx, float *scale, float *sumsq);
int32_t clatrd_(char *uplo, int32_t *n, int32_t *nb, complex *a, int32_t *lda, float *e, complex *tau, complex *w,
                int32_t *ldw, int16_t);
int32_t clatrs_(char *uplo, char *trans, char *diag, char *normin, int32_t *n, complex *a, int32_t *lda, complex *x,
                float *scale, float *cnorm, int32_t *info, int16_t, int16_t, int16_t, int16_t);
int32_t crot_(int32_t *n, complex *cx, int32_t *incx, complex *cy, int32_t *incy, float *c__, complex *s);
int32_t csteqr_(char *compz, int32_t *n, float *d__, float *e, complex *z__, int32_t *ldz, float *work, int32_t *info,
                int16_t ftn_len);
int32_t ctrevc_(char *side, char *howmny, int16_t *select, int32_t *n, complex *t, int32_t *ldt, complex *vl,
                int32_t *ldvl, complex *vr, int32_t *ldvr, int32_t *mm, int32_t *m, complex *work, float *rwork,
                int32_t *info, int16_t, int16_t);
int32_t ctrexc_(char *compq, int32_t *n, complex *t, int32_t *ldt, complex *q, int32_t *ldq, int32_t *ifst,
                int32_t *ilst, int32_t *info, int16_t);
int32_t cung2l_(int32_t *m, int32_t *n, int32_t *k, complex *a, int32_t *lda, complex *tau, complex *work,
                int32_t *info);
int32_t cung2r_(int32_t *m, int32_t *n, int32_t *k, complex *a, int32_t *lda, complex *tau, complex *work,
                int32_t *info);
int32_t cunghr_(int32_t *n, int32_t *ilo, int32_t *ihi, complex *a, int32_t *lda, complex *tau, complex *work,
                int32_t *lwork, int32_t *info);
int32_t cunmqr_(char *side, char *trans, int32_t *m, int32_t *n, int32_t *k, complex *a, int32_t *lda, complex *tau,
    complex *c__, int32_t *ldc, complex *work, int32_t *lwork, int32_t *info, int16_t, int16_t);
int32_t ctrevc3_(char *side, char *howmny, int16_t *select, int32_t *n, complex *t, int32_t *ldt, complex *vl,
                 int32_t *ldvl, complex *vr, int32_t *ldvr, int32_t *mm, int32_t *m, complex *work, int32_t *lwork,
                 float *rwork, int32_t *lrwork, int32_t *info, int16_t side_len, int16_t howmny_len);
int32_t ieeeck_(int32_t *ispec, float *zero, float *one);
int32_t ilaclc_(int32_t *m, int32_t *n, complex *a, int32_t *lda);
int32_t ilaclr_(int32_t *m, int32_t *n, complex *a, int32_t *lda);
int32_t ilaenv_(int32_t *ispec, char *name__, char *opts, int32_t *n1, int32_t *n2, int32_t *n3, int32_t *n4, int16_t,
                int16_t);
int32_t iparmq_(int32_t *ispec, char *name__, char *opts, int32_t *n, int32_t *ilo, int32_t *ihi, int32_t *lwork,
                int16_t, int16_t);
int16_t sisnan_(float *sin__);
int32_t slabad_(float *small, float *large);
int32_t sladiv_(float *a, float *b, float *c__, float *d__, float *p, float *q);
int32_t slae2_(float *a, float *b, float *c__, float *rt1, float *rt2);
int32_t slaev2_(float *a, float *b, float *c__, float *rt1, float *rt2, float *cs1, float *sn1);
int16_t slaisnan_(float *sin1, float *sin2);
float slamch_(char *, int16_t);
float slanst_(char *norm, int32_t *n, float *d__, float *e, int16_t);
float slapy2_(float *x, float *y);
float slapy3_(float *x, float *y, float *z__);
int32_t slartg_(float *f, float *g, float *cs, float *sn, float *r__);
int32_t slascl_(char *type__, int32_t *kl, int32_t *ku, float *cfrom, float *cto, int32_t *m, int32_t *n, float *a,
                int32_t *lda, int32_t *info, int16_t);
int32_t slasrt_(char *id, int32_t *n, float *d__, int32_t *info, int16_t);
int32_t slassq_(int32_t *n, float *x, int32_t *incx, float *scale, float *sumsq);
int32_t ssterf_(int32_t *n, float *d__, float *e, int32_t *info);
int32_t xerbla_(char *srname, int32_t *info, int16_t len);

// blaswrap.h
int32_t cungtr_(char *uplo, int32_t *n, complex *a, int32_t *lda, complex *tau, complex *work, int32_t *lwork,
                int32_t *info, int16_t uplo_len);
int32_t cungqr_(int32_t *m, int32_t *n, int32_t *k, complex *a, int32_t *lda, complex *tau, complex *work,
                int32_t *lwork, int32_t *info);
int32_t cungql_(int32_t *m, int32_t *n, int32_t *k, complex *a, int32_t *lda, complex *tau, complex *work,
                int32_t *lwork, int32_t *info);
int32_t cunmhr_(char *side, char *trans, int32_t *m, int32_t *n, int32_t *ilo, int32_t *ihi, complex *a, int32_t *lda,
                complex *tau, complex *c__, int32_t *ldc, complex *work, int32_t *lwork, int32_t *info,
                int16_t side_len, int16_t trans_len);
int32_t cunm2r_(char *side, char *trans, int32_t *m, int32_t *n, int32_t *k, complex *a, int32_t *lda, complex *tau,
                complex *c__, int32_t *ldc, complex *work, int32_t *info, int16_t side_len, int16_t trans_len);
float sroundup_lwork__(int32_t *lwork);
int32_t caxpy_(int32_t *n, complex *valpha, complex *vx, int32_t *incx, complex *vy, int32_t *incy);
int32_t ccopy_(int32_t *n, complex *cx, int32_t *incx, complex *cy, int32_t *incy);
void cdotc_(complex *ret_val, int32_t *n, complex *cx, int32_t *incx, complex *cy, int32_t *incy);
void cdotu_(complex *ret_val, int32_t *n, complex *cx, int32_t *incx, complex *cy, int32_t *incy);
int32_t cgemm_(char *transa, char *transb, int32_t *m, int32_t *n, int32_t *k, complex *alpha, complex *a, int32_t *lda,
               complex *b, int32_t *ldb, complex *beta, complex *c__, int32_t *ldc, int16_t transa_len,
               int16_t transb_len);
int32_t cgemv_(char *trans, int32_t *m, int32_t *n, complex *alpha, complex *a, int32_t *lda, complex *x, int32_t *incx,
               complex *beta, complex *y, int32_t *incy, int16_t trans_len);
int32_t cgerc_(int32_t *m, int32_t *n, complex *alpha, complex *x, int32_t *incx, complex *y, int32_t *incy, complex *a,
               int32_t *lda);
int32_t chemv_(char *uplo, int32_t *n, complex *alpha, complex *a, int32_t *lda, complex *x, int32_t *incx,
               complex *beta, complex *y, int32_t *incy, int16_t uplo_len);
int32_t cher2_(char *uplo, int32_t *n, complex *alpha, complex *x, int32_t *incx, complex *y, int32_t *incy, complex *a,
               int32_t *lda, int16_t uplo_len);
int32_t cher2k_(char *uplo, char *trans, int32_t *n, int32_t *k, complex *alpha, complex *a, int32_t *lda, complex *b,
                int32_t *ldb, float *beta, complex *c__, int32_t *ldc, int16_t uplo_len, int16_t trans_len);
int32_t cscal_(int32_t *n, complex *ca, complex *cx, int32_t *incx);
int32_t csscal_(int32_t *n, float *sa, complex *cx, int32_t *incx);
int32_t cswap_(int32_t *n, complex *cx, int32_t *incx, complex *cy, int32_t *incy);
int32_t ctrmm_(char *side, char *uplo, char *transa, char *diag, int32_t *m, int32_t *n, complex *alpha, complex *a,
               int32_t *lda, complex *b, int32_t *ldb, int16_t side_len, int16_t uplo_len, int16_t transa_len,
               int16_t diag_len);
int32_t ctrmv_(char *uplo, char *trans, char *diag, int32_t *n, complex *a, int32_t *lda, complex *x, int32_t *incx,
               int16_t uplo_len, int16_t trans_len, int16_t diag_len);
int32_t ctrsv_(char *uplo, char *trans, char *diag, int32_t *n, complex *a, int32_t *lda, complex *x, int32_t *incx,
               int16_t uplo_len, int16_t trans_len, int16_t diag_len);
int32_t digits_(float);
float epsilon_(float);
float huge_(float);
int32_t icamax_(int32_t *n, complex *cx, int32_t *incx);
int32_t isamax_(int32_t *n, float *sx, int32_t *incx);
int16_t lsame_(char *ca, char *cb, int16_t ca_len, int16_t cb_len);
float maxexponent_(float);
float minexponent_(float);
float scabs1_(complex *);
float scasum_(int32_t *n, complex *cx, int32_t *incx);
float scnrm2_(int32_t *n, complex *x, int32_t *incx);
int32_t sscal_(int32_t *n, float *sa, float *sx, int32_t *incx);
float tiny_(float);
float radix_(float z);
complex cdotu_k(int32_t n, float *x, int32_t inc_x, float *y, int32_t inc_y);
complex cdotc_k(int32_t n, float *x, int32_t inc_x, float *y, int32_t inc_y);
int32_t cher2k_k(uint32_t uplo, uint32_t trans, int32_t n, int32_t k, float alphar, float alphai, float *a, int32_t lda,
                 float *b, int32_t ldb, float beta, float *c__, int32_t ldc);
int32_t cgemm_k(uint32_t transa, uint32_t transb, int32_t m, int32_t n, int32_t k, float alphar, float alphai, float *a,
                int32_t lda, float *b, int32_t ldb, float betar, float betai, float *c, int32_t ldc);
int32_t ctrmm_k(uint32_t side, uint32_t uplo, uint32_t transa, uint32_t diag, int32_t m, int32_t n, float alphar,
                float alphai, float *a, int32_t lda, float *b, int32_t ldb);
#endif
