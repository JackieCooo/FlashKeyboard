#include <string.h>

#include "soc_osal.h"
#include "app_os_init.h"
#include "common_def.h"

#include "oled.h"
#include "key_scan.h"
#include "service_controller.h"

#define LOG_TAG "main"
#include "debug.h"

static uint8_t data[3 * 128];

void app_main(void *unused)
{
    (void)(unused);

    /* 初始化OLED */
    oled_init();
    memset(data, 0xFF, sizeof(data));
    oled_set_data(0, 0, 128, 16, data);
    /* 初始化按键扫描 */
    // (void) key_scan_init();
    /* 初始化服务控制器 */
    // service_ctrl_init();

    while (1) {
        // service_ctrl_detect();
        osal_msleep(10);
    }
}
