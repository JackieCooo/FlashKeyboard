#ifndef __HID_REPORT_DEFS_H__
#define __HID_REPORT_DEFS_H__

#include <stdint.h>


#define KEYBOARD_REPORT_ID 0x01
#define CONFIG_REPORT_ID 0x02


typedef enum {
    CMD_SET_KEY,
    CMD_SET_ENC,
} config_cmd_enum;

#pragma pack(1)

typedef struct {
    uint8_t left_ctrl : 1;
    uint8_t left_shift : 1;
    uint8_t left_alt : 1;
    uint8_t left_gui : 1;
    uint8_t right_ctrl : 1;
    uint8_t right_shift : 1;
    uint8_t right_alt : 1;
    uint8_t right_gui : 1;
} keyboard_modifier_t;

typedef struct {
    keyboard_modifier_t modifier;
    uint8_t reserved;
    uint8_t key[6];
} keyboard_report_t;

typedef struct {
    uint8_t cmd;
    union {
        keyboard_report_t key;
    } data;
} config_report_t;

typedef struct {
    uint8_t id;
    union {
        keyboard_report_t keyboard;
        config_report_t config;
    } report;
} hid_report_t;

#pragma pack()

#define HID_KEYBOARD_REPORT_LEN (sizeof(keyboard_report_t) + 1)
#define HID_CONFIG_REPORT_LEN (sizeof(config_report_t) + 1)

#endif
