/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2024. All rights reserved.
 * Description:  massdata buffer module.
 *
 * Create:  2024-04-03
 */

#include "securec.h"
#include "non_os.h"
#include "panic.h"
#include "soc_osal.h"
#include "common_def.h"
#include "memory_config.h"
#include "massdata_buffer.h"

#define CONTOL_BLOCK_MASS_LENGTH        sizeof(massdata_memory_region_control_t)
/* Offsets from the start of the memory region */
#define BT_CORE_MASS_OFFSET             (MASSDATA_REGION_START + CONTOL_BLOCK_MASS_LENGTH)
#define APPLICATION_CORE_MASS_OFFSET    (BT_CORE_MASS_OFFSET + BT_MASSDATA_LENGTH)

#if CORE == BT
#define MASS_MEMORY_REGION_SECTION_CORE MASS_MEMORY_REGION_SECTION_0
#elif CORE == APPS
#if CORE_NUMS == 1
#define MASS_MEMORY_REGION_SECTION_CORE MASS_MEMORY_REGION_SECTION_0
#else
#define MASS_MEMORY_REGION_SECTION_CORE MASS_MEMORY_REGION_SECTION_1
#endif
#elif CORE == GNSS
#define MASS_MEMORY_REGION_SECTION_CORE MASS_MEMORY_REGION_SECTION_0
#elif CORE == SECURITY
#define MASS_MEMORY_REGION_SECTION_CORE MASS_MEMORY_REGION_SECTION_0
#elif CORE == CONTROL_CORE
#define MASS_MEMORY_REGION_SECTION_CORE MASS_MEMORY_REGION_SECTION_0
#elif CORE == SENSOR
#define MASS_MEMORY_REGION_SECTION_CORE MASS_MEMORY_REGION_SECTION_0
#endif

#if (BTH_WITH_SMART_WEAR == YES) && defined(SUPPORT_IPC)

#include "ipc.h"
#include "ipc_actions.h"

void uapi_massdata_record_system_error(uint8_t event_id, uint8_t info1, uint8_t info2, uint32_t info3)
{
#if (CORE == BT)
    ipc_payload_mass_data_type chr_info;
    chr_info.type = MASS_ERROR_POINT;
    chr_info.event_id = event_id;
    chr_info.info1 = info1;
    chr_info.info2 = info2;
    chr_info.info3 = (uint16_t)info3;

    (void)ipc_spin_send_message_timeout(CORES_APPS_CORE,
                                        IPC_ACTION_MASS_DATA_INFORM,
                                        (ipc_payload *)((void *)&chr_info),
                                        sizeof(chr_info),
                                        IPC_PRIORITY_LOWEST,
                                        false,
                                        0);
#else
    unused(event_id);
    unused(info1);
    unused(info2);
    unused(info3);
#endif
}

void uapi_massdata_record_system_event(uint8_t event_id, uint8_t info1, uint8_t info2, uint32_t info3)
{
#if (CORE == BT)
    ipc_payload_mass_data_type chr_info;
    chr_info.type = MASS_EVENT_POINT;
    chr_info.event_id = event_id;
    chr_info.info1 = info1;
    chr_info.info2 = info2;
    chr_info.info3 = (uint16_t)info3;

    (void)ipc_spin_send_message_timeout(CORES_APPS_CORE,
                                        IPC_ACTION_MASS_DATA_INFORM,
                                        (ipc_payload *)((void *)&chr_info),
                                        sizeof(chr_info),
                                        IPC_PRIORITY_LOWEST,
                                        false,
                                        0);
#else
    unused(event_id);
    unused(info1);
    unused(info2);
    unused(info3);
#endif
}

#else /* #if (BTH_WITH_SMART_WEAR == YES) && defined(SUPPORT_IPC) */

#include "log_trigger.h"
#include "systick.h"
#include "log_printf.h"

#if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES))

STATIC volatile massdata_memory_section_control_t *g_mass_section_control = NULL;
STATIC uint32_t g_mass_share_mem_size;
STATIC uint32_t g_mass_share_mem_start;
STATIC uint8_t g_role_state;
STATIC uint16_t g_mass_write_ue_psn = 0;
STATIC uint16_t g_mass_write_dft_psn = 0;
STATIC uint32_t g_version_number = 0;

#if defined(SUPPORT_IPC)
#include "ipc.h"
STATIC bool mass_receive_remote_info(ipc_action_t message, const volatile ipc_payload *payload,
                                     cores_t src, uint32_t id)
{
    unused(message);
    unused(src);
    unused(id);

    if (payload != NULL) {
        g_version_number = *(uint32_t *)(uintptr_t)payload;
    }
    return true;
}
#endif /* defined(SUPPORT_IPC) */

/**
 * @brief  trigger ipc to inform A core to save data from share mem in flash
 */
STATIC void massdata_trigger(void *pay_i, uint8_t core, uint8_t type)
{
#if ((CORE != APPS) && defined(SUPPORT_IPC))
    ipc_payload_mass_data_type ipc_pay;
    system_event_s_t *pay = (system_event_s_t *)(pay_i);
    ipc_pay.core = core;
    ipc_pay.type = type;
    ipc_pay.event_id = pay->event_id;
    ipc_pay.time_stamp = pay->time_stamp;
    ipc_pay.event_info = pay->event_info;
    ipc_pay.chr_up_type = pay->chr_up_type;
    ipc_pay.psn = pay->psn;
    ipc_pay.role = pay->role;

    (void)ipc_send_message(CORES_APPS_CORE,
                           IPC_ACTION_MASS_DATA_INFORM,
                           (ipc_payload *)((void *)&ipc_pay),
                           sizeof(ipc_payload_mass_data_type),
                           IPC_PRIORITY_LOWEST, false);
#else
    unused(pay_i);
    unused(core);
    unused(type);
#endif
}

/* eventEnum : (info0 << 16) | (info1 << 8) | info2 */
STATIC uint32_t massdata_get_info0_value(uint32_t event_enum)
{
    return ((event_enum >> CHR_INFO_SHFIT_16BIT) & 0xFF);
}

STATIC uint16_t massdata_get_event_id_value(uint32_t event_enum)
{
    return (uint16_t)((event_enum >> CHR_INFO_SHFIT_8BIT) & 0xFFFF);
}

STATIC uint8_t massdata_get_info2_value(uint32_t event_enum)
{
    return (uint8_t)(event_enum & 0xFF);
}

/* write mass data to share mem roll buffer, index by core type */
STATIC uint32_t massdata_write_roll_buffer_by_step(massdata_memory_section_control_t *buffer_info,
    const uint8_t *data, uint32_t length)
{
    uint32_t remain_size;
    uint32_t to_end;

    /* check the message length is as much the buffer size */
    if ((data == NULL) || (buffer_info == NULL)) {
        return MASS_RET_ERROR_IN_PARAMETERS;
    }

    uint32_t read = buffer_info->read;
    uint32_t write = buffer_info->write;
    uint32_t size = buffer_info->region_len;
    uint8_t *start = (uint8_t *)(uintptr_t)buffer_info->region_start;

    if ((size == 0) || (write > size) || (read > size)) {
        return MASS_RET_ERROR_IN_PARAMETERS;
    }

    to_end = size - write;
    remain_size = (read <= write) ? (size - (write - read)) : (read - write);

    if (remain_size < length) {
        return MASS_RET_ERROR_NOT_ENOUGH_SPACE;
    }

    if (length <= to_end) {
        if (memcpy_s((void *)((uintptr_t)(start + write)), to_end, data, length) != EOK) {
            return MASS_MEM_COPY_FAIL;
        }
    } else {
        if (memcpy_s((void *)((uintptr_t)(start + write)), to_end, data, to_end) != EOK) {
            return MASS_MEM_COPY_FAIL;
        }
        if (memcpy_s((void *)(uintptr_t)start, length - to_end, data + to_end, length - to_end) != EOK) {
            return MASS_MEM_COPY_FAIL;
        }
    }

    buffer_info->write = (write + length) % size;
    return MASS_RET_OK;
}

/**
 * @brief  write seg in the mass share mem.
 * @return MASS_RET_OK or an error value
 */
STATIC uint32_t massdata_write_roll_buffer(const uint8_t *head, uint32_t head_len,
    const uint8_t *data, uint32_t data_len)
{
    uint32_t remain_size;
    uint32_t ret;
    massdata_memory_section_control_t buffer_info = {0};

    /* check the message length is as much the buffer size */
    if ((head == NULL) || (data == NULL) || (g_mass_section_control == NULL)) {
        return MASS_RET_ERROR_IN_PARAMETERS;
    }

    uint32_t irq = osal_irq_lock();

    /* check if mem overflow */
    if ((g_mass_section_control->read > g_mass_share_mem_size) ||
        (g_mass_section_control->write > g_mass_share_mem_size) ||
        (g_mass_section_control->region_len > g_mass_share_mem_size) ||
        (g_mass_section_control->region_start != g_mass_share_mem_start)) {
        /* recorvy mem offset */
        g_mass_section_control->read = 0;
        g_mass_section_control->write = 0;
        g_mass_section_control->region_start = g_mass_share_mem_start;
        g_mass_section_control->region_len = g_mass_share_mem_size;
        g_mass_section_control->water_line = g_mass_share_mem_size / CHR_BUFFER_WL_RATIO;
        osal_irq_restore(irq);
        return MASS_RET_ERROR_CORRUPT_SHARED_MEMORY;
    }

    (void)memcpy_s((void *)&buffer_info, sizeof(buffer_info), (void *)g_mass_section_control, sizeof(buffer_info));

    if (buffer_info.read <= buffer_info.write) {
        remain_size = buffer_info.region_len - (buffer_info.write - buffer_info.read);
    } else {
        remain_size = buffer_info.read - buffer_info.write;
    }

    if (remain_size < (head_len + data_len)) {
        ret = MASS_RET_ERROR_NOT_ENOUGH_SPACE;
        goto err;
    }

    ret = massdata_write_roll_buffer_by_step(&buffer_info, head, head_len);
    if (ret != MASS_RET_OK) {
        goto err;
    }

    ret = massdata_write_roll_buffer_by_step(&buffer_info, data, data_len);
    if (ret != MASS_RET_OK) {
        goto err;
    }

    g_mass_section_control->write = buffer_info.write;
    osal_irq_restore(irq);

    remain_size -= (head_len + data_len);

    if (remain_size < buffer_info.water_line) { return MASS_OVER_BUFFER_THD; }
    return MASS_RET_OK;
err:
    osal_irq_restore(irq);
    return ret;
}
#endif /* #if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES)) */

void uapi_massdata_record_ue_event(uint8_t eid, uint8_t sub_eid, uint8_t code, uint32_t sub_code)
{
    uint32_t event_enum = ((eid << CHR_INFO_SHFIT_16BIT) | (sub_eid << CHR_INFO_SHFIT_8BIT) | code);
    return uapi_massdata_record_long_event(event_enum, (uint8_t*)&sub_code, sizeof(uint32_t));
}

void uapi_massdata_record_system_error(uint8_t event_id, uint8_t info1, uint8_t info2, uint32_t info3)
{
    uint32_t event_enum = ((event_id << CHR_INFO_SHFIT_16BIT) | (info1 << CHR_INFO_SHFIT_8BIT) | info2);
    return uapi_massdata_record_long_event(event_enum, (uint8_t*)&info3, sizeof(uint32_t));
}

void uapi_massdata_record_system_event(uint8_t event_id, uint8_t info1, uint8_t info2, uint32_t info3)
{
    uint32_t event_enum = ((event_id << CHR_INFO_SHFIT_16BIT) | (info1 << CHR_INFO_SHFIT_8BIT) | info2);
    return uapi_massdata_record_long_event(event_enum, (uint8_t*)&info3, sizeof(uint32_t));
}

#endif /* #if (BTH_WITH_SMART_WEAR == YES) && defined(SUPPORT_IPC) */

void uapi_massdata_buffer_init(mass_data_memory_region_section_t sec)
{
    unused(sec);
#if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES))
#if CORE_NUMS > 1
    if (sec != MASS_MEMORY_REGION_SECTION_0 && sec != MASS_MEMORY_REGION_SECTION_1) {
        return;
    }
#else
    if (sec != MASS_MEMORY_REGION_SECTION_0) {
        return;
    }
#endif
    massdata_memory_region_control_t *mass_ctrl = (massdata_memory_region_control_t *)(uintptr_t)MASSDATA_REGION_START;
    g_mass_section_control = &(mass_ctrl->section_control[sec]);

    g_mass_share_mem_start = g_mass_section_control->region_start;
    g_mass_share_mem_size = g_mass_section_control->region_len;
    g_mass_section_control->water_line = g_mass_share_mem_size / CHR_BUFFER_WL_RATIO;
#if (BTH_WITH_SMART_WEAR == NO)
    ipc_register_handler(IPC_ACTION_MASS_DATA_INFORM, mass_receive_remote_info);
#endif /* (BTH_WITH_SMART_WEAR == NO) */
#endif /* #if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES)) */
}

void uapi_massdata_memory_region_init(void)
{
#if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES))
    memset_s((void *)(uintptr_t)MASSDATA_REGION_START, MASSDATA_REGION_LENGTH, 0, MASSDATA_REGION_LENGTH);
    massdata_memory_region_control_t *mass_ctrl = (massdata_memory_region_control_t *)(uintptr_t)MASSDATA_REGION_START;

    mass_ctrl->region_num = MASS_MEMORY_REGION_MAX_NUMBER;
    mass_ctrl->mem_len = MASSDATA_REGION_LENGTH;
    mass_ctrl->section_control[MASS_MEMORY_REGION_SECTION_0].region_start = BT_CORE_MASS_OFFSET;
    mass_ctrl->section_control[MASS_MEMORY_REGION_SECTION_0].region_len = BT_MASSDATA_LENGTH;
#if CORE_NUMS > 1
    mass_ctrl->section_control[MASS_MEMORY_REGION_SECTION_1].region_start = APPLICATION_CORE_MASS_OFFSET;
    mass_ctrl->section_control[MASS_MEMORY_REGION_SECTION_1].region_len = APP_MASSDATA_LENGTH -
                                                                          CONTOL_BLOCK_MASS_LENGTH;
#endif /* CORE_NUMS > 1 */
#endif /* #if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES)) */
    return;
}

void uapi_massdata_set_role(uint8_t role)
{
    unused(role);
#if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES))
    g_role_state = role;
#endif
}

void uapi_massdata_record_long_event(uint32_t event_enum, uint8_t *info3, uint8_t info3_len)
{
#if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES))
#define MS_PER_SECOND 1000
    uint32_t ret;
    uint16_t psn;
    uint8_t chr_type = CHR_UE_JSON;
    uint32_t irq;

    if (info3 == NULL || info3_len > MAX_INFO3_DATA_SIZE) {
        return;
    }

    if (massdata_get_info0_value(event_enum) >= EVENT_DFT_START) {
        chr_type = CHR_DFT;
        irq = osal_irq_lock();
        psn = g_mass_write_dft_psn++;
        osal_irq_restore(irq);
    } else {
        irq = osal_irq_lock();
        psn = g_mass_write_ue_psn++;
        osal_irq_restore(irq);
    }

    /* fill the mass packet header */
    system_event_s_t event_seg;
    event_seg.head_flag = MASSDATA_HEAD_MAGIC;
    event_seg.time_stamp = (uint32_t)(uapi_systick_get_s() + get_chr_basegmt_s());
    event_seg.data_len = info3_len;
    event_seg.event_id = massdata_get_event_id_value(event_enum);
    event_seg.event_info = massdata_get_info2_value(event_enum);
    event_seg.chr_up_type = chr_type;
    event_seg.version = g_version_number;
    event_seg.core = CORE;
    event_seg.reserved = 0;
    event_seg.psn = psn;
    event_seg.role = g_role_state;

    ret = massdata_write_roll_buffer((const uint8_t*)&event_seg, sizeof(system_event_s_t), info3, info3_len);
    if ((ret == MASS_OVER_BUFFER_THD || ret == MASS_RET_ERROR_NOT_ENOUGH_SPACE)) {
        massdata_trigger((void *)&event_seg, CORE, MASS_EVENT_POINT);
    }
#undef MS_PER_SECOND
#else /* #if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES)) */
    unused(event_enum);
    unused(info3);
    unused(info3_len);
#endif /* #if (defined(ENABLE_MASSDATA_RECORD) && (ENABLE_MASSDATA_RECORD == YES)) */
    return;
}

void log_buffer_record_system_event(uint32_t chr_event)
{
    if (chr_event == CHR_VALUE_NULL) {
        return;
    }

    uint8_t event_id;
    uint8_t info1;
    uint8_t info2;
    uint8_t info3;

    event_id = (chr_event >> CHR_INFO_SHFIT_24BIT) & CHR_VALUE_MASK;
    info1 = (chr_event >> CHR_INFO_SHFIT_16BIT) & CHR_VALUE_MASK;
    info2 = (chr_event >> CHR_INFO_SHFIT_8BIT) & CHR_VALUE_MASK;
    info3 = chr_event & CHR_VALUE_MASK;

    uapi_massdata_record_system_event(event_id, info1, info2, info3);
}

void log_buffer_record_system_error(uint32_t chr_error)
{
    if (chr_error == CHR_VALUE_NULL) {
        return;
    }

    uint8_t error_id;
    uint8_t info1;
    uint8_t info2;
    uint8_t info3;

    error_id = (chr_error >> CHR_INFO_SHFIT_24BIT) & CHR_VALUE_MASK;
    info1 = (chr_error >> CHR_INFO_SHFIT_16BIT) & CHR_VALUE_MASK;
    info2 = (chr_error >> CHR_INFO_SHFIT_8BIT) & CHR_VALUE_MASK;
    info3 = chr_error & CHR_VALUE_MASK;

    uapi_massdata_record_system_error(error_id, info1, info2, info3);
}

void massdata_init(void)
{
    return uapi_massdata_buffer_init(MASS_MEMORY_REGION_SECTION_CORE);
}

/* 暂时兼容旧接口 */
void massdata_record_system_error(uint8_t event_id, uint8_t info1, uint8_t info2, uint32_t info3)
{
    return uapi_massdata_record_system_error(event_id, info1, info2, info3);
}

void massdata_record_system_event(uint8_t event_id, uint8_t info1, uint8_t info2, uint32_t info3)
{
    return uapi_massdata_record_system_event(event_id, info1, info2, info3);
}
