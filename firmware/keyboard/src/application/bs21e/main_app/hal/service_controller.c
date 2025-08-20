#include "service_controller.h"
#include "ble_keyboard_server.h"
#include "usb_keyboard.h"
#include "service_ctrl_defs.h"

#include "gpio.h"
#include "pinctrl.h"

#include <stdbool.h>

#define LOG_TAG "srv_ctrl"
#include "debug.h"


typedef enum {
    SERVICE_NULL = -1,

    SERVICE_USB,
    SERVICE_BLE,
    SERVICE_SLE,

    SERVICE_NUM
} service_node_t;

static service_node_t g_service_type = SERVICE_NULL;
static bool g_service_on = false;
static service_ctrl_func_t* g_funcs[SERVICE_NUM] = {0};


static void service_ctrl_set_pin(uint32_t pin)
{
    uapi_pin_set_mode(pin, HAL_PIO_FUNC_GPIO);
    uapi_pin_set_pull(pin, PIN_PULL_UP);
    uapi_pin_set_ie(pin, PIN_IE_1);
    uapi_gpio_set_dir(pin, GPIO_DIRECTION_INPUT);
}

void service_ctrl_init(void)
{
    /* 初始化检测管脚 */
    service_ctrl_set_pin(CONFIG_BLE_DETECT_PIN);
    service_ctrl_set_pin(CONFIG_SLE_DETECT_PIN);

    /* 注册服务操作回调 */
    g_funcs[SERVICE_USB] = usb_keyboard_get_func();
    g_funcs[SERVICE_BLE] = ble_keyboard_get_func();

    /* 使能服务 */
    service_ctrl_detect();
}

void service_ctrl_deinit(void)
{

}

void service_ctrl_detect(void)
{
    int ret = 0;

    /* 检测模式 */
    service_node_t node = SERVICE_USB;
    if (uapi_gpio_get_val(CONFIG_BLE_DETECT_PIN) == GPIO_LEVEL_LOW) {
        node = SERVICE_BLE;
    } else if (uapi_gpio_get_val(CONFIG_SLE_DETECT_PIN) == GPIO_LEVEL_LOW) {
        node = SERVICE_SLE;
    }

    /* 切换模式 */
    if (node != g_service_type && node != SERVICE_NULL) {
        /* 反初始化现服务 */
        if (g_service_type != SERVICE_NULL) {
            ret = g_funcs[g_service_type]->deinit();
            if (ret) {
                LOG("deinit service %d failed", g_service_type);
            }
            g_service_on = false;
        }

        /* 初始化新服务 */
        ret = g_funcs[node]->init();
        if (ret) {
            LOG("init service %d failed", node);
            return;
        }
        LOG("service node change from %d to %d", g_service_type, node);
        g_service_type = node;
    }
}

int service_ctrl_send_key(uint8_t key)
{
    if (g_service_type <= SERVICE_NULL ||
        g_service_type >= SERVICE_NUM ||
        key >= CONFIG_KEY_NUM) {
        return -1;
    }

    int ret = 0;

    /* 获取键号对应的键值 */
    uint8_t keycode[8] = {0};

    /* 发送键值 */
    keycode[2] = 0x04 + key;
    ret = g_funcs[g_service_type]->send(keycode, 8);
    if (ret < 0) {
        LOG("send key failed, err: %d", ret);
    } else {
        LOG("send key %d", key);
        LOG_BUF("keycode", keycode, 8);
    }

    keycode[2] = 0x00;
    ret = g_funcs[g_service_type]->send(keycode, 8);

    return ret;
}
