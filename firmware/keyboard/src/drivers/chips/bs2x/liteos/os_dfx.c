/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Application core main function for standard \n
 *
 * History: \n
 * 2023-11-03, Create file. \n
 */

#include "securec.h"
#include "osal_debug.h"
#include "systick.h"
#include "os_dfx.h"

#ifdef USER_PRINT_OS_DFX
#define TSK_RECORD_MAX 32
#define HWI_RECORD_MAX 32
#else
#define TSK_RECORD_MAX 10
#define HWI_RECORD_MAX 10
#endif
#define OS_HWI_PRE 0x5a
#define OS_HWI_POST 0xa5

typedef struct {
    uint8_t tid_r;
    uint8_t tid_n;
    uint64_t timestamp;
} tsk_record_info_t;

typedef struct {
    uint8_t irqnum;
    uint8_t in_out_flag; // 0x5a: irq enter, 0xa5: irq exits_info_t
    uint64_t timestamp;
} hwi_record_info_t;

typedef struct {
    hwi_record_info_t hwi_info[HWI_RECORD_MAX];
    tsk_record_info_t tsk_info[TSK_RECORD_MAX];
} os_info_t;

#ifdef USER_PRINT_OS_DFX
os_info_t g_trace = {0}; // segment is in itcm
#else
__attribute__((section("os_info")))os_info_t g_trace = {0}; // segment is in itcm
#endif

static uint8_t g_trace_index = 0;
static uint8_t g_hwi_index = 0;

void os_dfx_trace_init(void)
{
    memset_s((void*)&g_trace, sizeof(os_info_t), 0, sizeof(os_info_t));
}

void os_dfx_task_switch_trace(uint32_t tid_r, uint32_t tid_n)
{
    uint8_t cur_index = g_trace_index % TSK_RECORD_MAX;
    g_trace.tsk_info[cur_index].tid_r = tid_r;
    g_trace.tsk_info[cur_index].tid_n = tid_n;
#ifdef USER_PRINT_OS_DFX
    osal_printk("task:%d,%d\r\n", tid_r, tid_n);
#else
    g_trace.tsk_info[cur_index].timestamp = uapi_systick_get_us();
#endif
    g_trace_index++;
}

void os_dfx_hwi_pre(uint32_t irq_num)
{
    if (irq_num == 0x7) {   // 7号中断（tick）不记录
        return;
    }
    uint8_t cur_index = g_hwi_index % HWI_RECORD_MAX;
    g_trace.hwi_info[cur_index].irqnum = irq_num;
    g_trace.hwi_info[cur_index].in_out_flag = OS_HWI_PRE;
#ifdef USER_PRINT_OS_DFX
    osal_printk("hwi_pre:%d\r\n", irq_num);
#else
    g_trace.hwi_info[cur_index].timestamp = uapi_systick_get_us();
#endif
    g_hwi_index++;
}

void os_dfx_hwi_post(uint32_t irq_num)
{
    if (irq_num == 0x7) {   // 7号中断（tick）不记录
        return;
    }
    uint8_t cur_index = g_hwi_index % HWI_RECORD_MAX;
    g_trace.hwi_info[cur_index].irqnum = irq_num;
    g_trace.hwi_info[cur_index].in_out_flag = OS_HWI_POST;
#ifdef USER_PRINT_OS_DFX
    osal_printk("hwi_post:%d\r\n", irq_num);
#else
    g_trace.hwi_info[cur_index].timestamp = uapi_systick_get_us();
#endif
    g_hwi_index++;
}

void os_dfx_print_info(void)
{
    osal_printk("\n<Task id>:");
    for (int i = 0; i < TSK_RECORD_MAX; i++) {
        osal_printk(" %d", g_trace.tsk_info[(g_trace_index + i) % TSK_RECORD_MAX].tid_n);
    }
    osal_printk("\n<Interrupt num>:");
    for (int i = 0; i < HWI_RECORD_MAX; i++) {
        if (g_trace.hwi_info[(g_hwi_index + i) % HWI_RECORD_MAX].in_out_flag == OS_HWI_PRE) {
            osal_printk(" %d", g_trace.hwi_info[(g_hwi_index + i) % HWI_RECORD_MAX].irqnum);
        }
    }
    osal_printk("\n");
}

#ifdef OS_TIMER_DEBUG_SUPPORT
#include "los_task_pri.h"
#include "los_swtmr_pri.h"

static bool g_os_sw_timer_enable = false;
void os_sw_timer_print_flag_set(bool enable)
{
    g_os_sw_timer_enable = enable;
}

void OsSwtmrDebug(uint32_t handler, uint32_t interval)
{
    if (!g_os_sw_timer_enable) {
        return;
    }
    osal_printk("swtmr:handler = 0x%x, interval = %d(ms)", handler, interval);
}

void os_task_timer_print(void)
{
    SortLinkList *sortList = NULL;
    LosTaskCB *taskCB = NULL;
    LOS_DL_LIST *listObject = NULL;
    SortLinkAttribute *taskSortLink = NULL;

    taskSortLink = &OsPercpuGet()->taskSortLink;
    SORTLINK_CURSOR_UPDATE(taskSortLink->cursor);
    SORTLINK_LISTOBJ_GET(listObject, taskSortLink);

    LOS_SpinLock(&g_taskSpin);

    if (LOS_ListEmpty(listObject)) {
        LOS_SpinUnlock(&g_taskSpin);
        return;
    }

    LOS_DL_LIST_FOR_EACH_ENTRY(sortList, listObject, SortLinkList, sortLinkNode) {
        taskCB = LOS_DL_LIST_ENTRY(sortList, LosTaskCB, sortList);

        osal_printk("[task]: taskId = %d, taskName = %s, taskStatus = %d\n", \
                    taskCB->taskId, taskCB->taskName, taskCB->taskStatus);

        if (LOS_ListEmpty(listObject)) {
            break;
        }
    }

    LOS_SpinUnlock(&g_taskSpin);
}

void os_sw_timer_print(void)
{
    SortLinkList *sortList = NULL;
    LosSwtmrCB *swtmr = NULL;
    LOS_DL_LIST *listObject = NULL;
    SortLinkAttribute* swtmrSortLink = &OsPercpuGet()->swtmrSortLink;

    SORTLINK_CURSOR_UPDATE(swtmrSortLink->cursor);
    SORTLINK_LISTOBJ_GET(listObject, swtmrSortLink);

    LOS_SpinLock(&g_swtmrSpin);

    if (LOS_ListEmpty(listObject)) {
        LOS_SpinUnlock(&g_swtmrSpin);
        return;
    }

    LOS_DL_LIST_FOR_EACH_ENTRY(sortList, listObject, SortLinkList, sortLinkNode) {
        swtmr = LOS_DL_LIST_ENTRY(sortList, LosSwtmrCB, sortList);

        osal_printk("[swtmr]: timerId = 0x%x, handler = 0x%x, interval = %d, state = %d\n", \
                    swtmr->timerId, swtmr->handler, swtmr->interval, swtmr->state);

        if (LOS_ListEmpty(listObject)) {
            break;
        }
    }

    LOS_SpinUnlock(&g_swtmrSpin);
}
#endif