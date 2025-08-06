#include "soc_osal.h"
#include "app_os_init.h"
#include "common_def.h"
#include "std_def.h"

#include "key_scan.h"
#include "ble_keyboard_server.h"

#define LOG_TAG "main"
#include "debug.h"


void app_main(void *unused)
{
    UNUSED(unused);

    key_scan_init();

    /* Init BLE */
    ble_keyboard_server_init();

    while (1) {
        key_scan();
        osal_msleep(10);
    }
}
