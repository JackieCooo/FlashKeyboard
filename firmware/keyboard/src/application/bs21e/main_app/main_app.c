#include "soc_osal.h"
#include "app_os_init.h"
#include "common_def.h"

#include "key_scan.h"
#include "service_controller.h"

#define LOG_TAG "main"
#include "debug.h"


void app_main(void *unused)
{
    (void)(unused);

    /* 初始化按键扫描 */
    (void) key_scan_init();
    /* 初始化服务控制器 */
    service_ctrl_init();

    while (1) {
        service_ctrl_detect();
        osal_msleep(10);
    }
}
