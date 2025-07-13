/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description:  OAM TRACE for RISCV31
 *
 * Create: 2021-10-25
 */
#include "oam_trace.h"
#include "memory_info.h"
#include "print_config.h"
#include "arch_encoding.h"
#include "arch_trace.h"
#ifdef SUPPORT_HEAP_PRINT
#include "os_dfx.h"
#endif
#include "los_task.h"
#include "los_task_pri.h"

#define USER_STACK_OFFSET_PER   4
#define USER_STACK_PRINT_DEPTH  30
#define SP_DEFAULT_VALUE 0xcacacaca

EXC_PROC_FUNC g_exec_hook;

static void back_trace(uint32_t sp)
{
    uint32_t back_sp = sp;
    uint32_t count = 0;
    TSK_INFO_S taskinfo;
    uint32_t back_sp_top = 0;
    uint32_t back_sp_size = 0;
    uint32_t ret = 0;
#ifdef SUPPORT_HEAP_PRINT
    print_os_dfx_info();
#endif
    for (uint32_t loop = 0; loop < g_taskMaxNum + 1; loop++) {
        ret = LOS_TaskInfoGet(loop, &taskinfo);
        if (ret != LOS_OK) {
            continue;
        }
        if ((taskinfo.usTaskStatus & OS_TASK_STATUS_UNUSED) != 0) {
            continue;
        }
        if ((back_sp >= taskinfo.uwTopOfStack) && (back_sp < (taskinfo.uwTopOfStack + taskinfo.uwStackSize))) {
            back_sp_top = taskinfo.uwTopOfStack;
            back_sp_size = taskinfo.uwStackSize;
            break;
        }
    }
    oam_trace_print("*******backtrace begin*******\n");

    while (back_sp_size >= USER_STACK_OFFSET_PER) {
        if (*((uint32_t *)(back_sp_top)) != SP_DEFAULT_VALUE) {
            if (back_sp_top == back_sp) {
                oam_trace_print("*******current sp*******\n");
            }
            oam_trace_print("traceback %d -- sp addr= 0x%x   sp content= 0x%x\n", \
                            count++, back_sp_top, *((uint32_t *)(back_sp_top)));
        }
        back_sp_top = back_sp_top + USER_STACK_OFFSET_PER;
        back_sp_size -= USER_STACK_OFFSET_PER;
    }
    oam_trace_print("*******backtrace end*******\n");
    UNUSED(count);
}

void riscv_cpu_trace_print(void)
{
    riscv_cpu_trace_item_t *item = (riscv_cpu_trace_item_t *)(uintptr_t)(RISCV_TRACE_MEM_REGION_START);
    uint32_t cpu_trace_end_addr = RISCV_TRACE_MEM_REGION_START + RISCV_TRACE_MEM_REGION_LENGTH;

    oam_trace_print("**************CPU Trace Information************** \n");

    for (uint32_t i = 0; &item[i] < (riscv_cpu_trace_item_t *)(uintptr_t)(cpu_trace_end_addr); i++) {
        oam_trace_print("cpu trace %d:\r\n", i);
        oam_trace_print("step time:0x%x   LR:0x%x   PC:0x%x\r\n", item[i].time, item[i].lr, item[i].pc);
    }

    oam_trace_print("**************CPU Trace end*********************** \n");
}

static void print_excptsc(void)
{
    oam_trace_print("cxcptsc = 0x%x\n", read_custom_csr(CXCPTSC));
}

#ifdef USE_CMSIS_OS
void exec_fault_handler(uint32_t exc_type, const ExcContext *exc_buff_addr)
{
#ifndef DFX_FAST_REBOOT
    riscv_cpu_trace_disable();
    if (g_exec_hook != NULL) {
        g_exec_hook(exc_type, exc_buff_addr);
    }
    // custom oam information
    print_excptsc();
    if (exc_buff_addr != NULL) {
        back_trace(exc_buff_addr->taskContext.sp);
    }
    riscv_cpu_trace_print();
#else
    unused(exc_type);
    unused(exc_buff_addr);
#endif
}
#endif

void register_os_exec_hook(void)
{
    g_exec_hook = ArchGetExcHook();
    ArchSetExcHook((EXC_PROC_FUNC)exec_fault_handler);
}
