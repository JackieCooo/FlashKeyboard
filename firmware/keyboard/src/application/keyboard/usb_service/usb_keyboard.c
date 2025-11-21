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


static void usb_keyboard_output_callback(uint8_t* data, uint32_t len)
{
    LOG_BUF("output data", data, len);

    if (g_cb) {
        g_cb((const void*) data, (size_t) len);
    }
}

int usb_keyboard_init(void)
{
    /* 厂商名 */
    const char manufacturer[] = USB_INIT_APP_MANUFACTURER;
    struct device_string str_manufacturer = {
        .str = manufacturer,
        .len = sizeof(manufacturer)
    };

    /* 产品名 */
    const char product[] = USB_INIT_APP_PRODUCT;
    struct device_string str_product = {
        .str = product,
        .len = sizeof(product)
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
    g_hid_handle = hid_add_report_descriptor(g_hid_report_desc, g_hid_report_desc_size, 0);
    LOG("USB HID handle: %d", g_hid_handle);

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

    /* 注册接收回调 */
    if (!fhid_register_output_callback(g_hid_handle, usb_keyboard_output_callback)) {
        LOG("set HID output callback failed");
        return -1;
    }

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
