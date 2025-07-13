/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * Description: Application core os initialize interface for standard \n
 *
 * History: \n
 * 2022-07-27, Create file. \n
 */

#include "soc_osal.h"
#include "log_common.h"
#include "log_def.h"
#include "log_uart.h"
#include "pmu_interrupt.h"
#include "log_reg_dump.h"
#include "watchdog.h"
#include "preserve.h"
#include "hal_reboot.h"
#ifdef OS_DFX_SUPPORT
#include "os_dfx.h"
#endif
#include "los_task_pri.h"
#include "debug_print.h"

#if (PM_MCPU_MIPS_STATISTICS_ENABLE == YES)
#include "pm_porting.h"
#endif

#include "chip_io.h"
#if defined(CONFIG_SAMPLE_ENABLE)
#include "app_init.h"
#endif
#if defined(SUPPORT_CARKEY)
#include "carkey_api.h"
#include "carkey_uapi.h"
#endif
#ifdef AT_COMMAND
#include "at_product.h"
#include "at_porting.h"
#endif
#ifdef TEST_SUITE
#include "main_test.h"
#endif
#include "memory_info.h"
#include "app_os_init.h"

/*
 *  优先级范围 0-31
 *  0：     SWT 线程优先级  （最高 不可更改）。
 *  1-10    协议栈线程优先级 （不可更改）。
 *  11-29   用户可使用。
 *  30      平台低功耗 （不可更改）。
 *  31：    IDLE 线程优先级（最低 不可更改）。
 */
/* 平台线程优先级 */
#define TASK_PRIORITY_CMD                   20
#define TASK_PRIORITY_APP                   21
#define TASK_PRIORITY_LOG                   29
/* 业务线程优先级 */
/* controller task */
#define TASK_PRIORITY_BT                    3
/* host task. */
#define TASK_PRIORITY_SCH                   4
#define TASK_PRIORITY_SRV                   5
#define TASK_PRIORITY_HADM                  27

#ifdef SLEM_CARKEY
#ifdef OS_DFX_SUPPORT
#define APP_STACK_SIZE                      0x800
#else
#define APP_STACK_SIZE                      0x500
#endif
#define LOG_STACK_SIZE                      0x600
#define AT_STACK_SIZE                       0x700
#define BT_STACK_SIZE                       0xA00
/* host schedule task stack. */
#define BTH_SCHEDULE_STACK_SIZE             0xC00
/* host service task stack. */
#define BTH_SERVICE_STACK_SIZE              0x900
/* host measure task stack. */
#ifdef CONFIG_SINGLE_CARKEY
#define BTH_HADM_SERVICE_STACK_SIZE         0xA00
#else
#define BTH_HADM_SERVICE_STACK_SIZE         0x1100
#endif
#else
/* 线程栈分配，推荐比水线高30%  */
#define APP_STACK_SIZE                      0xA00
#define LOG_STACK_SIZE                      0x600
#define AT_STACK_SIZE                       0xC00
#define BT_STACK_SIZE                       0xA00
/* host schedule task stack. */
#define BTH_SCHEDULE_STACK_SIZE             0xC00
/* host service task stack. */
#define BTH_SERVICE_STACK_SIZE              0xC00
#endif

#define TASK_COMMON_APP_DELAY_MS            20000

typedef struct {
    char *task_name;
    void *task_arg;
    uint32_t task_stack;
    uint32_t task_pri;
    osal_kthread_handler task_func;
} app_task_attr_t;

typedef struct {
    int task_prio;
    int stack_size;
} task_attr_t;

/* plt init thread. */
void app_main(void *unused);
/* rf init thread. */
void bt_thread_handle(void *para);

#ifndef DEVICE_ONLY
void bt_acore_task_main(const void *pvParams);
void bt_tran_task_queue_init(void);
void recv_data_task(void);
void btsrv_task_body(const void *data);
void sdk_msg_thread(void);
#endif
#if defined(CONFIG_SLE_AMIC_TRANS_PATH_CHECK)
void sle_vdt_get_trans_path_value(void);
#endif

/* bth schedule task attr. */
const task_attr_t g_bth_schedule_attr = {
    .task_prio = TASK_PRIORITY_SCH,
    .stack_size = BTH_SCHEDULE_STACK_SIZE,
};

/*
 *  开启内存维测：
 *  将 kernel\liteos\liteos_v208.6.0_b017\Huawei_LiteOS\tools\build\config\bs2x_xxx.config 文件内下列配置设置为 y。
 *      LOSCFG_MEM_TASK_STAT=y
 *      LOSCFG_MEM_DFX_SHOW_CALLER_RA=y
 *  查看内存维测信息：
 *      AT+TASKSTACK：查看所有线程栈使用信息，若线程栈空闲较多，建议减小栈大小，节省内存空间。
 *      AT+TASKMALLOC：查看所有线程内存堆申请信息，若线程堆申请较大，建议排查线程调用内存申请释放接口是否合理。
 *      AT+HEAPSTAT：查看堆整体使用信息。
 *  线程栈溢出常用分析方法：
 *      1）使用AT+TASKSTACK观察栈使用峰值是否接近栈顶。
 *      2）异常死机时查看串口打印的栈信息log，如果PeakUsage大于或者接近栈大小，则可能存在栈溢出。
 */

/*
 *  线程创建数组，数量上限通过：
 *  kernel\liteos\liteos_v208.6.0_b017\Huawei_LiteOS\tools\build\config\bs2x_xxx.config
 *  文件内的 LOSCFG_BASE_CORE_TSK_LIMIT=* 配置项进行配置。
 *  注意：线程越多，消耗内存越多，请合理配置使用。
 *  当前数组定义了7个线程:
 *      app: 平台SMAPLE主线程（用户可修改）。
 *      log: 平台LOG打印线程（用户可裁剪）。
 *          裁剪方式：Kconfig配置界面开关 Drivers → Chips → Chip Configurations for bs2x -> CREATE LOG MAIN THREAD.选项
 *      at: 平台AT命令线程（用户可修改）。
 *      bt: 协议栈controller层主线程（不可修改）。
 *      bt_service: 协议栈host层主线程（不可修改），线程内部创建bth_schedule线程（不可修改）。
 *      key: 星闪测距线程（可根据实际需求裁剪）。
 *  os隐式创建2个线程：SWT、IDLE（仅支持修改线程栈）。
 *      线程栈修改方式：修改kernel\liteos\liteos_v208.6.0_b017\Huawei_LiteOS\tools\build\config\bs2x_xxx.config文件内
 *                      LOSCFG_BASE_CORE_TSK_SWTMR_STACK_SIZE、LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE配置项大小。
 *  平台隐式创建1个线程：PM_ENTRY（仅支持修改线程栈）。
 *  包含测距业务总共创建10个线程，不包含总共创建9个线程。
 */
static const app_task_attr_t g_app_tasks[] = {
    {"app", NULL, APP_STACK_SIZE, TASK_PRIORITY_APP, (osal_kthread_handler)app_main},
#if (USE_COMPRESS_LOG_INSTEAD_OF_SDT_LOG == NO) && defined(CONFIG_SUPPORT_LOG_THREAD)
    {"log", NULL, LOG_STACK_SIZE, TASK_PRIORITY_LOG, (osal_kthread_handler)log_main},
#endif
#ifdef AT_COMMAND
    {"at", NULL, AT_STACK_SIZE, TASK_PRIORITY_CMD, (osal_kthread_handler)uapi_at_msg_main},
#endif
#ifdef BGLE_TASK_EXIST
    {"bt", NULL, BT_STACK_SIZE, TASK_PRIORITY_BT, (osal_kthread_handler)bt_thread_handle},
#ifndef DEVICE_ONLY
    {"bt_service", (void *)&g_bth_schedule_attr, BTH_SERVICE_STACK_SIZE, TASK_PRIORITY_SRV,
     (osal_kthread_handler)btsrv_task_body},
#endif
#ifdef SUPPORT_CARKEY
    {"key", NULL, BTH_HADM_SERVICE_STACK_SIZE, TASK_PRIORITY_HADM, (osal_kthread_handler)key_main},
#endif /* SUPPROT_CARKEY */
#endif
};

#define M_NUM_TASKS (sizeof(g_app_tasks) / sizeof(app_task_attr_t))

__attribute__((weak)) void app_os_init(void)
{
    osal_task *cur_handle = NULL;
    /* 线程创建接口 */
    osal_kthread_lock();
    for (uint8_t i = 0; i < M_NUM_TASKS; i++) {
        cur_handle = osal_kthread_create(g_app_tasks[i].task_func, g_app_tasks[i].task_arg,
                                         g_app_tasks[i].task_name, g_app_tasks[i].task_stack);
        if (cur_handle == NULL) {
            panic(PANIC_TASK_CREATE_FAILED, i);
        }
        osal_kthread_set_priority(cur_handle, g_app_tasks[i].task_pri);
        /* AT命令线程 */
#ifdef TEST_SUITE
#ifdef AT_COMMAND
        if (strcmp(g_app_tasks[i].task_name, "at") == 0) {
            uapi_set_at_task((uint32_t *)cur_handle->task);
            osal_kthread_suspend(cur_handle);
        }
#endif
#endif
    }
    osal_kthread_unlock();
#ifdef TEST_SUITE
    cmd_main_add_functions();
#endif
}

__attribute__((weak)) void app_main(void *unused)
{
    UNUSED(unused);
    hal_reboot_clear_history();
    system_boot_reason_print();
    system_boot_reason_process();
#if (USE_COMPRESS_LOG_INSTEAD_OF_SDT_LOG == NO)
    log_exception_dump_reg_check();
#endif
#if defined(CONFIG_SAMPLE_ENABLE)
    app_tasks_init();
#endif
#ifdef OS_DFX_SUPPORT
    print_os_task_id_and_name();
#endif

    while (1) {  //lint !e716 Main Loop
        (void)osal_msleep(TASK_COMMON_APP_DELAY_MS);
        oml_pf_log_print0(LOG_BCORE_PLT_DRIVER_REBOOT, LOG_NUM_DEBUG, LOG_LEVEL_INFO, "App main");
#if defined(PM_MCPU_MIPS_STATISTICS_ENABLE) && (PM_MCPU_MIPS_STATISTICS_ENABLE == YES)
        oml_pf_log_print2(LOG_BCORE_PLT, LOG_NUM_DEBUG, LOG_LEVEL_INFO,
            "[Mcpu mips statistics] total work time: %dus, total idle time: %dus.\r\n",
            pm_get_total_work_time(), pm_get_total_idle_time());
        PRINT("[Mcpu mips statistics] total work time: %dus, total idle time: %dus.\r\n",
            pm_get_total_work_time(), pm_get_total_idle_time());
#endif
        uapi_watchdog_kick();
#if defined(CONFIG_SLE_AMIC_TRANS_PATH_CHECK)
        sle_vdt_get_trans_path_value();
#endif
#if defined(MEMORY_INFO_PRINT_SUPPORT)
        print_stack_waterline_riscv();
        print_heap_statistics_riscv();
#endif
#ifdef OS_DFX_SUPPORT
        os_dfx_print_info();
#endif
    }
}