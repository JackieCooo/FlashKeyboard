/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides PDM port for SUSONG \n
 *
 * History: \n
 * 2023-01-31, Create file. \n
 */
#include "hal_pdm_v150.h"
#include "pdm_porting.h"
#if defined(CONFIG_PDM_SUPPORT_QUERY_REGS)
#include "chip_io.h"
#include "osal_debug.h"
#endif

#define PDM_BASE_ADDR 0x5208E000

uintptr_t pdm_porting_base_addr_get(void)
{
    return (uintptr_t)PDM_BASE_ADDR;
}

void pdm_port_clock_enable(bool en)
{
    if (en) {
        uapi_reg_clrbit(0x5200004c, 0x1);   // bit1: pdm_clk_sel xo
        uapi_reg_setbit(0x52000548, 10);    // bit10: glb_clken_pdm
    } else {
        uapi_reg_clrbit(0x52000548, 10);    // bit10: glb_clken_pdm
        uapi_reg_setbit(0x5200004c, 0x1);   // bit1: pdm_clk_sel rc
    }
}

void pdm_port_register_hal_funcs(void)
{
    hal_pdm_register_funcs(hal_pdm_v150_funcs_get());
}

void pdm_port_unregister_hal_funcs(void)
{
    hal_pdm_unregister_funcs();
}

#if defined(CONFIG_PDM_SUPPORT_QUERY_REGS)
#define PDM_ADDR_OFFEST_BYTES     4
void pdm_port_get_regs_value(void)
{
    uint32_t count = 0;
    osal_printk("pdm base regs");
    for (uint32_t addr = PDM_BASE_ADDR; addr <= PDM_BASE_ADDR + 0x30; addr += PDM_ADDR_OFFEST_BYTES) {
        if (count % PDM_ADDR_OFFEST_BYTES == 0) {
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