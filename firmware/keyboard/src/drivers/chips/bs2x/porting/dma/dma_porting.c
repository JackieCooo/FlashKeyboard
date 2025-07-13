/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides dma port \n
 *
 * History: \n
 * 2022-10-16， Create file. \n
 */
#include "chip_io.h"
#include "interrupt/osal_interrupt.h"
#include "platform_core.h"
#include "chip_core_irq.h"
#include "hal_dmac_v151.h"
#include "arch_port.h"
#include "dma_porting.h"
#if defined(CONFIG_DMA_SUPPORT_QUERY_REGS)
#include "osal_debug.h"
#endif

uintptr_t g_dma_base_addr =   (uintptr_t)DMA_BASE_ADDR;
uint8_t chn_sreq_info[DMA_CHANNEL_MAX_NUM];
uint8_t chn_dreq_info[DMA_CHANNEL_MAX_NUM];

static uint32_t g_dma_handshaking_channel_status = 0;

static void irq_dma_handler(void)
{
    hal_dma_v151_irq_handler();
    osal_irq_clear(M_SDMA_IRQN);
}

void dma_port_register_irq(void)
{
    osal_irq_request(M_SDMA_IRQN, (osal_irq_handler)irq_dma_handler, NULL, NULL, NULL);
    osal_irq_enable(M_SDMA_IRQN);
    osal_irq_set_priority(M_SDMA_IRQN, irq_prio(M_SDMA_IRQN));
}

void dma_port_unregister_irq(void)
{
    osal_irq_disable(M_SDMA_IRQN);
    osal_irq_free(M_SDMA_IRQN, NULL);
}

static hal_dma_mux_handshaking_status_t dma_port_get_handshaking_channel_status(hal_dma_handshaking_source_t channel)
{
    uint32_t status = 0;
    if (channel >= HAL_DMA_HANDSHAKING_MAX_NUM) {
        return HAL_DMA_MUX_HANDSHAKING_USING;
    }

    status = g_dma_handshaking_channel_status & (1UL<<channel);
    if (status != 0) {
        return HAL_DMA_MUX_HANDSHAKING_USING;
    } else {
        return HAL_DMA_MUX_HANDSHAKING_IDLE;
    }
}

/* 记录每个soure的状态, 0表示空闲， 1表示busy */
void dma_port_set_handshaking_channel_status(hal_dma_handshaking_source_t channel, bool on)
{
    if (channel >= HAL_DMA_HANDSHAKING_MAX_NUM) {
        return;
    }
    if (on) {
        g_dma_handshaking_channel_status |= (1UL << channel);
    } else {
        g_dma_handshaking_channel_status &= (uint32_t)(~(1UL << channel));
    }
}

void dma_port_set_handshaking_source(dma_channel_t ch, hal_dma_handshaking_source_t source,
    hal_dma_handshaking_source_t dest)
{
    if (ch >= DMA_CHANNEL_MAX_NUM) {
        return;
    }
    chn_sreq_info[ch] = source;
    chn_dreq_info[ch] = dest;
}

void dma_port_release_handshaking_source(dma_channel_t ch)
{
    dma_port_set_handshaking_channel_status(chn_sreq_info[ch], false);
    dma_port_set_handshaking_channel_status(chn_dreq_info[ch], false);
    chn_sreq_info[ch] = 0;
    chn_sreq_info[ch] = 0;
}

errcode_t dma_port_set_mux_channel(dma_channel_t ch, hal_dma_transfer_peri_config_t *per_cfg)
{
    if ((dma_port_get_handshaking_channel_status(per_cfg->hs_source) == HAL_DMA_MUX_HANDSHAKING_USING) &&
        (dma_port_get_handshaking_channel_status(per_cfg->hs_dest) == HAL_DMA_MUX_HANDSHAKING_USING)) {
        return ERRCODE_DMA_RET_HANDSHAKING_USING;
    }

    dma_port_set_handshaking_channel_status(per_cfg->hs_source, true);
    dma_port_set_handshaking_channel_status(per_cfg->hs_dest, true);
    dma_port_set_handshaking_source(ch, per_cfg->hs_source, per_cfg->hs_dest);
    return ERRCODE_SUCC;
}

hal_dma_master_select_t dma_port_get_master_select(dma_channel_t ch, uint32_t addr)
{
    unused(ch);
    unused(addr);
    return HAL_DMA_MASTER_SELECT_0;
}

void dma_port_add_sleep_veto(void)
{
    return;
}

void dma_port_remove_sleep_veto(void)
{
    return;
}

void dma_port_clock_enable(void)
{
    reg16_setbit(0x520003E0, 0x4);
}

void dma_port_clock_disable(void)
{
    reg16_clrbit(0x520003E0, 0x4);
}

#if defined(CONFIG_DMA_SUPPORT_QUERY_REGS)
#define DMA_ADDR_OFFEST_BYTES     4
void dma_port_get_regs_value(void)
{
    uint32_t count = 0;
    osal_printk("dma base regs");
    for (uint32_t addr = 0x52070004; addr <= 0x52070020; addr += DMA_ADDR_OFFEST_BYTES) {
        if (count % DMA_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    count = 0;
    osal_printk("\r\ndma channel regs");
    for (uint32_t addr = 0x52070100; addr <= 0x520701F4; addr += DMA_ADDR_OFFEST_BYTES) {
        if (count % DMA_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    osal_printk("\r\n");
}
#endif
