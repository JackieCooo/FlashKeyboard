/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: USB port for project
 *
 * Create:  2023-06-28
 */

#include "common_def.h"
#include "stdint.h"
#include "stdlib.h"
#include "osal_interrupt.h"
#include "usb_porting.h"
#ifndef BUILD_FLASHBOOT
#include "los_memory.h"
#include "gadget/f_hid.h"
#define USB_MEM_LEN  (450)
#else
#define USB_MEM_LEN  (1)
#endif

uint64_t g_usb_mem[USB_MEM_LEN];
uintptr_t g_usb_mem_addr_start  = (uintptr_t)&g_usb_mem;
unsigned long g_usb_mem_size  = sizeof(g_usb_mem);

uint32_t g_usb_ep0_buffer_size = CONFIG_DRIVERS_USB_DEVICE_CTRLREQ_BUF_SIZE;

usb_sof_cb g_sof_cb = NULL;


void usb_register_callback(usb_sof_cb sof_cb)
{
    uint32_t ret = osal_irq_lock();
    g_sof_cb = sof_cb;
    osal_irq_restore(ret);
}

void usb_unregister_callback(void)
{
    uint32_t ret = osal_irq_lock();
    g_sof_cb = NULL;
    osal_irq_restore(ret);
}

#ifndef BUILD_FLASHBOOT
void *usb_mem_alloc(uint32_t size, uint32_t boundary)
{
    return LOS_MemAllocAlign((void *)g_usb_mem_addr_start, size, boundary);
}

void usb_mem_free(void *ptr)
{
    LOS_MemFree((void *)g_usb_mem_addr_start, ptr);
}

void usb_sof_intr_callback(void)
{
    uint8_t *data = NULL;
    uint16_t len = 0;
    uint8_t device_index = 0;
    if (g_sof_cb == NULL) {
        return;
    }

    g_sof_cb(&data, &len, &device_index);
    // It will not send data if len == 0 or data == NULL
    fhid_send_data(device_index, (char *)data, len);
}
#else
void dma_cache_inv(uint32_t start, uint32_t end)
{
    unused(start);
    unused(end);
}

void dma_cache_clean(uint32_t start, uint32_t end)
{
    unused(start);
    unused(end);
}
#endif
