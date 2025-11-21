#include "stdbool.h"
#include "product.h"
#include "common_def.h"
#include "osal_debug.h"
#include "soc_osal.h"
#include "securec.h"
#include "bts_device_manager.h"
#include "bts_le_gap.h"

#include "ble_keyboard_server_adv.h"
#include "ble_hid_keyboard_server.h"
#include "ble_keyboard_server.h"

#define LOG_TAG "ble"
#include "debug.h"

static uint16_t g_ble_conn_id = 0;
static bool g_connected = false;
static bd_addr_t g_peer_addr = {0};

static service_ctrl_func_t g_func = {
    .init = ble_keyboard_server_init,
    .deinit = ble_keyboard_server_deinit,
    .connected = ble_connected,
    .send = ble_keyboard_send_key,
    .set_receive_callback = NULL
};


static void ble_keyboard_adv_enable_cbk(uint8_t adv_id, adv_status_t status)
{
    LOG("adv enabled, adv_id: %d, status: %d", adv_id, status);
}

static void ble_keyboard_adv_disable_cbk(uint8_t adv_id, adv_status_t status)
{
    LOG("adv disabled, adv_id: %d, status: %d", adv_id, status);
}

static void ble_keyboard_connect_change_cbk(
    uint16_t conn_id, bd_addr_t *addr, gap_ble_conn_state_t conn_state,
    gap_ble_pair_state_t pair_state, gap_ble_disc_reason_t disc_reason
)
{
    LOG("connection change, conn_id: %d, status: %d, pair_status: %d, disc_reason: 0x%x",
        conn_id, conn_state, pair_state, disc_reason);
    LOG_BUF("addr", addr->addr, BD_ADDR_LEN);

    if (conn_state == GAP_BLE_STATE_DISCONNECTED) {
        /* 连接断开 */
        g_ble_conn_id = 0;
        memset(&g_peer_addr, 0, sizeof(bd_addr_t));

        if (disc_reason != BLE_DISCONNECT_BY_LOCAL_HOST) {
            /* 重新广播 */
            ble_keyboard_set_adv_data();
            ble_keyboard_start_adv();
        }
    } else if (conn_state == GAP_BLE_STATE_CONNECTED) {
        /* 连接成功 */
        g_ble_conn_id = conn_id;
        memcpy(&g_peer_addr, addr, sizeof(bd_addr_t));
    }

    g_connected = conn_state;
}

static void ble_power_on_cbk(uint8_t status)
{
    LOG("power on status: 0x%x", status);
    enable_ble();
}

static void ble_enable_cbk(uint8_t status)
{
    LOG("enable status: 0x%x", status);

    ble_hiddev_keyboard_server_init();
    ble_keyboard_set_adv_data();
    ble_keyboard_start_adv();
}

static void ble_disable_cbk(uint8_t status)
{
    LOG("disable status: 0x%x", status);
}

int ble_keyboard_server_init(void)
{
    /* 注册BLE回调 */
    bts_dev_manager_callbacks_t dev_mgr_cb = { 0 };
    dev_mgr_cb.power_on_cb = ble_power_on_cbk;
    dev_mgr_cb.ble_enable_cb = ble_enable_cbk;
    dev_mgr_cb.ble_disable_cb = ble_disable_cbk;

    gap_ble_callbacks_t gap_cb = { 0 };
    gap_cb.start_adv_cb = ble_keyboard_adv_enable_cbk;
    gap_cb.stop_adv_cb = ble_keyboard_adv_disable_cbk;
    gap_cb.conn_state_change_cb = ble_keyboard_connect_change_cbk;

    if (gap_ble_register_callbacks(&gap_cb) != ERRCODE_BT_SUCCESS ||
        bts_dev_manager_register_callbacks(&dev_mgr_cb) != ERRCODE_BT_SUCCESS) {
        LOG("register ble_enable_cbk failed");
    }

    /* 使能BLE */
#if (CORE_NUMS < 2)
    enable_ble();
#endif

    return 0;
}

int ble_keyboard_server_deinit(void)
{
    /* 断开连接 */
    if (g_connected) {
        gap_ble_disconnect_remote_device(&g_peer_addr);
    }

    /* 关闭BLE */
    disable_ble();

    return 0;
}

bool ble_connected(void)
{
    return g_connected;
}

int ble_keyboard_send_key(const void* data, uint32_t len)
{
    return ble_hiddev_keyboard_server_send_input_report_by_uuid((const uint8_t*) data, (uint8_t) len);
}

service_ctrl_func_t* ble_keyboard_get_func(void)
{
    return &g_func;
}
