/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: at plt cmd func \n
 *
 */

#include "at_table.h"
#include "at_cmd.h"
#include "cpu_utils.h"
#include "hal_reboot.h"
#include "memory_info.h"
#if (ENABLE_LOW_POWER == YES)
#include "pm_veto.h"
#endif
#include "chip_io.h"
#include "osal_debug.h"
#ifdef OS_TIMER_DEBUG_SUPPORT
#include "os_dfx.h"
#endif
#ifdef LOSCFG_KERNEL_CPUP
#include "los_task_pri.h"
#include "los_hwi_pri.h"
#endif

#ifdef REG_OPERATION
#define ADDR_OFFSET 4
#endif

typedef struct {
    uint32_t para_map;
    uint8_t  task_id;
} plt_at_heap_stat_t;

const at_para_parse_syntax_t g_plt_at_heap_stats_params[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = 0,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 10,
        .offset = offsetof(plt_at_heap_stat_t, task_id)
    },
};

#ifdef REG_OPERATION
typedef struct {
    uint32_t para_map;
    uint32_t reg_addr;
    uint32_t reg_value;
} plt_at_write_reg_t;

typedef struct {
    uint32_t para_map;
    uint32_t reg_addr;
    uint8_t reg_len;
} plt_at_read_reg_t;

static at_ret_t plt_at_read_reg(const plt_at_read_reg_t *arg);
static at_ret_t plt_at_write_reg(const plt_at_write_reg_t *arg);

const at_para_parse_syntax_t g_plt_at_write_reg_params[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 0x7FFFFFF0,
        .offset = offsetof(plt_at_write_reg_t, reg_addr)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 0x7FFFFFF0,
        .offset = offsetof(plt_at_write_reg_t, reg_value)
    },
};

const at_para_parse_syntax_t g_plt_at_read_reg_params[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 0x7FFFFFF0,
        .offset = offsetof(plt_at_read_reg_t, reg_addr)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 100,
        .offset = offsetof(plt_at_read_reg_t, reg_len)
    },
};
#endif

#if (ENABLE_LOW_POWER == YES)
typedef struct {
    uint32_t para_map;
    int32_t para1; /* Range: 0~1 */
} at_set_sleep_args_t;

const at_para_parse_syntax_t g_at_sleep_para[] = {
    {
        .last = true,
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 1,
        .offset = offsetof(at_set_sleep_args_t, para1),
    }
};

static at_ret_t plt_at_set_vote_sleep(at_set_sleep_args_t *vote);
#ifdef SLP_VETO_AT_SUPPORT
static at_ret_t plt_pm_veto_print(void);
#endif
#endif
static at_ret_t plt_at_reboot(void);
static at_ret_t plt_at_help(void);
static at_ret_t plt_heap_stats(void);
static at_ret_t plt_task_stack_stats(void);
static at_ret_t plt_task_heap_stats(const plt_at_heap_stat_t *arg);
#ifdef OSAL_IRQ_RECORD_DEBUG
static at_ret_t plt_int_lock_dump(void);
#endif
#ifdef OS_TIMER_DEBUG_SUPPORT
static at_ret_t plt_os_timer_print(void);
#endif
#ifdef OS_DFX_SUPPORT
static at_ret_t plt_os_dfx_print(void);
#endif
#if defined(LOSCFG_KERNEL_CPUP)
at_ret_t plt_cpup_stats(void);
#endif

const at_cmd_entry_t at_plt_parse_table[] = {
    {
        "HELP",
        1,
        0,
        NULL,
        (at_cmd_func_t)plt_at_help,
        NULL,
        NULL,
        NULL,
    },
    {
        "REBOOT",
        2,
        0,
        NULL,
        (at_cmd_func_t)plt_at_reboot,
        NULL,
        NULL,
        NULL,
    },
#if (ENABLE_LOW_POWER == YES)
    {
        "SETSLEEP",
        3,
        0,
        g_at_sleep_para,
        NULL,
        (at_set_func_t)plt_at_set_vote_sleep,
        NULL,
        NULL,
    },
#endif
#ifdef TEST_SUITE
    {
        "TESTSUITE",
        4,
        0,
        NULL,
        (at_cmd_func_t)uapi_at_sw_testsuite,
        NULL,
        NULL,
        NULL,
    },
#endif
    {
        "HEAPSTAT",
        5,
        0,
        NULL,
        plt_heap_stats,
        NULL,
        NULL,
        NULL,
    },
    {
        "TASKSTACK",
        6,
        0,
        NULL,
        plt_task_stack_stats,
        NULL,
        NULL,
        NULL,
    },
    {
        "TASKMALLOC",
        7,
        0,
        g_plt_at_heap_stats_params,
        NULL,
        (at_set_func_t)plt_task_heap_stats,
        NULL,
        NULL,
    },
#ifdef REG_OPERATION
    {
        "WRITEREG",
        8,
        0,
        g_plt_at_write_reg_params,
        NULL,
        (at_set_func_t)plt_at_write_reg,
        NULL,
        NULL,
    },
    {
        "READREG",
        9,
        0,
        g_plt_at_read_reg_params,
        NULL,
        (at_set_func_t)plt_at_read_reg,
        NULL,
        NULL,
    },
#endif
#ifdef OSAL_IRQ_RECORD_DEBUG
    {
        "INTLOCKDUMP",
        10,
        0,
        NULL,
        plt_int_lock_dump,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef OS_TIMER_DEBUG_SUPPORT
    {
        "OSTIMERPRINT",
        11,
        0,
        NULL,
        plt_os_timer_print,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef OS_DFX_SUPPORT
    {
        "OSDFXPRINT",
        12,
        0,
        NULL,
        plt_os_dfx_print,
        NULL,
        NULL,
        NULL,
    },
#endif
#if (ENABLE_LOW_POWER == YES)
#ifdef SLP_VETO_AT_SUPPORT
    {
        "PMVETOINFO",
        13,
        0,
        NULL,
        plt_pm_veto_print,
        NULL,
        NULL,
        NULL,
    },
#endif
#endif
#ifdef LOSCFG_KERNEL_CPUP
    {
        "CPUP",
        11,
        0,
        NULL,
        plt_cpup_stats,
        NULL,
        NULL,
        NULL,
    },
#endif
};

#if (ENABLE_LOW_POWER == YES)
static at_ret_t plt_at_set_vote_sleep(at_set_sleep_args_t *vote)
{
    if (vote->para1 == 0) {
        osal_printk("disable enter sleep.\r\n");
        uapi_pm_add_sleep_veto(PM_VETO_ID_MCU);
        uapi_reg_write32(MEMORY_INFO_CRTL_REG, MEMORY_INFO_CLOSE);
    } else {
        osal_printk("enable enter sleep.\r\n");
        uapi_pm_remove_sleep_veto(PM_VETO_ID_MCU);
    }
    return AT_RET_OK;
}

#ifdef SLP_VETO_AT_SUPPORT
extern uint8_t g_pmu_cur_state;
at_ret_t plt_pm_veto_print(void)
{
    pm_veto_t *veto_info = uapi_pm_veto_get_info();
    osal_printk("[pm_veto]: total_counts = %d\n", veto_info->veto_counts.total_counts);
    if (veto_info->veto_counts.total_counts == 0) {
        return AT_RET_OK;
    }
    for (int i = 0; i < PM_VETO_ID_MAX; i++) {
        if (veto_info->veto_counts.sub_counts[i] == 0) {
            continue;
        }
        osal_printk("[pm_veto]: pm_sub_veto_id = %d\n", i);
    }
    if (veto_info->veto_counts.sub_counts[PM_VETO_ID_BTC] != 0) {
        osal_printk("[pm_veto]: bt_cur_state = %d\n", g_pmu_cur_state);
    }
    return AT_RET_OK;
}
#endif
#endif

static at_ret_t plt_at_reboot(void)
{
    cpu_utils_reset_chip_with_cause(REBOOT_CAUSE_APPLICATION_SYSRESETREQ);
    /* never arrive here. */
    return AT_RET_PROC_ABORT_CURRENT_COMMAND;
}

at_ret_t plt_heap_stats(void)
{
    print_heap_statistics_riscv();
    return AT_RET_OK;
}

at_ret_t plt_task_stack_stats(void)
{
    print_stack_waterline_riscv();
    return AT_RET_OK;
}

at_ret_t plt_task_heap_stats(const plt_at_heap_stat_t *arg)
{
    print_os_sys_task_heap(arg->task_id);
    return AT_RET_OK;
}

#ifdef REG_OPERATION
at_ret_t plt_at_read_reg(const plt_at_read_reg_t *arg)
{
    for (uint8_t i = 0; i < arg->reg_len; i++) {
        uint32_t addr = arg->reg_addr + i * ADDR_OFFSET;
        osal_printk("addr:%x = %x\r\n", addr, readl(addr));
        unused(addr);
    }
    return AT_RET_OK;
}

at_ret_t plt_at_write_reg(const plt_at_write_reg_t *arg)
{
    writel(arg->reg_addr, arg->reg_value);
    return AT_RET_OK;
}
#endif

#ifdef OSAL_IRQ_RECORD_DEBUG
at_ret_t plt_int_lock_dump(void)
{
    osal_print_irq_record();
    return AT_RET_OK;
}
#endif

#ifdef OS_TIMER_DEBUG_SUPPORT
at_ret_t plt_os_timer_print(void)
{
    os_task_timer_print();
    os_sw_timer_print();
    return AT_RET_OK;
}
#endif

#ifdef OS_DFX_SUPPORT
at_ret_t plt_os_dfx_print(void)
{
    print_os_task_id_and_name();
    os_dfx_print_info();
    return AT_RET_OK;
}
#endif

// at help dump cmd lists
static at_ret_t plt_at_help(void)
{
    uint32_t i;
    uint32_t cnt = at_cmd_get_entry_total();
    uint32_t total = 0;
    at_cmd_entry_t *cmd_entry = NULL;
    at_cmd_entry_t **cmd_tbl = (at_cmd_entry_t **)osal_kmalloc(sizeof(at_cmd_entry_t *) * cnt, 0);

    if (cmd_tbl == NULL) {
        return AT_RET_MALLOC_ERROR;
    }
    if (at_cmd_get_all_entrys((const at_cmd_entry_t **)cmd_tbl, cnt) != cnt) {
        osal_kfree(cmd_tbl);
        return AT_RET_MEM_API_ERROR;
    }
    osal_printk("+HELP:cmd cnt:%d\r\n", cnt);
    for (i = 0; i < cnt; ++i) {
        cmd_entry = (at_cmd_entry_t *)cmd_tbl[i];
        osal_printk("AT+%-28s ", cmd_entry->name);
        total++;
        if (total % 3 == 0) {  /* 3 entrys per newline */
            osal_printk("\r\n");
        }
    }
    osal_kfree(cmd_tbl);
    return AT_RET_OK;
}

uint32_t uapi_get_plt_table_size(void)
{
    return sizeof(at_plt_parse_table) / sizeof(at_cmd_entry_t);
}

const at_cmd_entry_t* uapi_get_plt_at_table(void)
{
    return at_plt_parse_table;
}

#ifdef LOSCFG_KERNEL_CPUP
at_ret_t plt_cpup_stats(void)
{
#if defined(LOSCFG_DEBUG_TASK)
    OsDbgTskInfoGet(OS_ALL_TASK_MASK);
#endif
#if defined(LOSCFG_DEBUG_HWI)
    PrintHwiInfo();
#endif
    return AT_RET_OK;
}
#endif