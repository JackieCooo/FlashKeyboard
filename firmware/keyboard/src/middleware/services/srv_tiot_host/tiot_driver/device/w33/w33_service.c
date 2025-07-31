/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: W33 service interface. \n
 *
 * History: \n
 * 2023-03-31, Create file. \n
 */
#include "w33_service.h"
#include "w33_device_info.h"
#include "w33_board_port.h"
#include "tiot_board_log.h"
#include "tiot_controller.h"
#include "tiot_controller_helper.h"
#ifdef CONFIG_W33_SPI
#include "tiot_board_spi_port.h"
#else
#include "tiot_packet.h"
#include "tiot_board_uart_port.h"
#endif
#include "tiot_sys_msg_types.h"
#include "tiot_board_pin_port.h"
#include "tiot_board_misc_port.h"
#include "tiot_board.h"
#include "tiot_service_interface.h"
#ifdef CONFIG_FILE_BY_ARRAY
#include "w33_fw_file_array.h"
#endif

#ifndef CONFIG_BOARD_DYNAMIC_ALLOC
/* 加载 & 数据收发复用，不同时使用 */
/* 数据收发暂定5K用于缓存数据. */
/* 固件加载时 ~3K用于保存解析后命令, ~128字节用于加载回复接收. */
#define W33_FW_LOAD_REUSE_BUFF_START 0U
#define W33_FW_LOAD_RX_BUFF_START    W33_FW_LOAD_REUSE_BUFF_START
#define W33_FW_LOAD_RX_BUFF_SIZE     128U

#define W33_FW_LOAD_REUSE_BUFF_END   (W33_FW_LOAD_RX_BUFF_START + W33_FW_LOAD_RX_BUFF_SIZE)

#define W33_COMMON_BUFF_MAX    ((CONFIG_W33_PACKET_BUFF_MAX > W33_FW_LOAD_REUSE_BUFF_END) ? \
                                 CONFIG_W33_PACKET_BUFF_MAX : W33_FW_LOAD_REUSE_BUFF_END)
#endif

#define W33_PACKET_QUEUE_NODES    8

typedef enum {
    W33_RX_SUBSYS_0 = 0,
    W33_RX_SUBSYS_MAX
} w33_rx_subsys_id;

static tiot_controller g_w33_controllers[CONFIG_W33_DEV_NUM] = { 0 };
#ifdef CONFIG_W33_UART
/* 仅UART需要，使用SPI可优化. */
static tiot_packet_context g_w33_pkt_ctx[CONFIG_W33_DEV_NUM];
static tiot_packet g_w33_pkt_queue_nodes[CONFIG_W33_DEV_NUM][W33_PACKET_QUEUE_NODES];
static tiot_packet_queue g_w33_pkt_queues[CONFIG_W33_DEV_NUM][W33_RX_SUBSYS_MAX];
#endif
#ifndef CONFIG_BOARD_DYNAMIC_ALLOC
static uint8_t g_w33_common_buff_mem[CONFIG_W33_DEV_NUM][W33_COMMON_BUFF_MAX];
#endif

void w33_service_close_ext(uint8_t dev_id);

static uint16_t w33_packet_manager_match_func(uint32_t subsys_code)
{
    if ((subsys_code == GNSS_LAST_MSG)) {
        return W33_RX_SUBSYS_0;
    }
    return W33_RX_SUBSYS_MAX;
}

static void w33_device_wakeup_host_cb(uint32_t pin)
{
    tiot_controller *cur_ctrl;
    (void)tiot_board_pin_enable_int(pin, TIOT_PIN_INT_DISABLE);
    cur_ctrl = tiot_find_controller_by_pin(g_w33_controllers, CONFIG_W33_DEV_NUM,
                                           pin, W33_PIN_DEVICE_WAKEUP_HOST);
    if (cur_ctrl != NULL) {
        tiot_pm_set_event(&cur_ctrl->pm, TIOT_PM_EVENT_WAKEUP_ACK);
    }
#ifdef CONFIG_W33_SPI
    osal_wait_wakeup_interruptible(&(cur_ctrl->transfer.rx_wait));
#endif

    (void)tiot_board_pin_enable_int(pin, TIOT_PIN_INT_ENABLE);
}

#ifdef CONFIG_W33_UART
static inline int32_t w33_send_host_need_wakeup(tiot_controller *ctrl)
{
    int32_t ret;
    const uint8_t host_need_wakeup_cmd = SYS_PL_PM_SET_WAKEUP_HOST_VALID;
    const tiot_xfer_tx_param param = { SUBSYS_SYS };
    ret = tiot_xfer_send(&ctrl->transfer, &host_need_wakeup_cmd, sizeof(uint8_t), &param);
    if (ret != sizeof(uint8_t)) {
        tiot_print_err("[TIOT]set host need wakeup cmd fail.\r\n");
        return -1;
    }
    return 0;
}

static int32_t w33_service_packet_init(tiot_controller *ctrl, uint8_t dev_id)
{
    int32_t ret;
#ifndef CONFIG_BOARD_DYNAMIC_ALLOC
    tiot_packet_buffer pkt_buff = { g_w33_common_buff_mem[dev_id], CONFIG_W33_PACKET_BUFF_MAX, 0 };
#endif
    tiot_packet_context_param param = {
        .flags = 0,
        .rx_manager_info = { g_w33_pkt_queues[dev_id], W33_RX_SUBSYS_MAX, w33_packet_manager_match_func },
        .buff = NULL
    };
#ifndef CONFIG_BOARD_DYNAMIC_ALLOC
    param.buff = &pkt_buff;
#endif
    g_w33_pkt_queues[dev_id][W33_RX_SUBSYS_0].queue_nodes = g_w33_pkt_queue_nodes[dev_id];
    g_w33_pkt_queues[dev_id][W33_RX_SUBSYS_0].queue_size = W33_PACKET_QUEUE_NODES;
    tiot_packet_context *pkt_ctx = &g_w33_pkt_ctx[dev_id];
    ret = tiot_packet_init(pkt_ctx, &ctrl->transfer, &param);
    if (ret != 0) {
        tiot_print_err("[TIOT]packet init fail.\r\n");
    }
    return ret;
}

static int32_t w33_service_install_packet(tiot_controller *ctrl, uint8_t dev_id)
{
    tiot_xfer_packet_ops w33_packet_ops = {
        .read_out = tiot_packet_read_out,
        .rx_data_store = tiot_packet_rx_data_store,
        .tx_push = tiot_packet_tx_push,
    };
    const tiot_device_timings *timings = &ctrl->dev_info->timings;
    /* 收发模块注册包格式 */
    if (tiot_xfer_install_packet(&ctrl->transfer, (tiot_xfer_packet_ops *)&w33_packet_ops,
                                 (uintptr_t)&g_w33_pkt_ctx[dev_id]) != 0) {
        tiot_print_err("[TIOT]packet install fail.\r\n");
        return -1;
    }
    /* 必须等待device UART初始化后才能接收消息, 待确认时间. */
    osal_msleep(timings->init_time_ms);
    tiot_pm_set_event(&ctrl->pm, TIOT_PM_EVENT_REPORT_WORK);
    return 0;
}
#endif

#ifdef CONFIG_W33_SPI
static inline int32_t w33_service_packet_init(tiot_controller *ctrl, uint8_t dev_id)
{
    tiot_unused(ctrl);
    tiot_unused(dev_id);
    return 0;
}

static int32_t w33_service_install_packet(tiot_controller *ctrl, uint8_t dev_id)
{
    tiot_unused(ctrl);
    tiot_unused(dev_id);
    return 0;
}
#endif

static inline int32_t w33_service_custom_msg_send(tiot_controller *ctrl, uint32_t dev_wkup_host_pin)
{
#ifdef CONFIG_W33_UART
    /* 通过管脚有无判断host是否需要唤醒 */
    if (dev_wkup_host_pin == TIOT_PIN_NONE) {
        return 0;
    }
    if (w33_send_host_need_wakeup(ctrl) != 0) {
        return -1;
    }
#else
    tiot_unused(ctrl);
    tiot_unused(dev_wkup_host_pin);
#endif
    /* 暂不支持切换波特率 */
    return 0;
}

static inline int32_t w33_get_dev_id(tiot_controller *ctrl, uint8_t *dev_id)
{
    uint8_t i;
    for (i = 0; i < CONFIG_W33_DEV_NUM; i++) {
        if (ctrl == &g_w33_controllers[i]) {
            *dev_id = i;
            return 0;
        }
    }
    tiot_print_err("[TIoT]invalid device\n");
    return -1;
}

static inline void w33_set_open_param(tiot_xfer_open_param *xfer_open_param, tiot_service_open_param *param)
{
    if (param == NULL) {
        return;
    }
    xfer_open_param->rx_cbk_param.buff = param->buff;
    xfer_open_param->rx_cbk_param.buff_len = param->buff_len;
    xfer_open_param->rx_cbk_param.rx_callback = param->rx_callback;
}

static inline int32_t w33_firmware_load(tiot_fw *firmware)
{
    int ret = -1;
#ifdef CONFIG_FILE_BY_ARRAY
    ret = tiot_firmware_load(firmware, w33_fw_file_path_get(), TIOT_FW_FILE_ARRAY_CFG_NAME);
#else
    ret = tiot_firmware_load(firmware, NULL, w33_board_get_info()->cfg_path);
#endif
    if (ret != 0) {
        tiot_print_err("[TIoT] W33 firmware load fail.\n");
    }
    return ret;
}

static int32_t w33_service_open(tiot_controller *ctrl, void *param)
{
    int ret = -1;
    uint8_t dev_id;
    uint32_t dev_wkup_host_pin;
    tiot_xfer_open_param xfer_open_param = { 0 };
    if (w33_get_dev_id(ctrl, &dev_id) != 0) {
        return -1;
    }
    w33_set_open_param(&xfer_open_param, param);
    /* 传输启动 */
    if (tiot_xfer_open(&ctrl->transfer, &xfer_open_param) != 0) {
        /* 通信接口打开失败 */
        goto xfer_open_fail;
    }
#ifdef CONFIG_W33_SPI
    /* 使能中断 */
    dev_wkup_host_pin = ctrl->pm.pm_info[TIOT_PIN_DEVICE_WAKEUP_HOST];
    tiot_pin_int_request(dev_wkup_host_pin, w33_device_wakeup_host_cb);
#endif
    /* 上电 */
    if (tiot_pm_set_event(&ctrl->pm, TIOT_PM_EVENT_POWERON) != 0) {
        /* 上电失败 */
        tiot_print_err("[TIoT]Power on fail\n");
        goto power_on_fail;
    }
    /* 加载固件 */
    ret = w33_firmware_load(&ctrl->firmware);
    if (ret != 0) {
        goto fw_load_fail;
    }
#ifdef CONFIG_W33_UART
    dev_wkup_host_pin = ctrl->pm.pm_info[TIOT_PIN_DEVICE_WAKEUP_HOST];
    tiot_pin_int_request(dev_wkup_host_pin, w33_device_wakeup_host_cb);
#endif
    ret = w33_service_packet_init(ctrl, dev_id);
    if (ret != 0) {
        goto pkt_init_fail;
    }
    ret = w33_service_install_packet(ctrl, dev_id);
    if (ret != 0) {
        goto pkt_install_fail;
    }
    ret = w33_service_custom_msg_send(ctrl, dev_wkup_host_pin);
    if (ret != 0) {
        goto pkt_install_fail;
    }
    /* 打开OK */
    tiot_print_suc("[TIoT]Controller open succ.\n");
    return 0;

pkt_install_fail:
#ifdef CONFIG_W33_UART
    tiot_packet_deinit(&g_w33_pkt_ctx[dev_id]);
#endif
pkt_init_fail:
#ifdef CONFIG_W33_UART
    tiot_pin_int_release(dev_wkup_host_pin);
#endif
fw_load_fail:
    (void)tiot_pm_set_event(&ctrl->pm, TIOT_PM_EVENT_POWEROFF);
power_on_fail:
#ifdef CONFIG_W33_SPI
    tiot_pin_int_release(dev_wkup_host_pin);
#endif
    tiot_xfer_close(&ctrl->transfer);
xfer_open_fail:
    return ret;
}

static void w33_service_close(tiot_controller *ctrl)
{
    uint8_t dev_id;
    uint32_t dev_wkup_host_pin;
    if (w33_get_dev_id(ctrl, &dev_id) != 0) {
        return;
    }
    dev_wkup_host_pin = ctrl->pm.pm_info[TIOT_PIN_DEVICE_WAKEUP_HOST];
    (void)tiot_pm_set_event(&ctrl->pm, TIOT_PM_EVENT_POWEROFF);
    tiot_pin_int_release(dev_wkup_host_pin);
    tiot_xfer_close(&ctrl->transfer);
    tiot_xfer_uninstall_packet(&ctrl->transfer);
#ifdef CONFIG_W33_UART
    tiot_packet_deinit(&g_w33_pkt_ctx[dev_id]);
#endif
}

static int32_t w33_service_write(tiot_controller *ctrl, const uint8_t *data,
                                 uint32_t len, const tiot_xfer_tx_param *param)
{
    tiot_unused(param);
    uint8_t dev_id;
    const tiot_xfer_tx_param tx_param = { SUBSYS_GNSS };
    if (w33_get_dev_id(ctrl, &dev_id) != 0) {
        return -1;
    }
    tiot_pm *pm = &ctrl->pm;
    (void)tiot_pm_set_event(pm, TIOT_PM_EVENT_WORK_VOTE_UP);
    /* 尝试唤醒device */
    int32_t ret = tiot_pm_set_event(pm, TIOT_PM_EVENT_WAKEUP);
    if (ret != 0) {
        tiot_print_err("[TIoT]wake up error\n");
        goto vote_down;
    }
    /* 等待成功唤醒device */
    ret = tiot_pm_wait_state(&ctrl->pm, TIOT_PM_STATE_WORK, ctrl->dev_info->timings.wakeup_wait_ms);
    if (ret <= 0) {
        tiot_print_warning("[TIoT]wakeup device fail, pm state is 0x%x.\n", pm->state);
    }
    ret = tiot_xfer_send(&ctrl->transfer, data, len, &tx_param);
vote_down:
    (void)tiot_pm_set_event(pm, TIOT_PM_EVENT_WORK_VOTE_DOWN);
    return ret;
}

static int32_t w33_service_read(tiot_controller *ctrl, uint8_t *buff, uint32_t buff_len,
                                const tiot_xfer_rx_param *param)
{
    uint8_t dev_id;
    if (w33_get_dev_id(ctrl, &dev_id) != 0) {
        return -1;
    }
    return tiot_xfer_recv(&ctrl->transfer, buff, buff_len, param);
}

int32_t w33_service_init(void)
{
    uint8_t i;
    int32_t ret;
    tiot_board_info per_board_info;
    tiot_controller_info ctrl_info;
    const w33_board_info *board_info;
    for (i = 0; i < CONFIG_W33_DEV_NUM; i++) {
        if (w33_board_init(&i) != 0) {
            tiot_print_err("[TIoT]board init fail\n");
            return -1;
        }
    }
    board_info = w33_board_get_info();
    if (board_info == NULL) {
        tiot_print_err("[TIoT]no board info.\n");
        goto deinit;
    }
    for (i = 0; i < CONFIG_W33_DEV_NUM; i++) {
        per_board_info.hw_info.xmit_id = board_info->hw_infos[i].xmit_id;
        per_board_info.hw_info.pm_info = board_info->hw_infos[i].pm_info;
        ctrl_info.board_info = &per_board_info;
        ctrl_info.dev_info = w33_device_get_info();

        ret = tiot_controller_init(&g_w33_controllers[i], &ctrl_info);
        if (ret != 0) {
            tiot_print_err("[TIoT]controller init fail\n");
            goto deinit;
        }
    }
    for (i = 0; i < CONFIG_W33_DEV_NUM; i++) {
        g_w33_controllers[i].ops.open = w33_service_open;
        g_w33_controllers[i].ops.close = w33_service_close;
        g_w33_controllers[i].ops.write = w33_service_write;
        g_w33_controllers[i].ops.read = w33_service_read;
        g_w33_controllers[i].ops.pm_ctrl = NULL;
    }
    return 0;
deinit:
    w33_service_deinit();
    return -1;
}

void w33_service_deinit(void)
{
    uint8_t i;
    for (i = 0; i < CONFIG_W33_DEV_NUM; i++) {
        tiot_controller_deinit(&g_w33_controllers[i]);
    }
    for (i = 0; i < CONFIG_W33_DEV_NUM; i++) {
        w33_board_deinit(&i);
    }
}

uintptr_t w33_service_get_ctrl(uint8_t dev_id)
{
    return (uintptr_t)&g_w33_controllers[dev_id];
}
