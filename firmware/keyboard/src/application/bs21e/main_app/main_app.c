#include "soc_osal.h"
#include "app_os_init.h"
#include "common_def.h"
#include "std_def.h"

#include "key_scan.h"
#include "ble_keyboard_server.h"
#include "usb_keyboard.h"

#define LOG_TAG "main"
#include "debug.h"


void app_main(void *unused)
{
    UNUSED(unused);

    /* 初始化按键扫描 */
    key_scan_init();

    /* 初始化BLE */
    // ble_keyboard_server_init();

    /* 初始化USB */
    usb_keyboard_init();

    while (1) {
        key_scan();
        osal_msleep(10);
    }
}
