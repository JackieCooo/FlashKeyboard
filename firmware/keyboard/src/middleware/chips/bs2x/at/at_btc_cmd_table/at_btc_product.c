/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: at plt cmd func \n
 *
 */

#include "debug_print.h"
#include "bts_factory.h"
#include "at_config.h"
#include "at_product.h"
#include "nv.h"
#include "key_id.h"
#include "chip_io.h"
#include "common.h"
#include "at_bt_cmd_table.h"
#include "clock_calibration.h"
#include "sle_device_manager.h"
#include "sle_factory_manager.h"
#include "sle_device_discovery.h"
#include "bts_device_manager.h"
#if BT_TSENSOR_ENABLE
#include "bt_tsensor.h"
#endif
#include "application_version.h"

#define BT_CUSTOMIZE_NV_VALID 11
#define BT_NV_START_OFFSET 2
#define BT_NV_CUSTOMIZE_PARAM 2
#define BT_NV_WRITE_MAX_LENGTH 1024

#define SET_NV_MAC_PARAM_CNT 2
#define NV_ID_BTH_PRODUCT_INFORMATION_CONFIG 0x9
#define NV_ID_SLE_PRODUCT_DATA_CONFIG 0xA
#define BLE_MAC_ADDR_NV 0
#define SLE_MAC_ADDR_NV 1
#define MAC_LEN 6

#ifndef CONFIG_BT_SLE_ONLY
extern int ble_at_cmd_factory_register_callbacks(void);
#endif
typedef enum {
    EN_BT_TX_SIG_LO         = 0,
} bt_tx_sig_enum;
extern void bt_adpll_start_tx(uint8_t freq, bt_tx_sig_enum mode); // 单音模式默认mode = 0
extern void bt_adpll_stop_tx(void);
#ifndef CONFIG_BT_BLE_ONLY
extern uint16_t g_sle_cfo_rpt;
extern int8_t g_sle_cfo_rssi_limit; // cfo上报rssi门限，默认-127dBm
extern int sle_at_cmd_factory_register_cbks(void);
#endif

const int32_t xo_ctrim_cali_increase_switch[] = {
    0, 1
};

const at_para_parse_syntax_t xo_ctrim_cali_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {2, xo_ctrim_cali_increase_switch},
        .offset = offsetof(xo_ctrim_cali_param_args_t, increase)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 255,
        .offset = offsetof(xo_ctrim_cali_param_args_t, step_num)
    },
};

#ifndef CONFIG_BT_SLE_ONLY
const int32_t ble_pk_payload_type_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
const int32_t ble_rf_tx_phy_values[] = {
    1, 2, 3, 4
};
const at_para_parse_syntax_t ble_rf_tx_write_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 39,
        .offset = offsetof(ble_rf_tx_param_args_t, tx_freq)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 37,
        .entry.int_range.max_val = 255,
        .offset = offsetof(ble_rf_tx_param_args_t, test_data_len)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {8, ble_pk_payload_type_values},
        .offset = offsetof(ble_rf_tx_param_args_t, pk_payload_type)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {4, ble_rf_tx_phy_values},
        .offset = offsetof(ble_rf_tx_param_args_t, tx_phy)
    },
};

const int32_t ble_rf_rx_phy_values[] = {
    1, 2, 3, 4
};
const int32_t ble_modulation_index_values[] = {
    0, 1
};
const at_para_parse_syntax_t ble_rf_rx_write_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 39,
        .offset = offsetof(ble_rf_rx_param_args_t, rx_freq)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {4, ble_rf_rx_phy_values},
        .offset = offsetof(ble_rf_rx_param_args_t, rx_phy)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {2, ble_modulation_index_values},
        .offset = offsetof(ble_rf_rx_param_args_t, modulation_index)
    },
};
#endif // ifndef CONFIG_BT_SLE_ONLY

const int32_t rf_single_tone_mode[] = {
    0, 255
};
const at_para_parse_syntax_t rf_single_tone_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 79,
        .offset = offsetof(rf_single_tone_param_args_t, rf_freq)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {2, rf_single_tone_mode},
        .offset = offsetof(rf_single_tone_param_args_t, rf_mode)
    },
};

#ifndef CONFIG_BT_BLE_ONLY
const int32_t sle_rf_tx_power_values[] = {
    0, 1, 2, 3, 4, 5
};
const int32_t sle_rf_tx_payload_type_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
const int32_t sle_rf_tx_phy_values[] = {
    0, 1, 4
};
const int32_t sle_rf_tx_format_values[] = {
    0, 2
};
const int32_t sle_rf_tx_rate_values[] = {
    0, 2, 3
};
const int32_t sle_rf_tx_pilot_ratio_values[] = {
    0, 1, 2, 3
};
const int32_t sle_rf_tx_polar_r_values[] = {
    0, 1, 2, 3
};
const at_para_parse_syntax_t sle_rf_tx_write_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 255,
        .offset = offsetof(sle_rf_tx_param_args_t, tx_freq)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {6, sle_rf_tx_power_values},
        .offset = offsetof(sle_rf_tx_param_args_t, tx_power)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 37,
        .entry.int_range.max_val = 260,
        .offset = offsetof(sle_rf_tx_param_args_t, test_data_len)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {8, sle_rf_tx_payload_type_values},
        .offset = offsetof(sle_rf_tx_param_args_t, pk_payload_type)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {3, sle_rf_tx_phy_values},
        .offset = offsetof(sle_rf_tx_param_args_t, tx_phy)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {2, sle_rf_tx_format_values},
        .offset = offsetof(sle_rf_tx_param_args_t, tx_format)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {3, sle_rf_tx_rate_values},
        .offset = offsetof(sle_rf_tx_param_args_t, tx_rate)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {4, sle_rf_tx_pilot_ratio_values},
        .offset = offsetof(sle_rf_tx_param_args_t, tx_pilot_ratio)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {4, sle_rf_tx_polar_r_values},
        .offset = offsetof(sle_rf_tx_param_args_t, tx_polar_r)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 5,
        .entry.int_range.max_val = 255,
        .offset = offsetof(sle_rf_tx_param_args_t, tx_interval)
    },
};

const int32_t sle_rf_rx_phy_values[] = {
    0, 1, 4
};
const int32_t sle_rf_rx_format_values[] = {
    0, 2
};
const int32_t sle_rf_rx_pilot_ratio_values[] = {
    0, 1, 2, 3
};
const at_para_parse_syntax_t sle_rf_rx_write_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 79,
        .offset = offsetof(sle_rf_rx_param_args_t, rx_freq)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {3, sle_rf_rx_phy_values},
        .offset = offsetof(sle_rf_rx_param_args_t, rx_phy)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {2, sle_rf_rx_format_values},
        .offset = offsetof(sle_rf_rx_param_args_t, rx_format)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {4, sle_rf_rx_pilot_ratio_values},
        .offset = offsetof(sle_rf_rx_param_args_t, rx_pilot_ratio)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 5,
        .entry.int_range.max_val = 255,
        .offset = offsetof(sle_rf_rx_param_args_t, rx_interval)
    },
};

const at_para_parse_syntax_t sle_cfo_rpt_rssi_limit_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = -127,
        .entry.int_range.max_val = 20,
        .offset = offsetof(rf_sle_cfo_rpt_rssi_limit_t, rssi_limit)
    },
};
#endif

const at_para_parse_syntax_t bt_write_customize_nv_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET, // 支持最大偏移为当前有效定制化的最大偏移
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset1)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value1)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset2)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value2)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset3)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value3)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset4)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value4)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset5)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value5)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset6)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value6)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset7)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value7)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset8)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value8)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset9)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value9)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset10)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value10)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset11)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value11)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset12)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value12)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset13)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value13)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset14)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value14)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE | AT_SYNTAX_ATTR_OPTIONAL,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = BT_CUSTOMIZE_NV_VALID - BT_NV_START_OFFSET,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_offset15)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL,
        .offset = offsetof(bt_write_customize_nv_param_args_t, customize_value15)
    },
};

const at_para_parse_syntax_t read_nv_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_AT_MIN_VALUE | AT_SYNTAX_ATTR_AT_MAX_VALUE,
        .entry.int_range.min_val = 0,
        .entry.int_range.max_val = 65535,
        .offset = offsetof(nv_read_param_args_t, key_id)
    },
};
const int32_t set_nv_mac_type_values[] = {
    0, 1
};
const at_para_parse_syntax_t set_nv_mac_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_STRING,
        .attribute = AT_SYNTAX_ATTR_OPTIONAL | AT_SYNTAX_ATTR_MAX_LENGTH,
        .entry.string.max_length = 17,
        .offset = offsetof(nv_mac_param_args_t, mac_addr)
    },
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {(sizeof(set_nv_mac_type_values) / sizeof(int32_t)), set_nv_mac_type_values},
        .offset = offsetof(nv_mac_param_args_t, mac_type)
    },
};

const int32_t write_fem_flag[] = {
    0, 1
};
const at_para_parse_syntax_t write_fem_flag_param_syntax[] = {
    {
        .type = AT_SYNTAX_TYPE_INT,
        .last = true,
        .attribute = AT_SYNTAX_ATTR_LIST_VALUE,
        .entry.int_list = {2, write_fem_flag},
        .offset = offsetof(fem_switch_param_args_t, fem_switch)
    },
};

#ifdef XO_32M_CALI
/* XO CTRIM CALIBRATION */
at_ret_t xo_ctrim_cali_cmd(const xo_ctrim_cali_param_args_t *args)
{
    bool step_increase = args->increase == 0x1 ? true : false;
    uint8_t xo_ctrim_step = (uint8_t)args->step_num;
    uint8_t xo_ctrim_value = 0;
    // 通过命令发送增加或减少左右电容的step，并配置寄存器
    calibration_xo_core_ctrim_algorithm(step_increase, xo_ctrim_step);
    // 回读寄存器确认值成功写入
    calibration_get_xo_core_ctrim_reg(&xo_ctrim_value);
    osal_printk("xo_ctrim_reg_val:0x%x\n", xo_ctrim_value);
    return AT_RET_OK;
}

/* XO CTRIM CALIBRATION WRITE EFUSE */
at_ret_t xo_ctrim_cali_write_efuse_cmd(void)
{
    uint8_t xo_ctrim_value = 0;
    // 读当前寄存器内值并写入efuse
    calibration_get_xo_core_ctrim_reg(&xo_ctrim_value);
    calibration_save_xo_core_ctrim(xo_ctrim_value);
    // 回读efuse检查是否写入，efuse只能写一次
    calibration_read_xo_core_ctrim(&xo_ctrim_value);
    osal_printk("efuse_val:0x%x\n", xo_ctrim_value);
    return AT_RET_OK;
}

/* XO CTRIM CALIBRATION READ EFUSE */
at_ret_t xo_ctrim_cali_read_efuse_cmd(void)
{
    uint8_t xo_ctrim_value = 0;
    // 回读efuse检查是否写入，efuse只能写一次
    calibration_read_xo_core_ctrim(&xo_ctrim_value);
    osal_printk("efuse_val:0x%x\n", xo_ctrim_value);
    return AT_RET_OK;
}

/* XO CTRIM CALIBRATION WRITE FLASH */
at_ret_t xo_ctrim_cali_write_flash_cmd(void)
{
    uint8_t xo_ctrim_value = 0;
    // 读当前寄存器内值并写入efuse
    calibration_get_xo_core_ctrim_reg(&xo_ctrim_value);
    calibration_xo_core_ctrim_save_flash(xo_ctrim_value);
    // 回读efuse检查是否写入，efuse只能写一次
    calibration_xo_core_ctrim_read_flash(&xo_ctrim_value);
    osal_printk("flash_val:0x%x\n", xo_ctrim_value);
    return AT_RET_OK;
}

/* XO CTRIM CALIBRATION READ FLASH */
at_ret_t xo_ctrim_cali_read_flash_cmd(void)
{
    uint8_t xo_ctrim_value = 0;
    // 回读efuse检查是否写入，efuse只能写一次
    calibration_xo_core_ctrim_read_flash(&xo_ctrim_value);
    osal_printk("flash_val:0x%x\n", xo_ctrim_value);
    return AT_RET_OK;
}

/* XO CTRIM CALIBRATION GET REG VAL */
at_ret_t xo_ctrim_get_reg_val_cmd(void)
{
    uint8_t xo_ctrim_value = 0;
    // 读取当前寄存器配置
    calibration_get_xo_core_ctrim_reg(&xo_ctrim_value);
    osal_printk("reg_val:0x%x\n", xo_ctrim_value);
    return AT_RET_OK;
}
#endif

#ifndef CONFIG_BT_SLE_ONLY
/* BLE长发 */
at_ret_t bt_at_ble_rf_tx_cmd(const ble_rf_tx_param_args_t *args)
{
    ble_rf_tx_start_t rf_tx_start = { 0 };
    rf_tx_start.tx_freq = (uint8_t)args->tx_freq;
    rf_tx_start.test_data_len = (uint8_t)args->test_data_len;
    rf_tx_start.payload_type = (uint8_t)args->pk_payload_type;
    rf_tx_start.tx_phy = (uint8_t)args->tx_phy;
    return ble_factory_rf_tx_start(&rf_tx_start);
}

/* BLE长收 */
at_ret_t bt_at_ble_rf_rx_cmd(const ble_rf_rx_param_args_t *args)
{
    ble_rf_rx_start_t rf_rx_start = { 0 };
    rf_rx_start.rx_freq = (uint8_t)args->rx_freq;
    rf_rx_start.rx_phy = (uint8_t)args->rx_phy;
    rf_rx_start.modulation_index = (uint8_t)args->modulation_index;
    return ble_factory_rf_rx_start(&rf_rx_start);
}

/* BLE TRX END */
at_ret_t bt_at_ble_rf_trxend_cmd(void)
{
    return ble_factory_rf_trx_end();
}

/* BLE RESET */
at_ret_t bt_at_ble_reset_cmd(void)
{
    return ble_factory_rf_reset();
}


/* BLE REGISTER CALLBACK */
at_ret_t bt_at_ble_register_callback_cmd(void)
{
    ble_at_cmd_factory_register_callbacks();
    return AT_RET_OK;
}
/* BT CALI NV */
at_ret_t bt_at_rf_cali_nv_cmd(void)
{
    ble_factory_rf_cali_nv();
    return AT_RET_OK;
}

/* BT SINGLE TONE */
at_ret_t bt_at_enable_ble_cmd(void)
{
    enable_ble();
    return AT_RET_OK;
}

at_ret_t bt_at_read_cali_nv_cmd(void)
{
    // TODO
    return AT_RET_OK;
}

at_ret_t bt_at_write_cali_nv_cmd(void)
{
    // TODO
    return AT_RET_OK;
}
#endif // CONFIG_BT_SLE_ONLY

/* BT SINGLE TONE */
at_ret_t bt_at_rf_single_tone_cmd(const rf_single_tone_param_args_t *args)
{
    ble_rf_single_tone_t param = { 0 };
    param.rf_freq = (uint8_t)args->rf_freq;
    param.rf_mode = (uint8_t)args->rf_mode;
    if (param.rf_mode == 0x0) {
        bt_adpll_start_tx(param.rf_freq, EN_BT_TX_SIG_LO);
    } else {
        bt_adpll_stop_tx();
    }

    return AT_RET_OK;
}
bool at_customize_features_set(uint8_t *features, uint8_t offset)
{
    // 记录定制化特性的开关bit位
    const uint8_t g_features_map[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9 // 0,1,2,3,4,5,6,7,8,9:开关bit相同表示该特性字段占多个字节
    };
    if (offset > (sizeof(g_features_map) - 0x1) + 0x2) {
        osal_printk("[ERROR]Offset out of range!\r\n");
        return false;
    }
    if (g_features_map[offset - 0x2] < 0x8) { // 0-7bit表示8个features开关
        features[0] = (features[0] | (0x1 << g_features_map[offset - 0x2]));     // 当前偏移即第1字节的开关
    } else if (g_features_map[offset - 0x2] >= 0x8 && g_features_map[offset - 0x2] <= 0xF) { // 8-15bit表示8个features开关
        features[0x1] = (features[0x1] | (0x1 << (g_features_map[offset - 0x2] - 0x8))); // 当前偏移-8即第2字节的开关
    }
    return true;
}

static uint8_t map_count_bit(uint32_t value)
{
    uint8_t count = 0;
    uint32_t val = value;
    while (val != 0) {
        val &= val - 0x1;
        count++;
    }
    return count;
}

at_ret_t bt_at_read_customize_nv_cmd(void)
{
    uint16_t key = 0xE;
    uint16_t key_len = (uint16_t)sizeof(bth_ble_nv_reserved_struct_t); // 写NV ipc payload的长度
    uint16_t real_len = 0;
    uint8_t *read_value = osal_vmalloc(key_len);
    if (uapi_nv_read(key, key_len, &real_len, read_value) != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_vfree(read_value);
        read_value = NULL;
        return AT_RET_PROGRESS_BLOCK;
    }
    for (int32_t i = BT_NV_START_OFFSET; i <= BT_CUSTOMIZE_NV_VALID; i++) {
        osal_printk("[NV]:read_value:%x \n", *(read_value + i));
    }
    if (read_value != NULL) {
        osal_vfree(read_value);
        read_value = NULL;
    }
    return AT_RET_OK;
}
/* BT LOAD CUSTOMIZE NV */
at_ret_t bt_at_customize_nv_cmd(const bt_write_customize_nv_param_args_t *args)
{
    uint16_t key = 0xE;
    uint16_t key_len = (uint16_t)sizeof(bth_ble_nv_reserved_struct_t); // 写NV ipc payload的长度
    uint16_t real_len = 0;
    uint8_t *read_value = osal_vmalloc(key_len);
    uint8_t map_bit = map_count_bit(args->para_map);
    int32_t *argv = (int32_t *)args;
    if (uapi_nv_read(key, key_len, &real_len, read_value) != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_vfree(read_value);
        read_value = NULL;
        return AT_RET_PROGRESS_BLOCK;
    }
    for (uint8_t i = 0x1; i <= map_bit; i += BT_NV_CUSTOMIZE_PARAM) {
        uint8_t offset = (uint8_t)argv[i] + BT_NV_START_OFFSET;
        uint8_t value = (uint8_t)argv[i + 1];
        if (!at_customize_features_set(read_value, offset)) {
            osal_vfree(read_value);
            read_value = NULL;
            return AT_RET_SYNTAX_ERROR;
        }
        read_value[offset] = value;
    }
    errcode_t nv_ret_value = uapi_nv_write(key, read_value, key_len);
    for (int32_t i = BT_NV_START_OFFSET; i <= BT_CUSTOMIZE_NV_VALID; i++) {
        osal_printk("[NV]:write_value:%x \n", *(read_value + i));
    }
    if (nv_ret_value != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_printk("[ERROR]write nv fail! ret:%x \r\n", nv_ret_value);
        osal_vfree(read_value);
        read_value = NULL;
        return AT_RET_PROGRESS_BLOCK;
    }
    if (read_value != NULL) {
        osal_vfree(read_value);
        read_value = NULL;
    }
    return AT_RET_OK;
}

#ifndef CONFIG_BT_BLE_ONLY
/* BT SINGLE TONE */
at_ret_t bt_at_enable_sle_cmd(void)
{
    enable_sle();
    return AT_RET_OK;
}

/* sle长发 */
at_ret_t bt_at_sle_rf_tx_cmd(const sle_rf_tx_param_args_t *args)
{
    sle_rf_tx_start_t rf_tx_start = { 0 };
    rf_tx_start.tx_freq = (uint8_t)args->tx_freq;
    rf_tx_start.tx_power = (uint8_t)args->tx_power;
    rf_tx_start.test_data_len = (uint16_t)args->test_data_len;
    rf_tx_start.pk_payload_type = (uint8_t)args->pk_payload_type;
    rf_tx_start.tx_phy = (uint8_t)args->tx_phy;
    rf_tx_start.tx_format = (uint8_t)args->tx_format;
    rf_tx_start.tx_rate = (uint8_t)args->tx_rate;
    rf_tx_start.tx_pilot_ratio = (uint8_t)args->tx_pilot_ratio;
    rf_tx_start.tx_polar_r = (uint8_t)args->tx_polar_r;
    rf_tx_start.tx_interval = (uint16_t)args->tx_interval;
    sle_rf_tx_start(&rf_tx_start);
    return AT_RET_OK;
}

/* sle长收 */
at_ret_t bt_at_sle_rf_rx_cmd(const sle_rf_rx_param_args_t *args)
{
    sle_rf_rx_start_t rf_rx_start = { 0 };
    rf_rx_start.rx_freq = (uint8_t)args->rx_freq;
    rf_rx_start.rx_phy = (uint8_t)args->rx_phy;
    rf_rx_start.rx_format = (uint8_t)args->rx_format;
    rf_rx_start.rx_pilot_ratio = (uint8_t)args->rx_pilot_ratio;
    rf_rx_start.rx_interval = (uint16_t)args->rx_interval;
    sle_rf_rx_start(&rf_rx_start);
    return AT_RET_OK;
}

/* sle TRX END */
at_ret_t bt_at_sle_rf_trxend_cmd(void)
{
    sle_rf_trx_end();
    return AT_RET_OK;
}

/* sle RESET */
at_ret_t bt_at_sle_reset_cmd(void)
{
    sle_rf_reset();
    return AT_RET_OK;
}

/* CFO re[prt] */
at_ret_t bt_at_cfo_rpt_cmd(void)
{
    if ((g_sle_cfo_rpt >> 0xF) == 0x1) {
        osal_printk("[AT]sle CFO report:%d\r\n", g_sle_cfo_rpt & 0xFF);
    } else {
        osal_printk("[AT]rssi lower than limit! last CFO:%d\r\n", g_sle_cfo_rpt & 0xFF);
    }
    return AT_RET_OK;
}

/* CFO re[prt] */
at_ret_t bt_at_cfo_rpt_rssi_limit_cmd(const rf_sle_cfo_rpt_rssi_limit_t *args)
{
    g_sle_cfo_rssi_limit = args->rssi_limit;
    osal_printk("[AT]sle CFO rssi limit:%d\r\n", g_sle_cfo_rssi_limit);
    return AT_RET_OK;
}

/* SLE REGISTER CALLBACK */
at_ret_t bt_at_sle_register_callback_cmd(void)
{
    sle_at_cmd_factory_register_cbks();
    return AT_RET_OK;
}
#endif

/* WRITE FEM ENABLE FLAG */
at_ret_t bt_at_set_fem_enable_flag(const fem_switch_param_args_t *args)
{
    uint16_t key = 0xE;
    uint16_t key_len = (uint16_t)sizeof(bth_ble_nv_reserved_struct_t); // 写NV ipc payload的长度
    uint16_t real_len = 0;
    uint8_t *read_value = osal_vmalloc(key_len);
    if (uapi_nv_read(key, key_len, &real_len, read_value) != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_vfree(read_value);
        read_value = NULL;
        osal_printk("[ERROR]fem_switch: read nv fail.\r\n");
        return AT_RET_PROGRESS_BLOCK;
    }
    if (args->fem_switch == 0x1) {
        read_value[0x7] = 0x1; // fem enable flag set to 1
    } else {
        read_value[0x7] = 0x0; // fem enable flag set to 1
    }
    read_value[0x1] = (read_value[0x1] | (0x1 << 0x2)); // nv被写标志位置1
    errcode_t nv_ret_value = uapi_nv_write(key, read_value, key_len);
    if (nv_ret_value != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_printk("[ERROR]write nv fail. ret:%x \r\n", nv_ret_value);
        osal_vfree(read_value);
        read_value = NULL;
        return AT_RET_PROGRESS_BLOCK;
    }
    if (read_value != NULL) {
        osal_vfree(read_value);
        read_value = NULL;
    }
    return AT_RET_OK;
}

/* DIE ID */
at_ret_t bt_at_read_dieid_cmd(void)
{
    int8_t index;
    osal_printk("[AT]DIE ID:\r\n");
    for (index = 0; index <= 0x20; index += 0x4) {
        osal_printk("%04x ", readw(0x57028800 + index));
    }
    osal_printk("\r\n");
    return AT_RET_OK;
}

#if BT_TSENSOR_ENABLE
/* 获取温度 */
at_ret_t bt_at_get_temp_cmd(void)
{
    int16_t temp = bt_tsensor_temperature_get();
    osal_printk("[AT]chip_temp:%x\r\n", temp);
    return AT_RET_OK;
}
#endif

/* 读取NV */
at_ret_t bt_at_nv_read_cmd(const nv_read_param_args_t *args)
{
    uint16_t nv_value_length = 0;
    uint8_t nv_value[BT_NV_WRITE_MAX_LENGTH] = {0};
    nv_key_attr_t nv_attr = {0};
    errcode_t ret = uapi_nv_read_with_attr((uint16_t)args->key_id, BT_NV_WRITE_MAX_LENGTH, &nv_value_length,
        nv_value, &nv_attr);
    if (ret != ERRCODE_SUCC) {
        osal_printk("[AT]nv_read failed, ret = [0x%x]\r\n", ret);
        return ret;
    }
    if (nv_attr.permanent == true) {
        osal_printk("NV[0x%x] is permanent\r\n", args->key_id);
    }
    if (nv_attr.encrypted == true) {
        osal_printk("NV[0x%x] is encrypted\r\n", args->key_id);
    }
    if (nv_attr.non_upgrade == true) {
        osal_printk("NV[0x%x] is non_upgrade\r\n", args->key_id);
    }
    for (int i = 0; i < nv_value_length; i++) {
        osal_printk("nv_value[%d] = [0x%x]\r\n", i, nv_value[i]);
    }
    return AT_RET_OK;
}

static uint32_t nv_read_mac(uint16_t key, uint8_t *mac_addr, uint16_t key_len, uint16_t addr_len)
{
    uint16_t real_len = 0;
    uint8_t *read_value = osal_vmalloc(key_len);
    errcode_t ret = uapi_nv_read(key, key_len, &real_len, read_value);
    if ((ret != ERRCODE_SUCC) || (real_len < addr_len)) {
        /* ERROR PROCESS */
        osal_vfree(read_value);
        read_value = NULL;
        osal_printk("[ERROR]nv_read_mac: read nv fail. ret:%x \r\n", ret);
        return ERRCODE_FAIL;
    }
    for (uint8_t i = 0; i < addr_len; i++) {
        mac_addr[i] = read_value[i];
    }
    if (read_value != NULL) {
        osal_vfree(read_value);
        read_value = NULL;
    }
    return ERRCODE_SUCC;
}

static uint32_t nv_write_mac(uint16_t key, uint8_t *mac_addr, uint16_t key_len, uint16_t addr_len)
{
    uint16_t real_len = 0;
    uint8_t *read_value = osal_vmalloc(key_len);
    errcode_t ret = uapi_nv_read(key, key_len, &real_len, read_value);
    if ((ret != ERRCODE_SUCC) || (real_len < addr_len)) {
        /* ERROR PROCESS */
        osal_vfree(read_value);
        read_value = NULL;
        osal_printk("[ERROR]nv_write_mac: read nv fail. ret:%x \r\n", ret);
        return ERRCODE_FAIL;
    }
    for (uint8_t i = 0; i < addr_len; i++) {
        read_value[i] = mac_addr[i];
    }
    ret = uapi_nv_write(key, read_value, key_len);
    if (ret != ERRCODE_SUCC) {
        /* ERROR PROCESS */
        osal_printk("[ERROR]write nv fail. ret:%x \r\n", ret);
        osal_vfree(read_value);
        read_value = NULL;
        return ERRCODE_FAIL;
    }
    if (read_value != NULL) {
        osal_vfree(read_value);
        read_value = NULL;
    }
    return ERRCODE_SUCC;
}

static uint32_t bt_at_check_mac_elem(const char elem)
{
    if (elem >= '0' && elem <= '9') {
        return ERRCODE_SUCC;
    } else if (elem >= 'A' && elem <= 'F') {
        return ERRCODE_SUCC;
    } else if (elem >= 'a' && elem <= 'f') {
        return ERRCODE_SUCC;
    } else if (elem == ':') {
        return ERRCODE_SUCC;
    }

    return ERRCODE_FAIL;
}

static uint32_t bt_at_cmd_strtoaddr(const char *param, unsigned char *mac_addr, uint32_t addr_len)
{
    uint32_t cnt;
    char *tmp1 = (char *)param;
    char *tmp2 = NULL;
    char *tmp3 = NULL;

    for (cnt = 0; cnt < 17; cnt++) {    /* 17 */
        if (bt_at_check_mac_elem(param[cnt]) != ERRCODE_SUCC) {
            return ERRCODE_FAIL;
        }
    }

    for (cnt = 0; cnt < (addr_len - 1); cnt++) {
        tmp2 = (char*)strsep(&tmp1, ":");
        if (tmp2 == NULL) {
            return ERRCODE_FAIL;
        }
        mac_addr[cnt] = (unsigned char)strtoul(tmp2, &tmp3, 16); /* 16 */
    }

    if (tmp1 == NULL) {
        return ERRCODE_FAIL;
    }
    mac_addr[cnt] = (unsigned char)strtoul(tmp1, &tmp3, 16); /* 16 */
    return ERRCODE_SUCC;
}

static void bt_at_print_mac_addr(unsigned char *mac_addr, uint32_t addr_len)
{
    for (unsigned int i = 0; i < addr_len - 1; i++) {
        osal_printk("%02x:", mac_addr[i]);
    }
    osal_printk("%02x\r\n", mac_addr[addr_len - 1]);
}

static uint32_t set_mac_addr_with_type(uint32_t mac_type, uint8_t *mac_addr, uint16_t addr_len)
{
    uint32_t ret;
    uint16_t key_len;
    switch (mac_type) {
        case BLE_MAC_ADDR_NV:
            key_len = (uint16_t)sizeof(bth_product_information_config_t);
            ret = nv_write_mac(NV_ID_BTH_PRODUCT_INFORMATION_CONFIG, mac_addr, key_len, addr_len);
            if (ret != ERRCODE_SUCC) {
                osal_printk("[AT]SET NV MAC ERROR, ret : 0x%x\r\n", ret);
                return ret;
            }
            break;
        case SLE_MAC_ADDR_NV:
            key_len = (uint16_t)sizeof(sle_product_data_config_stru_t);
            ret = nv_write_mac(NV_ID_SLE_PRODUCT_DATA_CONFIG, mac_addr, key_len, addr_len);
            if (ret != ERRCODE_SUCC) {
                osal_printk("[AT]SET NV SLE MAC ERROR, ret : 0x%x\r\n", ret);
                return ret;
            }
            break;
        default:
            return ERRCODE_FAIL;
            break;
    }
    return ERRCODE_SUCC;
}

/* 设置mac地址 */
at_ret_t bt_at_set_nv_mac_addr(const nv_mac_param_args_t *args)
{
    uint32_t argc = map_count_bit(args->para_map);
    uint8_t mac_addr[MAC_LEN] = {0};

    if (argc != SET_NV_MAC_PARAM_CNT || strlen((const char *)args->mac_addr) != 17) { /* 17 mac string len */
        return AT_RET_SYNTAX_ERROR;
    }

    if (bt_at_cmd_strtoaddr((const char *)args->mac_addr, mac_addr, MAC_LEN) != ERRCODE_SUCC) {
        osal_printk("[AT]set_device_addr FAIL\r\n");
        return AT_RET_PARSE_PARA_ERROR;
    }

    if (set_mac_addr_with_type(args->mac_type, mac_addr, MAC_LEN) != ERRCODE_SUCC) {
        return AT_RET_CMD_PARA_ERROR;
    }

    return AT_RET_OK;
}

/* 获取mac地址 */
at_ret_t bt_at_get_nv_mac_addr(void)
{
    uint8_t mac_addr[MAC_LEN] = {0};
    uint16_t key_len = (uint16_t)sizeof(bth_product_information_config_t);
    errcode_t ret;

    ret = nv_read_mac(NV_ID_BTH_PRODUCT_INFORMATION_CONFIG, mac_addr, key_len, MAC_LEN);
    if (ret != ERRCODE_SUCC) {
        osal_printk("GET NV MAC ERROR, ret : 0x%x\r\n", ret);
    } else {
        osal_printk("NV BLE MAC: ");
        bt_at_print_mac_addr(mac_addr, MAC_LEN);
    }

    key_len = (uint16_t)sizeof(sle_product_data_config_stru_t);
    ret = nv_read_mac(NV_ID_SLE_PRODUCT_DATA_CONFIG, mac_addr, key_len, MAC_LEN);
    if (ret != ERRCODE_SUCC) {
        osal_printk("GET NV SLE MAC ERROR, ret : 0x%x\r\n", ret);
    } else {
        osal_printk("NV SLE MAC: ");
        bt_at_print_mac_addr(mac_addr, MAC_LEN);
    }

    return AT_RET_OK;
}

/* 获取版本号 */
at_ret_t bt_at_get_version_cmd(void)
{
    osal_printk("[AT]chip version:%s\r\n", APPLICATION_VERSION_STRING);
    return AT_RET_OK;
}

/* 获取电源模式 */
at_ret_t bt_at_get_power_mode_cmd(void)
{
#ifdef CONFIG_POWER_SUPPLY_BY_LDO
    osal_printk("[AT]Power supply by ldo.\r\n");
#else
    osal_printk("[AT]Power supply by buck.\r\n");
#endif
    return AT_RET_OK;
}