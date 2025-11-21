#include "hid_report_desc.h"
#include "hid_report_defs.h"


static uint8_t hid_report_desc[] = {
    /* 标准键盘 */
    0x05, 0x01,        /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x06,        /* USAGE (Keyboard) */
    0xa1, 0x01,        /* COLLECTION (Application) */

    0x85, KEYBOARD_REPORT_ID,        /* Report Id (1) */

    0x05, 0x07,        /* USAGE_PAGE (Keyboard/Keypad) */
    0x19, 0xe0,        /* USAGE_MINIMUM (Keyboard LeftControl) */
    0x29, 0xe7,        /* USAGE_MAXIMUM (Keyboard Right GUI) */
    0x15, 0x00,        /* LOGICAL_MINIMUM (0) */
    0x25, 0x01,        /* LOGICAL_MAXIMUM (1) */
    0x95, 0x08,        /* REPORT_COUNT (8) */
    0x75, 0x01,        /* REPORT_SIZE (1) */
    0x81, 0x02,        /* INPUT (Data,Var,Abs) */

    0x95, 0x01,        /* REPORT_COUNT (1) */
    0x75, 0x08,        /* REPORT_SIZE (8) */
    0x81, 0x03,        /* INPUT (Cnst,Var,Abs) */

    0x95, 0x06,        /* REPORT_COUNT (6) */
    0x75, 0x08,        /* REPORT_SIZE (8) */
    0x15, 0x00,        /* LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,  /* LOGICAL_MAXIMUM (255) */
    0x05, 0x07,        /* USAGE_PAGE (Keyboard/Keypad) */
    0x19, 0x00,        /* USAGE_MINIMUM (Reserved (no event indicated)) */
    0x29, 0x65,        /* USAGE_MAXIMUM (Keyboard Application) */
    0x81, 0x00,        /* INPUT (Data,Ary,Abs) */

    0xc0,              /* END_COLLECTION */

    /* 用户配置 */
    0x06, 0x00, 0xff,  // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,        // USAGE (Vendor Usage 1)
    0xa1, 0x01,        // COLLECTION (Application)

    0x85, CONFIG_REPORT_ID,        //   REPORT_ID (2)

    0x75, 0x08,        //   REPORT_SIZE (8)
    0x95, 0x09,        //   REPORT_COUNT (9)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,  //   LOGICAL_MAXIMUM (255)
    0x09, 0x00,        //   USAGE (Undefined)
    0x91, 0x00,        //   OUTPUT (Data,Ary,Abs)

    0xc0               // END_COLLECTION
};

uint8_t* g_hid_report_desc = hid_report_desc;
uint16_t g_hid_report_desc_size = sizeof(hid_report_desc);
