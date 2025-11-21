#ifndef BLE_UUID_SERVER_H
#define BLE_UUID_SERVER_H

#include <stdbool.h>
#include <stdint.h>

#include "bts_def.h"

#include "service_ctrl_defs.h"


/* Service UUID */
#define BLE_UUID_UUID_SERVER_SERVICE                 0xABCD
/* Characteristic UUID */
#define BLE_UUID_UUID_SERVER_REPORT                  0xCDEF
/* Client Characteristic Configuration UUID */
#define BLE_UUID_CLIENT_CHARACTERISTIC_CONFIGURATION 0x2902
/* Server ID */
#define BLE_UUID_SERVER_ID 1

/* Characteristic Property */
#define UUID_SERVER_PROPERTIES   (GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY)


int ble_keyboard_server_init(void);
int ble_keyboard_server_deinit(void);
bool ble_connected(void);
int ble_keyboard_send_key(const void* data, uint32_t len);
service_ctrl_func_t* ble_keyboard_get_func(void);

#endif
