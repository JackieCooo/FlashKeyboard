#include <stdbool.h>
#include <string.h>

#include "gadget/f_hid.h"
#include "implementation/usb_init.h"
#include "hid_report_desc.h"
#include "usb_keyboard.h"

#define LOG_TAG "usb_hid"
#include "debug.h"

#define USB_INIT_APP_MANUFACTURER  { 'H', 0, 'i', 0, 's', 0, 'i', 0, 'l', 0, 'i', 0, 'c', 0, 'o', 0, 'n', 0 }
#define USB_INIT_APP_PRODUCT  { 'F', 0, 'l', 0, 'a', 0, 's', 0, 'h', 0, ' ', 0, 'K', 0, 'e', 0, 'y', 0, 'b', 0, 'o', 0, 'a', 0, 'r', 0, 'd', 0 }
#define USB_INIT_APP_SERIAL   { '2', 0, '0', 0, '2', 0, '0', 0, '0', 0, '6', 0, '2', 0, '4', 0 }

static bool g_usb_inited = false;
static int g_hid_handle = -1;

static service_ctrl_func_t g_func = {
    .init = usb_keyboard_init,
    .deinit = usb_keyboard_deinit,
    .connected = usb_keyboard_connected,
    .send = usb_keyboard_send_input
};

int usb_keyboard_init(void)
{
    if (g_usb_inited) {
        return 0;
    }

    const char manufacturer[] = USB_INIT_APP_MANUFACTURER;
    struct device_string str_manufacturer = {
        .str = manufacturer,
        .len = sizeof(manufacturer)
    };

    const char product[] = USB_INIT_APP_PRODUCT;
    struct device_string str_product = {
        .str = product,
        .len = sizeof(product)
    };

    const char serial[] = USB_INIT_APP_SERIAL;
    struct device_string str_serial_number = {
        .str = serial,
        .len = sizeof(serial)
    };

    struct device_id dev_id = {
        .vendor_id = 0x1111,
        .product_id = 0x0009,
        .release_num = 0x0800
    };

    /* 添加HID报告描述符 */
    g_hid_handle = hid_add_report_descriptor(g_hid_report_desc, g_hid_report_desc_size, 0);
    LOG("hid handle: %d", g_hid_handle);

    /* 设置设备信息 */
    if (usbd_set_device_info(DEV_HID, &str_manufacturer, &str_product, &str_serial_number, dev_id) != 0) {
        LOG("set device info failed");
        return -1;
    }

    /* 初始化USB */
    if (usb_init(DEVICE, DEV_HID) != 0) {
        LOG("init failed");
        return -1;
    }

    g_usb_inited = true;

    return 0;
}

int usb_keyboard_deinit(void)
{
    int ret = 0;

    if (g_usb_inited) {
        ret = usb_deinit();
        g_usb_inited = false;
    }

    return ret;
}

bool usb_keyboard_connected(void)
{
    return g_usb_inited;
}

int usb_keyboard_send_input(const void* data, uint32_t len)
{
    int ret = fhid_send_data((uint8_t) g_hid_handle, (const char*) data, len);
    if (ret < 0) {
        LOG("send input data falied, ret: 0x%x", ret);
    }

    return ret;
}

service_ctrl_func_t* usb_keyboard_get_func(void)
{
    return &g_func;
}
