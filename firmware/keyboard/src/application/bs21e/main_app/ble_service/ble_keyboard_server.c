#include "stdbool.h"
#include "product.h"
#include "common_def.h"
#include "osal_debug.h"
#include "securec.h"
#include "bts_device_manager.h"
#include "bts_le_gap.h"
#include "ble_keyboard_server_adv.h"
#include "ble_hid_keyboard_server.h"
#include "ble_keyboard_server.h"

#define LOG_TAG "ble"
#include "debug.h"


static uint16_t g_ble_uart_conn_id = 0;
static bool g_connection_state = false;


static void ble_keyboard_adv_enable_cbk(uint8_t adv_id, adv_status_t status)
{
    LOG("adv enabled, adv_id: %d, status: %d", adv_id, status);
}

static void ble_keyboard_adv_disable_cbk(uint8_t adv_id, adv_status_t status)
{
    LOG("adv disabled, adv_id: %d, status: %d", adv_id, status);
}

bool ble_connected(void)
{
    return g_connection_state;
}

static void ble_keyboard_connect_change_cbk(
    uint16_t conn_id, bd_addr_t *addr, gap_ble_conn_state_t conn_state,
    gap_ble_pair_state_t pair_state, gap_ble_disc_reason_t disc_reason
)
{
    LOG("connect state change conn_id: %d, status: %d, pair_status:%d, disc_reason 0x%x",
        conn_id, conn_state, pair_state, disc_reason);
    LOG_BUF("addr", addr->addr, BD_ADDR_LEN);

    if (conn_state == GAP_BLE_STATE_DISCONNECTED) {
        /* 连接断开 */
        g_ble_uart_conn_id = 0;

        /* 重新广播 */
        ble_keyboard_set_adv_data();
        ble_keyboard_start_adv();
    } else if (conn_state == GAP_BLE_STATE_CONNECTED) {
        /* 连接成功 */
        g_ble_uart_conn_id = conn_id;
    }

    g_connection_state = conn_state;
}

static void ble_power_on_cbk(uint8_t status)
{
    LOG("power on status:0x%x", status);
    enable_ble();
}

static void ble_enable_cbk(uint8_t status)
{
    LOG("enable status:0x%x", status);

    ble_hiddev_keyboard_server_init();
    ble_keyboard_set_adv_data();
    ble_keyboard_start_adv();
}

void bt_core_enable_cb_register(void)
{
    bts_dev_manager_callbacks_t dev_mgr_cb = { 0 };
    gap_ble_callbacks_t gap_cb = { 0 };
    dev_mgr_cb.power_on_cb = ble_power_on_cbk;
    dev_mgr_cb.ble_enable_cb = ble_enable_cbk;
    gap_cb.start_adv_cb = ble_keyboard_adv_enable_cbk;
    gap_cb.stop_adv_cb = ble_keyboard_adv_disable_cbk;
    gap_cb.conn_state_change_cb = ble_keyboard_connect_change_cbk;

    if (gap_ble_register_callbacks(&gap_cb) != ERRCODE_BT_SUCCESS ||
        bts_dev_manager_register_callbacks(&dev_mgr_cb) != ERRCODE_BT_SUCCESS) {
        LOG("register ble_enable_cbk failed");
    }

#if (CORE_NUMS < 2)
    enable_ble();
#endif
}

void ble_keyboard_server_init(void)
{
    bt_core_enable_cb_register();
    LOG("ble init ok");
}
