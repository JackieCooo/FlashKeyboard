#ifndef __USB_KEYBOARD_H__
#define __USB_KEYBOARD_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>
#include <stdint.h>

#include "service_ctrl_defs.h"


int usb_keyboard_init(void);
int usb_keyboard_deinit(void);
bool usb_keyboard_connected(void);
int usb_keyboard_send_input(const void* data, uint32_t len);
void usb_keyboard_set_receive_callback(service_ctrl_receive_cb_t cb);

service_ctrl_func_t* usb_keyboard_get_func(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
