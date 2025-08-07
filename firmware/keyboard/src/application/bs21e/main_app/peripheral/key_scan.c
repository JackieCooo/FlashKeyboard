#include "key_scan.h"

#include <stdint.h>

#include "gpio.h"
#include "pinctrl.h"
#include "soc_osal.h"
#include "ble_hid_keyboard_server.h"
#include "usb_keyboard.h"

#define LOG_TAG "key_scan"
#include "debug.h"


#define BUTTON_PIN 15


static uint8_t keycode[8] = {0};

void key_scan(void)
{
    if (uapi_gpio_get_val(BUTTON_PIN) == GPIO_LEVEL_LOW) {
        while (uapi_gpio_get_val(BUTTON_PIN) == GPIO_LEVEL_LOW) {
            osal_msleep(10);
        }

        // keycode[1] = 0x04;
        // ble_hiddev_keyboard_server_send_input_report_by_uuid((uint8_t *)&keycode, 7);
        // keycode[1] = 0x00;
        // ble_hiddev_keyboard_server_send_input_report_by_uuid((uint8_t *)&keycode, 7);

        keycode[2] = 0x04;
        usb_keyboard_send_input(keycode, sizeof(keycode));
        keycode[2] = 0x00;
        usb_keyboard_send_input(keycode, sizeof(keycode));

        LOG("send keycode");
    }
}

void key_scan_init(void)
{
    uapi_pin_set_mode(BUTTON_PIN, HAL_PIO_FUNC_GPIO);
    uapi_pin_set_pull(BUTTON_PIN, PIN_PULL_UP);
    uapi_gpio_set_dir(BUTTON_PIN, GPIO_DIRECTION_INPUT);
}
