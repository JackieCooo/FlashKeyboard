#include "soc_osal.h"
#include "app_init.h"
#include "common_def.h"


static int main_app_task(void *arg)
{
    unused(arg);

    while (1) {
        osal_msleep(1000);
        osal_printk("working.\r\n");
    }

    return 0;
}

static void main_app_entry(void)
{
    osal_task *task_handle = NULL;

    osal_kthread_lock();
    task_handle = osal_kthread_create(main_app_task, NULL, "main_app_task", 4096);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, 12);
    }
    osal_kthread_unlock();
}

app_run(main_app_entry);
