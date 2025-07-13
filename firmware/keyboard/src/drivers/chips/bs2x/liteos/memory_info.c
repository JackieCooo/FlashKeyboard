/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2024. All rights reserved.
 * Description:  LiteOs Heap and Stack info
 *
 * Create: 2022-09-27
 */

#include <stdint.h>
#if defined(PM_MCPU_MIPS_STATISTICS_ENABLE) && (PM_MCPU_MIPS_STATISTICS_ENABLE == YES)
#include "log_uart.h"
#include "log_common.h"
#include "log_printf.h"
#include "log_def.h"
#endif
#include "los_task_pri.h"
#include "los_memory.h"
#include "chip_io.h"
#include "osal_debug.h"
#include "memory_info.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

void print_os_task_id_and_name(void)
{
    TSK_INFO_S taskinfo;
    uint32_t ret = 0;
    for (uint32_t loop = 0; loop < g_taskMaxNum + 1; loop++) {
        ret = LOS_TaskInfoGet(loop, &taskinfo);
        if (ret != LOS_OK) {
            continue;
        }
        osal_printk("task_id: %d, task_name: %s\n", taskinfo.uwTaskID, taskinfo.acName);
    }
}

void print_stack_waterline_riscv(void)
{
    if (readl(MEMORY_INFO_CRTL_REG) == MEMORY_INFO_CLOSE) { return; }
    TSK_INFO_S taskinfo;
    uint32_t ret = 0;

    osal_printk("task_id  taskName          stackTop   stackLen   peakUsage    sp       peakRatio\r\n");
    for (uint32_t loop = 0; loop < g_taskMaxNum + 1; loop++) {
        ret = LOS_TaskInfoGet(loop, &taskinfo);
        if (ret != LOS_OK) {
            continue;
        }
        if ((taskinfo.usTaskStatus & OS_TASK_STATUS_UNUSED) != 0) {
            continue;
        }
#if defined(PM_MCPU_MIPS_STATISTICS_ENABLE) && (PM_MCPU_MIPS_STATISTICS_ENABLE == YES)
        oml_pf_log_print_alter(LOG_BCORE_PLT_INFO_STACK, LOG_NUM_INFO_STACK, LOG_LEVEL_INFO, \
            "[STACK] id:%d, top:0x%x, size:0x%x, task usage peak:0x%x, sp:0x%x", FIVE_ARG, \
            (uint32_t)(taskinfo.uwTaskID), (uint32_t)(taskinfo.uwTopOfStack), \
            (uint32_t)(taskinfo.uwStackSize), (uint32_t)(taskinfo.uwPeakUsed), (uint32_t)((uintptr_t)(taskinfo.uwSP)));
#endif
        osal_printk("%02d      %-18s 0x%08X 0x%08X 0x%08X 0x%08X %02d%%\r\n", \
            taskinfo.uwTaskID, taskinfo.acName, \
            taskinfo.uwTopOfStack, taskinfo.uwStackSize, taskinfo.uwPeakUsed, (uint32_t)((uintptr_t)(taskinfo.uwSP)), \
            taskinfo.uwPeakUsed * 100 / taskinfo.uwStackSize); // * 100 for calculate percent.
    }
}

void print_heap_statistics_riscv(void)
{
    if (readl(MEMORY_INFO_CRTL_REG) == MEMORY_INFO_CLOSE) { return; }
    LOS_MEM_POOL_STATUS status;

    LOS_MemInfoGet(OS_SYS_MEM_ADDR, &status);
#if defined(PM_MCPU_MIPS_STATISTICS_ENABLE) && (PM_MCPU_MIPS_STATISTICS_ENABLE == YES)
    oml_pf_log_print4(LOG_BCORE_PLT_INFO_HEAP, LOG_NUM_INFO_HEAP, LOG_LEVEL_INFO,
                      "[HEAP_STAT1] total:0x%x, used:0x%x, free:0x%x, usage waterline:0x%x",
                      (uint32_t)(status.uwTotalFreeSize + status.uwTotalUsedSize),
                      (uint32_t)(status.uwTotalUsedSize), (uint32_t)(status.uwTotalFreeSize),
                      (uint32_t)(status.uwUsageWaterLine));
#endif
    osal_printk("[SysHeap stat] total:0x%x, used:0x%x, current free:0x%x, peak usage:0x%x, peak free:0x%x\r\n", \
                (uint32_t)(status.uwTotalFreeSize + status.uwTotalUsedSize), \
                (uint32_t)(status.uwTotalUsedSize), (uint32_t)(status.uwTotalFreeSize), \
                (uint32_t)(status.uwUsageWaterLine), \
                (uint32_t)(status.uwTotalFreeSize + status.uwTotalUsedSize - status.uwUsageWaterLine));

    /* print all task heap usage info */
    osal_printk("Idx     TaskName        current malloc  peak malloc\r\n");
    for (uint8_t idx = 0; idx < g_taskMaxNum + 1; idx++) {
        if (idx == g_taskMaxNum) {
            osal_printk("---------non-task alloc(e.g. startup stage)----------\r\n");
        }
        LOS_TaskMemInfoShow((void *)OS_SYS_MEM_ADDR, idx, osal_printk);
    }
    osal_printk("Done\r\n");
}

void print_os_sys_task_heap(uint8_t taskid)
{
    LOS_MemTaskHeapInfoGet((void *)OS_SYS_MEM_ADDR, taskid, osal_printk);
}

void print_os_all_sys_task_heap(void)
{
    for (uint8_t idx = 0; idx < g_taskMaxNum + 1; idx++) {
        print_os_sys_task_heap(idx);
    }
}

void print_os_dfx_info(void)
{
    print_stack_waterline_riscv();
    print_heap_statistics_riscv();
    print_os_all_sys_task_heap();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */