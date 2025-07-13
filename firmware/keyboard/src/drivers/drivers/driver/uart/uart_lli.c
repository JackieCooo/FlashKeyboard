/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Provides uart driver source \n
 *
 * History: \n
 * 2024-09-09, Create file. \n
 */
#include <stdbool.h>
#include "common_def.h"
#include "soc_osal.h"
#include "securec.h"
#include "common_def.h"
#include "hal_dma.h"
#include "dma_porting.h"
#include "dma.h"
#include "uart.h"

#define UART_DATA_FIFO_OFFSET_ADDR         0x4
/* Uart message start byte. */
#define UART_MSG_START                     0x7E
#define UART_MSG_TYPE                      0x39
/* Uart message header length. */
#define UART_MSG_HEADER_LEN                4
#define UART_LLI_MSG_LEN                   2
#define UART_LLI_CHECK_BUFF_LEN            3
#define UART_BUSY                          1
#define UART_IDLE                          0
#define DMA_CH_CONFIG_OFFSET               0x104
#define DMA_RX_CHANNEL_OFFSET              0x20

#define UART_CHECK_BUFF_INDEX0             0
#define UART_CHECK_BUFF_INDEX1             1
#define UART_CHECK_BUFF_INDEX2             2
#define UART_CHECK_BUFF_INDEX3             3

typedef enum uart_lli_rx_sts {
    UART_LLI_HEAD,
    UART_LLI_CONTINUE_HEAD,
    UART_LLI_PAYLOAD
} uart_lli_rx_sts_t;

typedef struct uart_lli_head {
    uint8_t uart_msg_start;
    uint8_t uart_msg_type;
    uint16_t uart_msg_len;
} uart_lli_head_t;

typedef struct {
    uart_bus_t uart_bus;
    const void *buffer;
    uint16_t length;
    uart_data_send_cb_t cb;
} uart_msg_info_t;

typedef struct {
    dma_channel_t tx_dma_channel;
    dma_channel_t rx_dma_channel;
    dma_ch_user_peripheral_config_t tx_dma_config;
    dma_ch_user_peripheral_config_t rx_dma_config;
} uart_dma_lli_ch_t;

typedef struct {
    uint8_t head;
    uint8_t tail;
    uart_msg_info_t msg_buf[CONFIG_UART_DMA_LLI_MAX_MEMBER];
} msg_info_queue_stru_t;

#if defined(CONFIG_UART_SUPPORT_RECV_RAW_DATA)
static uint8_t g_uart_dma_receive_buff[CONFIG_DMA_WAIT_UART_IDLE_INT_LEN + CONFIG_UART_IDLE_PAYLOAD_LEN] = { 0 };
static uint8_t g_uart_idle_receive_buff[CONFIG_UART_IDLE_PAYLOAD_LEN] = { 0 };

static uart_buffer_config_t g_app_uart_buffer_config = {
    .rx_buffer = g_uart_idle_receive_buff,
    .rx_buffer_size = CONFIG_UART_IDLE_PAYLOAD_LEN
};

static uart_idle_int_receive_cb_t g_idle_recv_cb[UART_BUS_MAX_NUMBER] = { NULL };
dma_channel_t g_rx_dma_channel = DMA_CHANNEL_NONE;
dma_ch_user_peripheral_config_t g_rx_dma_config = { 0 };
uint8_t g_uart_bus = 0;
#endif

static uint8_t *g_lli_rx_payload_buff[UART_BUS_MAX_NUMBER] = { NULL };
static uint16_t g_lli_rx_payload_buff_len[UART_BUS_MAX_NUMBER] = { 0 };
static uart_data_recv_cb_t g_data_recv_cb[UART_BUS_MAX_NUMBER] = { NULL };
static uart_data_send_cb_t g_data_send_cb[UART_BUS_MAX_NUMBER] = { NULL };
volatile uint8_t  g_uart_dma_status[UART_BUS_MAX_NUMBER] = { 0 };
uart_lli_head_t g_uart_lli_header_buff[UART_BUS_MAX_NUMBER] = { {0} };
#if defined(CONFIG_UART_RX_DMA_NEED_HEADER)
static uint8_t g_uart_lli_rx_status[UART_BUS_MAX_NUMBER] = { 0 };
static uint8_t g_check_buff[UART_LLI_CHECK_BUFF_LEN] = { 0 };
#endif
static msg_info_queue_stru_t g_msg_info_queue[UART_BUS_MAX_NUMBER] = {0};
uart_dma_lli_ch_t g_uart_lli_dma_cfg[UART_BUS_MAX_NUMBER] = {
    {DMA_CHANNEL_NONE, DMA_CHANNEL_NONE, { 0 }, { 0 }},
#if UART_BUS_MAX_NUM > 1
    {DMA_CHANNEL_NONE, DMA_CHANNEL_NONE, { 0 }, { 0 }},
#endif
#if UART_BUS_MAX_NUM > 2
    {DMA_CHANNEL_NONE, DMA_CHANNEL_NONE, { 0 }, { 0 }}
#endif
};

static void uart_dma_lli_recv_cb(uint8_t intr, uint8_t channel, uintptr_t arg);
#if defined(CONFIG_UART_SUPPORT_RECV_RAW_DATA)
static void uart_dma_lli_idle_recv_cb(uint8_t intr, uint8_t channel, uintptr_t arg)
{
    unused(intr & arg & channel);
}
#endif
static void uart_dma_lli_tx_config(uart_bus_t bus)
{
    g_uart_lli_dma_cfg[bus].tx_dma_channel = (dma_channel_t)uapi_dma_get_lli_channel(HAL_DMA_BURST_TRANSACTION_LENGTH_1,
        uart_port_get_dma_trans_dest_handshaking(bus));
    g_uart_lli_dma_cfg[bus].tx_dma_config.src = (uint32_t)(uintptr_t)&g_uart_lli_header_buff[bus];
    g_uart_lli_dma_cfg[bus].tx_dma_config.dest = uart_porting_base_addr_get(bus) + UART_DATA_FIFO_OFFSET_ADDR;
    g_uart_lli_dma_cfg[bus].tx_dma_config.src_handshaking = 0;
    g_uart_lli_dma_cfg[bus].tx_dma_config.dest_handshaking = uart_port_get_dma_trans_dest_handshaking(bus);
    g_uart_lli_dma_cfg[bus].tx_dma_config.trans_type = HAL_DMA_TRANS_MEMORY_TO_PERIPHERAL_DMA;
    g_uart_lli_dma_cfg[bus].tx_dma_config.trans_dir = HAL_DMA_TRANSFER_DIR_MEM_TO_PERIPHERAL;
    g_uart_lli_dma_cfg[bus].tx_dma_config.src_increment = HAL_DMA_ADDRESS_INC_INCREMENT;
    g_uart_lli_dma_cfg[bus].tx_dma_config.dest_increment = HAL_DMA_ADDRESS_INC_NO_CHANGE;
    g_uart_lli_dma_cfg[bus].tx_dma_config.src_width = HAL_DMA_TRANSFER_WIDTH_8;
    g_uart_lli_dma_cfg[bus].tx_dma_config.dest_width = HAL_DMA_TRANSFER_WIDTH_8;
    g_uart_lli_dma_cfg[bus].tx_dma_config.priority = HAL_DMA_CH_PRIORITY_0;
    g_uart_lli_dma_cfg[bus].tx_dma_config.burst_length = HAL_DMA_BURST_TRANSACTION_LENGTH_1;
    g_uart_lli_dma_cfg[bus].tx_dma_config.protection = HAL_DMA_PROTECTION_CONTROL_NONE;
    g_uart_lli_dma_cfg[bus].tx_dma_config.transfer_num =
        (uint16_t)(sizeof(uart_lli_head_t) >> g_uart_lli_dma_cfg[bus].tx_dma_config.src_width);
}

static void uart_dma_lli_rx_head_config(uart_bus_t bus)
{
    g_uart_lli_dma_cfg[bus].rx_dma_config.src = uart_porting_base_addr_get(bus) + UART_DATA_FIFO_OFFSET_ADDR;
    g_uart_lli_dma_cfg[bus].rx_dma_config.trans_type = HAL_DMA_TRANS_PERIPHERAL_TO_MEMORY_DMA;
    g_uart_lli_dma_cfg[bus].rx_dma_config.trans_dir = HAL_DMA_TRANSFER_DIR_PERIPHERAL_TO_MEM;
    g_uart_lli_dma_cfg[bus].rx_dma_config.src_increment = HAL_DMA_ADDRESS_INC_NO_CHANGE;
    g_uart_lli_dma_cfg[bus].rx_dma_config.dest_increment = HAL_DMA_ADDRESS_INC_INCREMENT;
    g_uart_lli_dma_cfg[bus].rx_dma_config.src_width = HAL_DMA_TRANSFER_WIDTH_8;
    g_uart_lli_dma_cfg[bus].rx_dma_config.dest_width = HAL_DMA_TRANSFER_WIDTH_8;
    g_uart_lli_dma_cfg[bus].rx_dma_config.priority = HAL_DMA_CH_PRIORITY_0;
    g_uart_lli_dma_cfg[bus].rx_dma_config.burst_length = HAL_DMA_BURST_TRANSACTION_LENGTH_1;
    g_uart_lli_dma_cfg[bus].rx_dma_config.protection = HAL_DMA_PROTECTION_CONTROL_NONE;
    g_uart_lli_dma_cfg[bus].rx_dma_config.src_handshaking = uart_port_get_dma_trans_src_handshaking(bus);
    g_uart_lli_dma_cfg[bus].rx_dma_config.dest_handshaking = 0;
    g_uart_lli_dma_cfg[bus].rx_dma_channel = (dma_channel_t)uapi_dma_get_lli_channel(HAL_DMA_BURST_TRANSACTION_LENGTH_1,
        uart_port_get_dma_trans_src_handshaking(bus));
#if defined(CONFIG_UART_RX_DMA_NEED_HEADER)
    g_uart_lli_dma_cfg[bus].rx_dma_config.dest = (uint32_t)(uintptr_t)&g_uart_lli_header_buff[bus];
    g_uart_lli_dma_cfg[bus].rx_dma_config.transfer_num =
        (uint16_t)(sizeof(uart_lli_head_t) >> g_uart_lli_dma_cfg[bus].rx_dma_config.src_width);
#else
    g_uart_lli_dma_cfg[bus].rx_dma_config.dest = (uint32_t)(uintptr_t)g_lli_rx_payload_buff[bus];
    g_uart_lli_dma_cfg[bus].rx_dma_config.transfer_num =
        (g_lli_rx_payload_buff_len[bus] >> g_uart_lli_dma_cfg[bus].rx_dma_config.src_width);
#endif
}

#if defined(CONFIG_UART_RX_DMA_NEED_HEADER)
static errcode_t uart_dma_lli_update_recv_payload(uint8_t bus, uintptr_t dest)
{
    errcode_t ret = ERRCODE_FAIL;
    g_uart_lli_dma_cfg[bus].rx_dma_config.src_handshaking = uart_port_get_dma_trans_src_handshaking(bus);
    g_uart_lli_dma_cfg[bus].rx_dma_config.dest_handshaking = 0;
    g_uart_lli_dma_cfg[bus].rx_dma_config.dest = dest;
    g_uart_lli_dma_cfg[bus].rx_dma_channel = (dma_channel_t)uapi_dma_get_lli_channel(HAL_DMA_BURST_TRANSACTION_LENGTH_1,
        uart_port_get_dma_trans_src_handshaking(bus));
    g_uart_lli_dma_cfg[bus].rx_dma_config.transfer_num =
        (uint16_t)((g_uart_lli_header_buff[bus].uart_msg_len) >> g_uart_lli_dma_cfg[bus].rx_dma_config.src_width);

    ret = uapi_dma_configure_peripheral_transfer_lli(g_uart_lli_dma_cfg[bus].rx_dma_channel,
                                                     &(g_uart_lli_dma_cfg[bus].rx_dma_config), NULL);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
    return uapi_dma_enable_lli(g_uart_lli_dma_cfg[bus].rx_dma_channel, uart_dma_lli_recv_cb, (uintptr_t)dest);
}
#endif

errcode_t uapi_uart_dma_lli_continue_recv(uart_bus_t uart_bus)
{
    if (uart_bus >= UART_BUS_MAX_NUMBER) {
        return ERRCODE_FAIL;
    }
    g_uart_lli_dma_cfg[uart_bus].rx_dma_config.src_handshaking =
                    uart_port_get_dma_trans_src_handshaking(uart_bus);
    g_uart_lli_dma_cfg[uart_bus].rx_dma_channel =
        (dma_channel_t)uapi_dma_get_lli_channel(HAL_DMA_BURST_TRANSACTION_LENGTH_1,
        uart_port_get_dma_trans_src_handshaking(uart_bus));
#if defined(CONFIG_UART_RX_DMA_NEED_HEADER)
    g_uart_lli_dma_cfg[uart_bus].rx_dma_config.dest = (uint32_t)(uintptr_t)&g_uart_lli_header_buff[uart_bus];
    g_uart_lli_dma_cfg[uart_bus].rx_dma_config.transfer_num =
        (uint16_t)(sizeof(uart_lli_head_t) >> g_uart_lli_dma_cfg[uart_bus].rx_dma_config.src_width);
#endif
    uapi_dma_configure_peripheral_transfer_lli(g_uart_lli_dma_cfg[uart_bus].rx_dma_channel,
                                               &(g_uart_lli_dma_cfg[uart_bus].rx_dma_config), NULL);
    return uapi_dma_enable_lli(g_uart_lli_dma_cfg[uart_bus].rx_dma_channel, uart_dma_lli_recv_cb,
                               (uint32_t)(uintptr_t)&g_uart_lli_header_buff[uart_bus]);
}

#if defined(CONFIG_UART_RX_DMA_NEED_HEADER)
static errcode_t uart_dma_lli_head_check(uart_bus_t uart_bus)
{
    uint8_t *data = (uint8_t *)&g_uart_lli_header_buff[uart_bus];

    if ((data[UART_CHECK_BUFF_INDEX0] == UART_MSG_START) && (data[UART_CHECK_BUFF_INDEX1] == UART_MSG_TYPE)) {
        return ERRCODE_SUCC;
    } else if ((data[UART_CHECK_BUFF_INDEX1] == UART_MSG_START) && (data[UART_CHECK_BUFF_INDEX2] == UART_MSG_TYPE)) {
        g_check_buff[0] = data[UART_CHECK_BUFF_INDEX3];
        uapi_uart_read(uart_bus, g_check_buff + 1, 1, 0);
        memcpy_s(&(g_uart_lli_header_buff[uart_bus].uart_msg_len), UART_LLI_MSG_LEN, g_check_buff, UART_LLI_MSG_LEN);
        return ERRCODE_SUCC;
    } else if ((data[UART_CHECK_BUFF_INDEX2] == UART_MSG_START) && (data[UART_CHECK_BUFF_INDEX3] == UART_MSG_TYPE)) {
        uapi_uart_read(uart_bus, g_check_buff, UART_LLI_MSG_LEN, 0);
        memcpy_s(&(g_uart_lli_header_buff[uart_bus].uart_msg_len), UART_LLI_MSG_LEN, g_check_buff, UART_LLI_MSG_LEN);
        return ERRCODE_SUCC;
    } else if ((data[UART_CHECK_BUFF_INDEX3] == UART_MSG_START)) {
        uapi_uart_read(uart_bus, g_check_buff, UART_LLI_CHECK_BUFF_LEN, 0);
        if (g_check_buff[0] == UART_MSG_TYPE) {
            memcpy_s(&(g_uart_lli_header_buff[uart_bus].uart_msg_len),
                     UART_LLI_MSG_LEN, g_check_buff + 1, UART_LLI_MSG_LEN);
            return ERRCODE_SUCC;
        }
        return ERRCODE_UART_DMA_HEAD_WRONG;
    } else {
        return ERRCODE_UART_DMA_HEAD_WRONG;
    }
}

static errcode_t uart_dma_lli_head_recv(uart_bus_t uart_bus)
{
    if (uart_dma_lli_head_check(uart_bus) != ERRCODE_SUCC) {
        if (g_data_recv_cb[uart_bus] != NULL) {
            g_data_recv_cb[uart_bus](g_uart_lli_dma_cfg[uart_bus].rx_dma_channel,
                                     g_uart_lli_dma_cfg[uart_bus].rx_dma_config.transfer_num,
                                     ERRCODE_UART_DMA_HEAD_WRONG);
        }
        return ERRCODE_UART_DMA_HEAD_WRONG;
    }

    if (g_uart_lli_header_buff[uart_bus].uart_msg_len > g_lli_rx_payload_buff_len[uart_bus]) {
        if (g_data_recv_cb[uart_bus] != NULL) {
            g_data_recv_cb[uart_bus](g_uart_lli_dma_cfg[uart_bus].rx_dma_channel,
                                     g_uart_lli_dma_cfg[uart_bus].rx_dma_config.transfer_num,
                                     ERRCODE_UART_DMA_BUFF_OVERSIZE);
        }
        return ERRCODE_UART_DMA_BUFF_OVERSIZE;
    }
    /* 接收到头的场景: 停止DMA传输后 启动下一次DMA以接收真实数据 */
    g_uart_lli_rx_status[uart_bus] = UART_LLI_PAYLOAD;
    errcode_t ret = uart_dma_lli_update_recv_payload(uart_bus, (uintptr_t)g_lli_rx_payload_buff[uart_bus]);
    if (ret != ERRCODE_SUCC)  {
        return ret;
    }
    return ERRCODE_SUCC;
}
#endif

static void uart_dma_lli_recv_cb(uint8_t intr, uint8_t channel, uintptr_t arg)
{
    unused(intr & arg);
    bool is_continue = true;
    /* 获取当前channel对应的UART bus */
    uint8_t cur_bus = 0;
    while (cur_bus < UART_BUS_MAX_NUMBER) {
        if (g_uart_lli_dma_cfg[cur_bus].rx_dma_channel == channel) { break; }
        cur_bus++;
    }
    if (cur_bus >= UART_BUS_MAX_NUMBER) {
        return;
    }

    errcode_t ret = uapi_dma_end_transfer(channel);
    if (ret != ERRCODE_SUCC) { return; }
#if defined(CONFIG_UART_RX_DMA_NEED_HEADER)
    switch (g_uart_lli_rx_status[cur_bus]) {
        case UART_LLI_HEAD:
            uart_dma_lli_head_recv(cur_bus);
            break;
        case UART_LLI_PAYLOAD:
            g_uart_lli_rx_status[cur_bus] = UART_LLI_HEAD;
            if (g_data_recv_cb[cur_bus] != NULL) {
                is_continue = g_data_recv_cb[cur_bus](channel,
                    g_uart_lli_dma_cfg[cur_bus].rx_dma_config.transfer_num, ERRCODE_SUCC);
            }
            if (is_continue) {
                ret = uapi_uart_dma_lli_continue_recv(cur_bus);
                if (ret != ERRCODE_SUCC)  {
                    return;
                }
            }
            break;
        default:
            break;
    }
#else
    if (g_data_recv_cb[cur_bus] != NULL) {
        is_continue = g_data_recv_cb[cur_bus](channel,
            g_uart_lli_dma_cfg[cur_bus].rx_dma_config.transfer_num, ERRCODE_SUCC);
    }
    if (is_continue) {
        (void)uapi_uart_dma_lli_continue_recv(cur_bus);
    }
    return;
#endif
}

errcode_t uapi_uart_update_dma_recv_buff(uart_bus_t uart_bus, void *buffer, uint16_t length)
{
    if ((uart_bus >= UART_BUS_MAX_NUM) || (buffer == NULL)) {
        return ERRCODE_INVALID_PARAM;
    }
    uint32_t lock = osal_irq_lock();
    g_lli_rx_payload_buff[uart_bus] = ((uint8_t *)buffer);
    g_lli_rx_payload_buff_len[uart_bus] = length;
    osal_irq_restore(lock);
    return ERRCODE_SUCC;
}

errcode_t uapi_uart_dma_recv_register(uart_bus_t uart_bus, void *buffer, uint16_t length, uart_data_recv_cb_t cb)
{
    errcode_t ret = ERRCODE_FAIL;
    /* 接收head的DMA配置 */
    if (cb == NULL || uart_bus >= UART_BUS_MAX_NUMBER || buffer == NULL) {
        return ERRCODE_INVALID_PARAM;
    }

    uint32_t lock = osal_irq_lock();
    g_data_recv_cb[uart_bus] = cb;
    g_lli_rx_payload_buff_len[uart_bus] = length;
    g_lli_rx_payload_buff[uart_bus] = ((uint8_t *)buffer);
    osal_irq_restore(lock);
    uart_dma_lli_rx_head_config(uart_bus);
    /* 启动DMA以准备接收HEAD */
    ret = uapi_dma_configure_peripheral_transfer_lli(g_uart_lli_dma_cfg[uart_bus].rx_dma_channel,
                                                     &(g_uart_lli_dma_cfg[uart_bus].rx_dma_config), NULL);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    return uapi_dma_enable_lli(g_uart_lli_dma_cfg[uart_bus].rx_dma_channel, uart_dma_lli_recv_cb, (uintptr_t)buffer);
}

static bool msg_info_queue_is_empty(uart_bus_t uart_bus)
{
    if (g_msg_info_queue[uart_bus].head == g_msg_info_queue[uart_bus].tail) {
        return true;
    }
    return false;
}

static void uart_dma_lli_send_cb(uint8_t intr, uint8_t channel, uintptr_t arg)
{
    unused(arg);
    /* 获取当前channel对应的UART bus */
    uint8_t cur_bus = 0;
    while (cur_bus < UART_BUS_MAX_NUMBER) {
        if (g_uart_lli_dma_cfg[cur_bus].tx_dma_channel == channel) { break; }
        cur_bus++;
    }
    if (cur_bus >= UART_BUS_MAX_NUMBER) {
        return;
    }
    if (intr == 0) {
        uapi_dma_end_transfer(channel);
        g_uart_dma_status[cur_bus] = UART_IDLE;
        if (g_data_send_cb[cur_bus] != NULL) {
            g_data_send_cb[cur_bus]((uint8_t *)(uintptr_t)(g_uart_lli_dma_cfg[cur_bus].tx_dma_config.src),
                                    g_uart_lli_dma_cfg[cur_bus].tx_dma_config.transfer_num  <<
                                    g_uart_lli_dma_cfg[cur_bus].tx_dma_config.src_width,
                                    ERRCODE_SUCC);
        }
        if (!msg_info_queue_is_empty(cur_bus)) {
            uint8_t head = g_msg_info_queue[cur_bus].head;
            uapi_uart_dma_send(g_msg_info_queue[cur_bus].msg_buf[head].uart_bus,
                               g_msg_info_queue[cur_bus].msg_buf[head].buffer,
                               g_msg_info_queue[cur_bus].msg_buf[head].length,
                               g_msg_info_queue[cur_bus].msg_buf[head].cb);
            g_msg_info_queue[cur_bus].head = (head + 1) % CONFIG_UART_DMA_LLI_MAX_MEMBER;
        }
    }
}

errcode_t uapi_uart_dma_send(uart_bus_t uart_bus, const void *buffer, uint16_t length, uart_data_send_cb_t cb)
{
    errcode_t ret = ERRCODE_FAIL;
    if (uart_bus >= UART_BUS_MAX_NUMBER) { return ret; }
    uint32_t lock = osal_irq_lock();
    if (g_uart_dma_status[uart_bus] == UART_BUSY) {
        if (g_msg_info_queue[uart_bus].tail - g_msg_info_queue[uart_bus].head >= (CONFIG_UART_DMA_LLI_MAX_MEMBER - 1)) {
            osal_irq_restore(lock);
            return ERRCODE_UART_ADD_QUEUE_FAIL;
        }
        uint8_t tail = g_msg_info_queue[uart_bus].tail;
        g_msg_info_queue[uart_bus].msg_buf[tail].uart_bus = uart_bus;
        g_msg_info_queue[uart_bus].msg_buf[tail].buffer = buffer;
        g_msg_info_queue[uart_bus].msg_buf[tail].length = length;
        if (cb != NULL) { g_msg_info_queue[uart_bus].msg_buf[tail].cb = cb; }
        g_msg_info_queue[uart_bus].tail = (tail + 1) % CONFIG_UART_DMA_LLI_MAX_MEMBER;
        osal_irq_restore(lock);
        return ERRCODE_UART_DMA_TRANSFER_BUSY;
    }
    uart_dma_lli_tx_config(uart_bus);
#if defined(CONFIG_UART_TX_DMA_NEED_HEADER)
    g_uart_lli_header_buff[uart_bus].uart_msg_len = length;
    g_uart_lli_header_buff[uart_bus].uart_msg_start = UART_MSG_START;
    g_uart_lli_header_buff[uart_bus].uart_msg_type = UART_MSG_TYPE;
    /* Uart dma config. */
    ret = uapi_dma_configure_peripheral_transfer_lli(g_uart_lli_dma_cfg[uart_bus].tx_dma_channel,
                                                     &(g_uart_lli_dma_cfg[uart_bus].tx_dma_config), NULL);
    if (ret != ERRCODE_SUCC) {
        osal_irq_restore(lock);
        return ret;
    }
#endif
    if (cb != NULL) { g_data_send_cb[uart_bus] = cb; }
    g_uart_lli_dma_cfg[uart_bus].tx_dma_config.src = (uint32_t)(uintptr_t)buffer;
    g_uart_lli_dma_cfg[uart_bus].tx_dma_config.transfer_num =
        (uint16_t)(length >> g_uart_lli_dma_cfg[uart_bus].tx_dma_config.src_width);
    ret = uapi_dma_configure_peripheral_transfer_lli(g_uart_lli_dma_cfg[uart_bus].tx_dma_channel,
                                                     &(g_uart_lli_dma_cfg[uart_bus].tx_dma_config), NULL);
    if (ret != ERRCODE_SUCC) {
        osal_irq_restore(lock);
        return ret;
    }

    /* Enable uart dma linked list. */
    ret = uapi_dma_enable_lli(g_uart_lli_dma_cfg[uart_bus].tx_dma_channel, uart_dma_lli_send_cb, (uintptr_t)NULL);
    if (ret != ERRCODE_SUCC) {
        osal_irq_restore(lock);
        return ret;
    }
    g_uart_dma_status[uart_bus] = UART_BUSY;
    osal_irq_restore(lock);
    return ret;
}

errcode_t uapi_uart_dma_recv_end_transfer(uart_bus_t uart_bus)
{
    if (uart_bus >= UART_BUS_MAX_NUM) {
        return ERRCODE_INVALID_PARAM;
    }
    return uapi_dma_end_transfer(g_uart_lli_dma_cfg[uart_bus].rx_dma_channel);
}

errcode_t uapi_uart_dma_send_end_transfer(uart_bus_t uart_bus)
{
    if (uart_bus >= UART_BUS_MAX_NUM) {
        return ERRCODE_INVALID_PARAM;
    }
    return uapi_dma_end_transfer(g_uart_lli_dma_cfg[uart_bus].tx_dma_channel);
}

#if defined(CONFIG_UART_SUPPORT_RECV_RAW_DATA)
errcode_t uapi_uart_recv_raw_data_end_transfer(uart_bus_t uart_bus)
{
    if (uart_bus >= UART_BUS_MAX_NUM) {
        return ERRCODE_INVALID_PARAM;
    }
    return uapi_dma_end_transfer(g_rx_dma_channel);
}

static void uart_idle_rx_cb(const void *buffer, uint16_t length, bool error)
{
    unused(buffer);
    unused(error);
    uapi_dma_end_transfer(g_rx_dma_channel);
    uint32_t addr = uapi_reg_read_val32(DMA_BASE_ADDR + DMA_CH_CONFIG_OFFSET +
                    (DMA_RX_CHANNEL_OFFSET * g_rx_dma_channel));
    uint32_t len = addr - (uint32_t)g_uart_dma_receive_buff;

    uint32_t combin_length = len + (uint32_t)length;
    (void)memcpy_s(g_uart_dma_receive_buff + len, combin_length, g_uart_idle_receive_buff, length);

    bool is_continue = true;
    if (g_idle_recv_cb[g_uart_bus] != NULL) {
        is_continue = g_idle_recv_cb[g_uart_bus](g_uart_dma_receive_buff, combin_length);
    }

    if (is_continue) {
        dma_channel_t rx_dma_channel = (dma_channel_t)uapi_dma_get_lli_channel(HAL_DMA_BURST_TRANSACTION_LENGTH_16,
            uart_port_get_dma_trans_src_handshaking(g_uart_bus));
        g_rx_dma_channel = rx_dma_channel;
        uapi_dma_configure_peripheral_transfer_lli(rx_dma_channel, &g_rx_dma_config, NULL);
        uapi_dma_enable_lli(rx_dma_channel, NULL, (uintptr_t)g_uart_dma_receive_buff);
    } else {
        uapi_uart_unregister_rx_callback(g_uart_bus);
    }

    return;
}

static errcode_t uapi_uart_recv_raw_data_dma_config(uart_bus_t uart_bus)
{
    errcode_t ret = ERRCODE_FAIL;
    g_rx_dma_config.src = uart_porting_base_addr_get(uart_bus) + UART_DATA_FIFO_OFFSET_ADDR;
    g_rx_dma_config.trans_type = HAL_DMA_TRANS_PERIPHERAL_TO_MEMORY_DMA;
    g_rx_dma_config.trans_dir = HAL_DMA_TRANSFER_DIR_PERIPHERAL_TO_MEM;
    g_rx_dma_config.src_increment = HAL_DMA_ADDRESS_INC_NO_CHANGE;
    g_rx_dma_config.dest_increment = HAL_DMA_ADDRESS_INC_INCREMENT;
    g_rx_dma_config.src_width = HAL_DMA_TRANSFER_WIDTH_8;
    g_rx_dma_config.dest_width = HAL_DMA_TRANSFER_WIDTH_8;
    g_rx_dma_config.priority = HAL_DMA_CH_PRIORITY_0;
    g_rx_dma_config.burst_length = HAL_DMA_BURST_TRANSACTION_LENGTH_16;
    g_rx_dma_config.protection = HAL_DMA_PROTECTION_CONTROL_NONE;
    g_rx_dma_config.src_handshaking = uart_port_get_dma_trans_src_handshaking(uart_bus);
    g_rx_dma_config.dest_handshaking = 0;
    g_rx_dma_config.dest = (uint32_t)(uintptr_t)g_uart_dma_receive_buff;
    g_rx_dma_config.transfer_num = CONFIG_DMA_WAIT_UART_IDLE_INT_LEN;
    dma_channel_t rx_dma_channel = (dma_channel_t)uapi_dma_get_lli_channel(HAL_DMA_BURST_TRANSACTION_LENGTH_16,
        uart_port_get_dma_trans_src_handshaking(uart_bus));
    g_rx_dma_channel = rx_dma_channel;
    ret = uapi_dma_configure_peripheral_transfer_lli(rx_dma_channel, &g_rx_dma_config, NULL);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
    ret = uapi_dma_enable_lli(rx_dma_channel, uart_dma_lli_idle_recv_cb, (uintptr_t)g_uart_dma_receive_buff);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }
    return uapi_uart_register_rx_callback(uart_bus, UART_RX_CONDITION_MASK_IDLE,
                                          CONFIG_UART_IDLE_PAYLOAD_LEN, uart_idle_rx_cb);
}

errcode_t uapi_uart_dma_recv_raw_data(uart_bus_t uart_bus, uart_idle_int_receive_cb_t callback)
{
    errcode_t ret = ERRCODE_FAIL;

    uart_attr_t attr = {
        .baud_rate = CONFIG_UART_LLI_BAUDRATE,
        .data_bits = UART_DATA_BIT_8,
        .stop_bits = UART_STOP_BIT_1,
        .parity = UART_PARITY_NONE,
#if defined(CONFIG_UART_SUPPORT_FLOW_CTRL)
        .flow_ctrl = UART_FLOW_CTRL_RTS_CTS
#endif
    };

    uart_extra_attr_t extra_attr = {
        .tx_dma_enable = true,
        .tx_int_threshold = UART_FIFO_INT_TX_LEVEL_EQ_0_CHARACTER,
        .rx_dma_enable = true,
        .rx_int_threshold = CONFIG_UART_FIFO_INT_RX_LEVEL
    };

    uart_pin_config_t pins = { 0 };
    uapi_uart_deinit(uart_bus);
    ret = uapi_uart_init(uart_bus, &pins, &attr, &extra_attr, &g_app_uart_buffer_config);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    ret = uapi_uart_recv_raw_data_dma_config(uart_bus);
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    if (callback != NULL) {
        g_idle_recv_cb[uart_bus] = callback;
    }
    g_uart_bus = uart_bus;
    return ret;
}
#endif
