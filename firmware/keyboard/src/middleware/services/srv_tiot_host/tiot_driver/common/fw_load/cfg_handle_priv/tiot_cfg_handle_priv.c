/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description:  \n
 *
 * History: \n
 * 2023-12-05, Create file. \n
 */

#include "tiot_firmware.h"
#include "tiot_firmware_utils.h"
#include "tiot_board_log.h"
#include "tiot_cfg_handle_priv.h"

#define TIOT_FW_FILE_STA        0x43
#define TIOT_FW_FILE_ACK        0x06
#define TIOT_FW_FILE_NAK        0x15
#define TIOT_FW_PRIV_CMD_OK     'G'

#define TIOT_FW_PRIV_ACK_MAX    128

typedef enum {
    TIOT_FW_EXTCMD_ACK = 0x0,
    TIOT_FW_EXTCMD_ACK_EXT = 0x1,
    TIOT_FW_EXTCMD_NUM
} tiot_fw_ext_cmd_priv;

typedef int32_t (*priv_ext_cmd_handle_func)(tiot_fw *fw, const uint8_t *cmd, uint16_t len);

static int32_t tiot_fw_ack_handle_priv(tiot_fw *fw, const uint8_t *cmd, uint16_t len);
static int32_t tiot_fw_ack_ext_handle_priv(tiot_fw *fw, const uint8_t *cmd, uint16_t len);

static const priv_ext_cmd_handle_func tiot_fw_priv_ext_cmd_table[TIOT_FW_EXTCMD_NUM] = {
    tiot_fw_ack_handle_priv,
    tiot_fw_ack_ext_handle_priv
};

static int32_t tiot_fw_ack_handle_priv(tiot_fw *fw, const uint8_t *cmd, uint16_t len)
{
    int32_t ret = 0;
    uint8_t ack = 0;
    const uint8_t quit_cmd[] = "QUIT ";
    // QUIT命令特殊处理
    if ((len == (sizeof(quit_cmd) - 1)) && (memcmp(quit_cmd, cmd, len) == 0)) {
        return 0;
    }
    ret = tiot_firmware_read(fw, &ack, sizeof(uint8_t));
    if (ret != sizeof(uint8_t)) {
        return -1;
    }
    tiot_print_dbg("[TIoT:priv]ack 0x%x", ack);
    if ((ack == TIOT_FW_PRIV_CMD_OK) || (ack == TIOT_FW_FILE_STA) || (ack == TIOT_FW_FILE_ACK)) {
        return TIOT_FW_LOAD_RET_OK;
    } else if (ack == TIOT_FW_FILE_NAK) {
        return TIOT_FW_LOAD_RET_RETRY;
    }
    return TIOT_FW_LOAD_RET_ERR;
}

static int32_t tiot_fw_ack_ext_handle_priv(tiot_fw *fw, const uint8_t *cmd, uint16_t len)
{
    tiot_unused(cmd);
    tiot_unused(len);
    int32_t ret = 0;
    uint8_t ack_buff[TIOT_FW_PRIV_ACK_MAX] = { 0 };
    ret = tiot_firmware_read(fw, ack_buff, sizeof(ack_buff));
    if ((ret <= 0) || ((uint32_t)ret > sizeof(ack_buff))) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    tiot_print_info("[TIoT:fw]ack ext: ");
    for (uint32_t i = 0; i < (uint32_t)ret; i++) {
        tiot_print_info("0x%x ", ack_buff[i]);
    }
    tiot_print_info("\r\n");
    return TIOT_FW_LOAD_RET_OK;
}

int32_t tiot_fw_ext_cmd_handle_priv(tiot_fw *fw, uint16_t ext_cmd, const uint8_t *cmd, uint16_t len)
{
    if (ext_cmd >= TIOT_FW_EXTCMD_NUM) {
        tiot_print_err("[TIoT:fw]priv ext cmd exceed max!\r\n");
        return -1;
    }
    return tiot_fw_priv_ext_cmd_table[ext_cmd](fw, cmd, len);
}