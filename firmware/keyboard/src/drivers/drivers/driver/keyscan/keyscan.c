/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2024. All rights reserved.
 *
 * Description: Provides keyscan driver source \n
 *
 * History: \n
 * 2022-09-15, Create file. \n
 */
#include <stdbool.h>
#include "securec.h"
#include "soc_osal.h"
#include "securec.h"
#include "errcode.h"
#include "common_def.h"
#include "chip_io.h"
#include "hal_keyscan.h"
#include "keyscan_porting.h"
#include "keyscan.h"
#ifdef CONFIG_KEYSCAN_SOFTWARE_GHOST_DETECT
#include "osal_list.h"
#endif

#define KEYSCAN_MAX_REPORT_CHANNEL_NUM                      3
#define KEY_VALUE_STRING_LEN                                5
#define CONVERT_DEC_TO_HEX                                  16
#define KEYSCAN_KEY_VALUE_FIFO_LEN                          0x09
#define KEYSCAN_COL_BITS                                    0x7
#define KEYSCAN_ROW_BITS                                    3
#define KEYSCAN_GHOST_MIN_KEYS                              3

#ifdef CONFIG_KEYSCAN_SOFTWARE_GHOST_DETECT
typedef struct {
    uint8_t col;
    uint8_t row;
    bool is_ghost;
} key_pos_t;

typedef struct {
    bool flag;
    key_pos_t pos;
    struct osal_list_head list;
} key_node_t;
#endif

static keyscan_report_callback_t g_hal_keyscan_report_list[KEYSCAN_MAX_REPORT_CHANNEL_NUM];
static bool g_keyscan_inited = false;
static bool g_powered = false;
static uint8_t g_keyscan_value_map[CONFIG_KEYSCAN_ENABLE_ROW][CONFIG_KEYSCAN_ENABLE_COL] = { 0 };
#ifndef CONFIG_KEYSCAN_ENABLE_REP_ALL_KEY
static uint8_t g_pressed_key_num = 0;
static uint8_t g_key_value_num[CONFIG_KEYSCAN_REPORT_MAX_NUMS] = { 0 };
static void keyscan_event_callback(uint16_t key_value);
#else
#define KEYSCAN_MAX_COLUMN                                  8
#define KEYSCAN_ALL_REPORT_INT_TIMES                        1
static uint8_t g_scan_int_times = 0;
static void keyscan_porting_all_keys_report(uint16_t key_value);
#endif

#ifdef CONFIG_KEYSCAN_SOFTWARE_GHOST_DETECT
static struct osal_list_head g_key_list = {0};
static key_node_t g_key_nodes[CONFIG_KEYSCAN_REPORT_MAX_NUMS] = {0};
static key_pos_t g_pre_key_pos[CONFIG_KEYSCAN_REPORT_MAX_NUMS] = {0};
static uint8_t g_pre_key_cnt = 0;
#endif

void uapi_keyscan_init(keyscan_pulse_time_t time, keyscan_mode_t mode, keyscan_int_t event_type)
{
    if (unlikely(g_keyscan_inited == true)) {
        return;
    }
    keyscan_port_register_hal_funcs();
#ifndef CONFIG_KEYSCAN_ENABLE_REP_ALL_KEY
    hal_keyscan_init(time, mode, event_type, keyscan_event_callback);
#else
    hal_keyscan_init(time, mode, event_type, keyscan_porting_all_keys_report);
#endif
#ifdef CONFIG_KEYSCAN_SOFTWARE_GHOST_DETECT
    OSAL_INIT_LIST_HEAD(&g_key_list);
#endif
    g_keyscan_inited = true;
}

void uapi_keyscan_deinit(void)
{
    if (unlikely(g_keyscan_inited == false)) {
        return;
    }
    hal_keyscan_deinit();
    keyscan_port_unregister_hal_funcs();
    g_keyscan_inited = false;
}

errcode_t uapi_keyscan_enable(void)
{
    if (unlikely(g_keyscan_inited == false)) {
        return ERRCODE_KEYSCAN_NOT_INIT;
    }
    uint32_t irq_sts = osal_irq_lock();
    errcode_t ret = hal_keyscan_enable();
    if (likely(ret == ERRCODE_KEYSCAN_POWER_ON)) {
        g_powered = true;
    }
    osal_irq_restore(irq_sts);

    return ret;
}

errcode_t uapi_keyscan_disable(void)
{
    if (unlikely(g_powered == false)) {
        return ERRCODE_KEYSCAN_NOT_POWER_ON;
    }
    errcode_t ret = hal_keyscan_disable();
    if (unlikely(ret == ERRCODE_SUCC)) {
        g_powered = false;
    }
    return ret;
}

errcode_t uapi_keyscan_register_callback(keyscan_report_callback_t callback)
{
    if (callback == NULL) {
        return ERRCODE_FAIL;
    }

    for (int32_t i = 0; i < KEYSCAN_MAX_REPORT_CHANNEL_NUM; i++) {
        if (g_hal_keyscan_report_list[i] == NULL) {
            g_hal_keyscan_report_list[i] = callback;
            return ERRCODE_SUCC;
        }
        if (g_hal_keyscan_report_list[i] == callback) {
            return ERRCODE_SUCC;
        }
    }
    return ERRCODE_FAIL;
}

errcode_t uapi_keyscan_unregister_callback(keyscan_report_callback_t callback)
{
    if (callback == NULL) {
        return ERRCODE_FAIL;
    }

    for (int32_t i = 0; i < KEYSCAN_MAX_REPORT_CHANNEL_NUM; i++) {
        if (g_hal_keyscan_report_list[i] == callback) {
            g_hal_keyscan_report_list[i] = NULL;
            return ERRCODE_SUCC;
        }
    }
    return ERRCODE_FAIL;
}

static void keyscan_info_report_invoke_callbacks(int key_nums, uint8_t *key_values)
{
    for (int32_t i = 0; i < KEYSCAN_MAX_REPORT_CHANNEL_NUM; i++) {
        if (g_hal_keyscan_report_list[i] != NULL) {
            g_hal_keyscan_report_list[i](key_nums, key_values);
        }
    }
}

#if defined(CONFIG_SAMPLE_SUPPORT_SLE_KEYBOARD) || defined(CONFIG_SAMPLE_SUPPORT_SLE_RCU_SERVER)
void keyscan_info_report_at_cmd(int key_nums, uint8_t *key_values)
{
    for (int32_t i = 0; i < KEYSCAN_MAX_REPORT_CHANNEL_NUM; i++) {
        if (g_hal_keyscan_report_list[i] != NULL) {
            g_hal_keyscan_report_list[i](key_nums, key_values);
        }
    }
}
#endif

#ifndef CONFIG_KEYSCAN_ENABLE_REP_ALL_KEY
static uint16_t keyscan_get_row(uint16_t value)
{
    uint8_t low_bits = value & 0xFF;
    return (uint16_t)(low_bits >> KEYSCAN_ROW_BITS);
}

static uint8_t keyscan_get_col(uint16_t value)
{
    uint8_t low_bits = value & 0xFF;
    return (low_bits & KEYSCAN_COL_BITS);
}

static key_value_report_status_t keyscan_get_value_and_status(uint16_t key_value, uint16_t *row, uint8_t *clo)
{
    *row = keyscan_get_row(key_value);
    *clo = keyscan_get_col(key_value);
    if ((*row > CONFIG_KEYSCAN_ENABLE_ROW) || (*clo > CONFIG_KEYSCAN_ENABLE_COL)) {
        *row = 0;
        *clo = 0;
        return KEY_ERROR;
    }
    return (key_value >> (KEYSCAN_KEY_VALUE_FIFO_LEN - 1));
}

#ifdef CONFIG_KEYSCAN_SOFTWARE_GHOST_DETECT
static bool key_node_append(key_pos_t *pos)
{
    for (uint8_t i = 0; i < CONFIG_KEYSCAN_REPORT_MAX_NUMS; i++) {
        if (!g_key_nodes[i].flag) {
            g_key_nodes[i].pos.row = pos->row;
            g_key_nodes[i].pos.col = pos->col;
            g_key_nodes[i].flag = true;
            osal_list_add_tail(&g_key_nodes[i].list, &g_key_list);
            g_pressed_key_num++;
            return true;
        }
    }
    return false;
}

static void key_node_delete(key_pos_t *pos)
{
    struct osal_list_head *node = g_key_list.next;
    key_node_t *key_data = osal_list_entry(node, key_node_t, list);
    key_node_t *next_data = osal_list_entry(node->next, key_node_t, list);
    osal_list_for_each_entry_safe(key_data, next_data, &g_key_list, list) {
        if (key_data->flag && key_data->pos.row == pos->row && key_data->pos.col == pos->col) {
            key_data->flag = false;
            osal_list_del(&key_data->list);
            g_pressed_key_num--;
        }
    }
}

static uint8_t key_node_export(key_pos_t *buf, uint8_t bufLen)
{
    uint8_t cnt = 0;
    key_node_t *key_data = NULL;
    osal_list_for_each_entry(key_data, &g_key_list, list) {
        if (cnt >= bufLen) {
            return 0 ;
        }
        if (buf[cnt].is_ghost == 0) {
            buf[cnt].row = key_data->pos.row;
            buf[cnt].col = key_data->pos.col;
            cnt++;
        }
    }
    return cnt;
}

static void keyscan_sw_ghost_detect(key_value_report_status_t status, uint16_t row, uint8_t col)
{
    key_pos_t pos = {.col = col, .row = row};
    if (status == KEY_PRESS) {
        key_node_append(&pos);
    } else {
        key_node_delete(&pos);
    }
    key_pos_t cur_key_pos[CONFIG_KEYSCAN_REPORT_MAX_NUMS] = { 0 };
    uint8_t cur_key_cnt = key_node_export(cur_key_pos, CONFIG_KEYSCAN_REPORT_MAX_NUMS);
    uint8_t act_key_cnt = 0;

    if (cur_key_cnt >= KEYSCAN_GHOST_MIN_KEYS) {
        bool same_col = false;
        bool same_row = false;
        for (int i = 0; i < cur_key_cnt; i++) {
            for (int j = i + 1; j < cur_key_cnt; j++) {
                if (cur_key_pos[i].col == cur_key_pos[j].col) {
                    same_col = true;
                } else if (cur_key_pos[i].row == cur_key_pos[j].row) {
                    same_row = true;
                }
            }

            if (same_col && same_row) {
                bool is_exist = false;
                for (int k = 0; k < g_pre_key_cnt; k++) {
                    if (g_pre_key_pos[k].row == cur_key_pos[i].row && g_pre_key_pos[k].col == cur_key_pos[i].col) {
                        is_exist = true;
                        break;
                    }
                }
                if (!is_exist) { cur_key_pos[i].is_ghost = true; }
            }
        }
        act_key_cnt = key_node_export(cur_key_pos, CONFIG_KEYSCAN_REPORT_MAX_NUMS);
    } else {
        act_key_cnt = cur_key_cnt;
    }

    if (act_key_cnt == g_pre_key_cnt &&
        memcmp((void*)&g_pre_key_pos, (void*)&cur_key_pos, act_key_cnt * sizeof(key_pos_t)) == 0) { return; }
    (void)memcpy_s(&g_pre_key_pos, sizeof(g_pre_key_pos), &cur_key_pos, sizeof(cur_key_pos));
    g_pre_key_cnt = act_key_cnt;

    for (int i = 0; i < act_key_cnt; i++) {
        g_key_value_num[i] = g_keyscan_value_map[cur_key_pos[i].row][cur_key_pos[i].col];
    }
    keyscan_info_report_invoke_callbacks(act_key_cnt, g_key_value_num);
}
#endif

static void keyscan_event_callback(uint16_t key_value)
{
    uint16_t row;
    uint8_t col;
    key_value_report_status_t status = keyscan_get_value_and_status(key_value, &row, &col);
    uint8_t temp = g_keyscan_value_map[row][col];
    if (temp == 0 || status == KEY_ERROR) {
        return;
    }
#ifdef CONFIG_KEYSCAN_SOFTWARE_GHOST_DETECT
    keyscan_sw_ghost_detect(status, row, col);
#else
    if (status == KEY_PRESS) {
        bool is_key_pressed = false;
        for (uint8_t i = 0; i <= g_pressed_key_num; i++) {
            if (g_key_value_num[i] == temp) {
                is_key_pressed = true;
            }
        }

        if ((!is_key_pressed) && (g_pressed_key_num < CONFIG_KEYSCAN_REPORT_MAX_NUMS)) {
            g_key_value_num[g_pressed_key_num] = temp;
            g_pressed_key_num++;
        }
    } else if (status == KEY_RELEASE) {
        for (uint8_t i = 0; i < g_pressed_key_num; i++) {
            if (g_key_value_num[i] == temp) {
                g_key_value_num[i] = g_key_value_num[g_pressed_key_num - 1];
                g_key_value_num[g_pressed_key_num - 1] = 0;
                g_pressed_key_num--;
                break;
            }
        }
    }
    keyscan_info_report_invoke_callbacks(g_pressed_key_num, g_key_value_num);
#endif
}
#else
/*
    SOC use 9*16-bits registers to make a matrix to indicate key pressed status.
         0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15
    0x0  r0c0  r0c1  r0c2  r0c3  r0c4  r0c5  r0c6  r0c7  r1c0  r1c1  r1c2  r1c3  r1c4  r1c5  r1c6  r1c7
    0x4  r2c0  r2c1  r2c2  r2c3  r2c4  r2c5  r2c6  r2c7  r3c0  r3c1  r3c2  r3c3  r3c4  r3c5  r3c6  r3c7
    0x8  r4c0  r4c1  r4c2  r4c3  r4c4  r4c5  r4c6  r4c7  r5c0  r5c1  r5c2  r5c3  r5c4  r5c5  r5c6  r5c7
    0xC  r6c0  r6c1  r6c2  r6c3  r6c4  r6c5  r6c6  r6c7  r7c0  r7c1  r7c2  r7c3  r7c4  r7c5  r7c6  r7c7
    0x10 r8c0  r8c1  r8c2  r8c3  r8c4  r8c5  r8c6  r8c7  r9c0  r9c1  r9c2  r9c3  r9c4  r9c5  r9c6  r9c7
    0x14 r10c0 r10c1 r10c2 r10c3 r10c4 r10c5 r10c6 r10c7 r11c0 r11c1 r11c2 r11c3 r11c4 r11c5 r11c6 r11c7
    0x18 r12c0 r12c1 r12c2 r12c3 r12c4 r12c5 r12c6 r12c7 r13c0 r13c1 r13c2 r13c3 r13c4 r13c5 r13c6 r13c7
    0x1C r16c0 r16c1 r16c2 r16c3 r16c4 r16c5 r16c6 r16c7 r15c0 r15c1 r15c2 r15c3 r15c4 r15c5 r15c6 r15c7
    0x20 r18c0 r18c1 r18c2 r18c3 r18c4 r18c5 r18c6 r18c7 r17c0 r17c1 r17c2 r17c3 r17c4 r17c5 r17c6 r17c7
*/
static void keyscan_porting_all_keys_report(uint16_t key_value)
{
    unused(key_value);
    // Scan matrix to achieve all keys report need to enable scan once int, so cannot respond every irq.
    if (g_scan_int_times++ < KEYSCAN_ALL_REPORT_INT_TIMES) { return; }
    uint8_t pressed_key_num = 0;
    uint8_t *key_value_arr = (uint8_t *)osal_vmalloc(CONFIG_KEYSCAN_ENABLE_ROW * CONFIG_KEYSCAN_ENABLE_COL);
    if (key_value_arr == NULL) {
        return;
    }
    g_scan_int_times = 0;
    for (int8_t i = 0; i < ((CONFIG_KEYSCAN_ENABLE_ROW + 1) >> 1); i++) {
        uint16_t scan_reg = readw(KEYSCAN_MARX_ADDR + (i * sizeof(uint32_t)));
        if (!scan_reg) { continue; }
        // 一个寄存器表示两个row，i需要乘2再加上j除以8来确定目前的row值，若j大于7则当前bit表示的column在下一组row
        // 因最多有8个bit来确认column，第二组row的j只取低三位，就代表了该column的序号
        for (int8_t j = 0; j < CONFIG_KEYSCAN_ENABLE_COL; j++) {
            if (scan_reg & BIT(j)) {
                key_value_arr[pressed_key_num++] = g_keyscan_value_map[(i << 1) + (j >> 0x3)][j & 0x7];
            }
        }
        for (int8_t j = KEYSCAN_MAX_COLUMN; j < KEYSCAN_MAX_COLUMN + CONFIG_KEYSCAN_ENABLE_COL; j++) {
            if (scan_reg & BIT(j)) {
                key_value_arr[pressed_key_num++] = g_keyscan_value_map[(i << 1) + (j >> 0x3)][j & 0x7];
            }
        }
    }
    keyscan_info_report_invoke_callbacks(pressed_key_num, key_value_arr);
    osal_vfree(key_value_arr);
}
#endif

errcode_t uapi_set_keyscan_value_map(uint8_t **map_addr, uint16_t row, uint8_t col)
{
    if (map_addr == NULL) {
        return ERRCODE_KEYSCAN_NULL;
    }
    if (row != CONFIG_KEYSCAN_ENABLE_ROW || col != CONFIG_KEYSCAN_ENABLE_COL) {
        return ERRCODE_KEYSCAN_MAP_WRONG_SIZE;
    }
    if (memcpy_s(g_keyscan_value_map, sizeof(g_keyscan_value_map), map_addr, sizeof(g_keyscan_value_map)) != 0) {
        return ERRCODE_KEYSCAN_SAFE_FUNC_FAIL;
    }
    return ERRCODE_SUCC;
}

#if defined (CONFIG_KEYSCAN_SUPPORT_LPM)
static uint8_t g_suspend_flag = 0;
errcode_t uapi_keyscan_suspend(uintptr_t arg)
{
    errcode_t ret = ERRCODE_SUCC;
    if (g_keyscan_inited == false) {
        return ret;
    }
    if (g_suspend_flag == 1) {
        return ERRCODE_SUCC;
    } else {
        ret = hal_keyscan_suspend(arg);
        if (ret == ERRCODE_SUCC) {
            g_suspend_flag = 1;
        }
    }
    return ret;
}

errcode_t uapi_keyscan_resume(uintptr_t arg)
{
    errcode_t ret = ERRCODE_SUCC;
    if (g_keyscan_inited == false) {
        return ret;
    }
    if (g_suspend_flag == 0) {
        return ERRCODE_SUCC;
    } else {
        ret = hal_keyscan_resume(arg);
        if (ret == ERRCODE_SUCC) {
            g_suspend_flag = 0;
        }
    }
    return ret;
}
#endif