#include "key_scan.h"

#include <stdint.h>
#include <string.h>

#include "keyscan.h"
#include "soc_osal.h"
#include "service_controller.h"

#define LOG_TAG "key_scan"
#include "debug.h"


/* 键位映射（先行再列） */
static uint8_t g_key_gpio_map[CONFIG_KEYSCAN_ENABLE_ROW + CONFIG_KEYSCAN_ENABLE_COL] = {
    21, 22, 23, 24
};
static uint8_t g_key_map[CONFIG_KEYSCAN_ENABLE_ROW][CONFIG_KEYSCAN_ENABLE_COL] = {
    { 0x00, 0x01 },
    { 0x02, 0x03 }
};

static int key_scan_report_callback(int key_nums, uint8_t key_values[])
{
    static uint32_t pre = 0;
    static uint32_t cur = 0;

    for (int i = 0; i < key_nums; i++) {
        cur |= (1 << key_values[i]);
    }

    for (uint32_t p = 1, i = 0; i < CONFIG_KEY_NUM; i++, p <<= 1) {
        if (!(cur & p) && (pre & p)) {
            LOG("key %d pressed", i);
            service_ctrl_send_key((uint8_t) i);
        }
    }

    pre = cur;
    cur = 0;

    return 0;
}

int key_scan_init(void)
{
    int ret = ERRCODE_SUCC;

    ret = keyscan_porting_set_gpio(g_key_gpio_map);
    if (ret) {
        LOG("set key gpio failed");
        return ret;
    }
    ret = uapi_set_keyscan_value_map((uint8_t **) g_key_map, CONFIG_KEYSCAN_ENABLE_ROW, CONFIG_KEYSCAN_ENABLE_COL);
    if (ret != ERRCODE_SUCC) {
        LOG("set key map failed");
        return ret;
    }
    uapi_keyscan_init(EVERY_ROW_PULSE_40_US, HAL_KEYSCAN_MODE_2, KEYSCAN_INT_VALUE_RDY);
    ret = uapi_keyscan_register_callback(key_scan_report_callback);
    if (ret != ERRCODE_SUCC) {
        LOG("set irq callback failed");
        return ret;
    }
    if (uapi_keyscan_enable() != ERRCODE_KEYSCAN_POWER_ON) {
        LOG("key scan enable start failed");
        return -1;
    }

    return ret;
}
