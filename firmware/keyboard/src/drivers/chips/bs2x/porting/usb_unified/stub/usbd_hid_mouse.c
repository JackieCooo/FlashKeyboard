/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: USB HID Mouse driver.
 *
 * Create:  2022-04-22
 */
#include "common_def.h"
#include "usbd_hid_mouse.h"

bool usb_hid_mouse_register_report_desc(uint8_t *report_desc, uint16_t lenth)
{
    unused(report_desc);
    unused(lenth);
    return true;
}
