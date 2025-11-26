#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "gadget/f_hid.h"
#include "implementation/usb_init.h"
#include "hid_report_defs.h"
#include "usb_keyboard.h"

#define LOG_TAG "usb_hid"
#include "debug.h"

#define USB_INIT_APP_SERIAL  { '2', 0, '0', 0, '2', 0, '0', 0, '0', 0, '6', 0, '2', 0, '4', 0 }
#define USB_HID_REPORT_DESC { \
    0x05, 0x01,                /* USAGE_PAGE (Generic Desktop) */\
    0x09, 0x06,                /* USAGE (Keyboard) */\
    0xa1, 0x01,                /* COLLECTION (Application) */\
\
    0x85, KEYBOARD_REPORT_ID,  /* Report Id (1) */\
\
    0x05, 0x07,                /* USAGE_PAGE (Keyboard/Keypad) */\
    0x19, 0xe0,                /* USAGE_MINIMUM (Keyboard LeftControl) */\
    0x29, 0xe7,                /* USAGE_MAXIMUM (Keyboard Right GUI) */\
    0x15, 0x00,                /* LOGICAL_MINIMUM (0) */\
    0x25, 0x01,                /* LOGICAL_MAXIMUM (1) */\
    0x95, 0x08,                /* REPORT_COUNT (8) */\
    0x75, 0x01,                /* REPORT_SIZE (1) */\
    0x81, 0x02,                /* INPUT (Data,Var,Abs) */\
\
    0x95, 0x01,                /* REPORT_COUNT (1) */\
    0x75, 0x08,                /* REPORT_SIZE (8) */\
    0x81, 0x03,                /* INPUT (Cnst,Var,Abs) */\
\
    0x95, 0x06,                /* REPORT_COUNT (6) */\
    0x75, 0x08,                /* REPORT_SIZE (8) */\
    0x15, 0x00,                /* LOGICAL_MINIMUM (0) */\
    0x26, 0xFF, 0x00,          /* LOGICAL_MAXIMUM (255) */\
    0x05, 0x07,                /* USAGE_PAGE (Keyboard/Keypad) */\
    0x19, 0x00,                /* USAGE_MINIMUM (Reserved (no event indicated)) */\
    0x29, 0x65,                /* USAGE_MAXIMUM (Keyboard Application) */\
    0x81, 0x00,                /* INPUT (Data,Ary,Abs) */\
\
    0xc0,                      /* END_COLLECTION */\
\
    0x06, 0x00, 0xff,          /* USAGE_PAGE (Vendor Defined Page 1)*/\
    0x09, 0x01,                /* USAGE (Vendor Usage 1)*/\
    0xa1, 0x01,                /* COLLECTION (Application)*/\
\
    0x85, CONFIG_REPORT_ID,    /* REPORT_ID (2)*/\
\
    0x75, 0x08,                /* REPORT_SIZE (8)*/\
    0x95, 0x09,                /* REPORT_COUNT (9)*/\
    0x15, 0x00,                /* LOGICAL_MINIMUM (0)*/\
    0x26, 0xff, 0x00,          /* LOGICAL_MAXIMUM (255)*/\
    0x09, 0x00,                /* USAGE (Undefined)*/\
    0x91, 0x00,                /* OUTPUT (Data,Ary,Abs)*/\
\
    0xc0                       /* END_COLLECTION */\
} \

static bool g_usb_connected = false;
static int g_hid_handle = -1;

static service_ctrl_receive_cb_t g_cb = NULL;
static service_ctrl_func_t g_func = {
    .init = usb_keyboard_init,
    .deinit = usb_keyboard_deinit,
    .connected = usb_keyboard_connected,
    .send = usb_keyboard_send_input,
    .set_receive_callback = usb_keyboard_set_receive_callback
};


static char* str_to_unicode(const char* str, size_t* len)
{
    size_t str_len = strlen(str);
    char* unicode = (char*) malloc(str_len * 2);

    char* p = unicode;
    for (size_t i = 0; i < str_len; i++, p += 2) {
        *p = *(str + i);
        *(p + 1) = 0;
    }

    if (len) {
        *len = str_len * 2;
    }

    return unicode;
}

static void usb_keyboard_output_callback(uint8_t* data, uint32_t len)
{
    LOG_BUF("output data", data, len);

    if (g_cb) {
        g_cb((const void*) data, (size_t) len);
    }
}

int usb_keyboard_init(void)
{
    size_t len = 0;
    char* name = str_to_unicode(CONFIG_KEYBOARD_NAME, &len);

    /* 厂商名 */
    struct device_string str_manufacturer = {
        .str = name,
        .len = len
    };

    /* 产品名 */
    struct device_string str_product = {
        .str = name,
        .len = len
    };

    /* 序列号 */
    const char serial[] = USB_INIT_APP_SERIAL;
    struct device_string str_serial_number = {
        .str = serial,
        .len = sizeof(serial)
    };

    /* UUID */
    struct device_id dev_id = {
        .vendor_id = 0x12d1,
        .product_id = 0x0009,
        .release_num = 0x0001
    };

    /* 添加HID报告描述符 */
    uint8_t hid_report_desc[] = USB_HID_REPORT_DESC;
    g_hid_handle = hid_add_report_descriptor(hid_report_desc, sizeof(hid_report_desc), 0);
    LOG("USB HID handle: %d", g_hid_handle);

    /* 设置设备信息 */
    if (usbd_set_device_info(DEV_HID, &str_manufacturer, &str_product, &str_serial_number, dev_id) != 0) {
        LOG("set device info failed");
        free(name);
        return -1;
    }

    /* 初始化USB */
    if (usb_init(DEVICE, DEV_HID) != 0) {
        LOG("init failed");
        free(name);
        return -1;
    }

    /* 注册接收回调 */
    if (!fhid_register_output_callback(g_hid_handle, usb_keyboard_output_callback)) {
        LOG("set HID output callback failed");
        free(name);
        return -1;
    }

    free(name);
    return 0;
}

int usb_keyboard_deinit(void)
{
    return (int) usb_deinit();
}

bool usb_keyboard_connected(void)
{
    return g_usb_connected;
}

int usb_keyboard_send_input(const void* data, uint32_t len)
{
    int ret = fhid_send_data((uint8_t) g_hid_handle, (const char*) data, len);
    if (ret < 0) {
        LOG("send input data falied, ret: 0x%x", ret);
    }

    return ret;
}

void usb_keyboard_set_receive_callback(service_ctrl_receive_cb_t cb)
{
    g_cb = cb;
}

service_ctrl_func_t* usb_keyboard_get_func(void)
{
    return &g_func;
}
