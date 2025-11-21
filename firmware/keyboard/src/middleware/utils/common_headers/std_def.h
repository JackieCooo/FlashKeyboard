/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description:   his should be #included by every C source file
 * Author:
 * Create:  2018-10-15
 */

#ifndef STD_DEF_H
#define STD_DEF_H

/**
 * @defgroup connectivity_libs_common_types COMMON Types
 * @ingroup  connectivity_libs_common
 * @{
 */
#include <stddef.h>
#include <stdbool.h>
#include "stdint.h"
#ifdef __LITEOS__
#include "los_typedef.h"
#endif
#include "product.h"

#ifndef var_segment
#if (defined __GNUC__) || (defined __ICCARM__)
#define var_segment(seg) __attribute__((section(seg)))
#else
#if (defined PC_ST)
#define var_segment(seg)
#else
#error "You need to define var_segment for your compiler."
#endif  // (defined PC_ST)
#endif  // (defined __GNUC__ )
#endif  // var_segment

#ifndef UNUSED
#define UNUSED(var) do { (void)(var); } while (0)
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define cassert(predicate, file) impl_cassert_line(predicate, __LINE__, file)

#define impl_paste(a, b)                         a##b
#define impl_cassert_line(predicate, line, file) \
    typedef char impl_paste(assertion_failed_##file##_, line)[2 * !!(predicate) - 1]

#ifndef BIT
#define BIT(x) (1UL << (uint32_t)(x))
#endif

// Macro to check at compile time certain expressions not supported by the preprocessor
#define ct_assert(e) enum LENGTH_CHECK { ct_assert_value = 1 / ((!(!(e)))) }

#define __IRQ
#define __ISR __IRQ

enum ERROR_CODE_ENUM {
    ERR = -1,
    SUCC = 0
};

/**
 * @}
 */
#endif
