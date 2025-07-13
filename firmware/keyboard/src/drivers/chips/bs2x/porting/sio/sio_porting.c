/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: I2S port for project
 *
 * Create: 2023-03-10
 */
#include "soc_osal.h"
#include "pinctrl.h"
#include "pm_clock.h"
#include "chip_core_irq.h"
#include "hal_sio_v151.h"
#include "arch_port.h"
#include "sio_porting.h"
#if defined(CONFIG_I2S_SUPPORT_DMA)
#include "dma_porting.h"
#endif
#if defined(CONFIG_I2S_SUPPORT_QUERY_REGS)
#include "osal_debug.h"
#endif

static uintptr_t g_sio_base_addrs[I2S_MAX_NUMBER] = {
    (uintptr_t)I2S_BUS_0_BASE_ADDR,
};

typedef struct sio_interrupt {
    core_irq_t irq_num;
    osal_irq_handler irq_func;
}sio_interrupt_t;

static const sio_interrupt_t g_sio_interrupt_lines[I2S_MAX_NUMBER] = {
    { I2S_IRQN, (osal_irq_handler)irq_sio0_handler },
};

uintptr_t sio_porting_base_addr_get(sio_bus_t bus)
{
    return g_sio_base_addrs[bus];
}

void sio_porting_register_hal_funcs(sio_bus_t bus)
{
    hal_sio_register_funcs(bus, hal_sio_v151_funcs_get());
}

void sio_porting_unregister_hal_funcs(sio_bus_t bus)
{
    hal_sio_unregister_funcs(bus);
}

void sio_porting_register_irq(sio_bus_t bus)
{
    osal_irq_request(g_sio_interrupt_lines[bus].irq_num, g_sio_interrupt_lines[bus].irq_func, NULL, NULL, NULL);
    osal_irq_enable(g_sio_interrupt_lines[bus].irq_num);
    osal_irq_set_priority(g_sio_interrupt_lines[bus].irq_num, g_sio_interrupt_lines[bus].irq_num);
}

void sio_porting_unregister_irq(sio_bus_t bus)
{
    osal_irq_disable(g_sio_interrupt_lines[bus].irq_num);
}

void irq_sio0_handler(void)
{
    hal_sio_v151_irq_handler(SIO_BUS_0);
    osal_irq_clear(I2S_IRQN);
}

void sio_porting_clock_enable(bool enable)
{
    if (enable) {
        uapi_clock_crg_config(CLOCK_CRG_ID_I2S, CLOCK_CLK_SRC_TCXO, 0x8);
        uapi_reg_setbit(M_CTL_RB_BASE + M_CLKEN0, I2S_CLKEN_BIT);
    } else {
        uapi_reg_clrbit(M_CTL_RB_BASE + M_CLKEN0, I2S_CLKEN_BIT);
        uapi_clock_crg_config(CLOCK_CRG_ID_I2S, CLOCK_CLK_SRC_NONE, 0x8);
    }
}

uint32_t sio_porting_get_bclk_div_num(uint8_t data_width, uint32_t ch)
{
    uint32_t mclk_div_num, s_clk, bclk_div_num, freq_of_need;
    float middle_div;
    s_clk = I2S_S_CLK;
    freq_of_need = FREQ_OF_NEED;
    mclk_div_num = uapi_reg_getbits(M_CTL_RB_BASE + I2S_CR, I2S_CR_DIV_NUM_BIT, I2S_CR_DIV_NUM_LEN);
    if ((mclk_div_num == 0) || (mclk_div_num == 1) || (mclk_div_num == I2S_MCLK_DIV_NUM)) {
        mclk_div_num = I2S_MCLK_DIV;
    }
    middle_div = (s_clk / mclk_div_num) / (freq_of_need * data_width * ch);
    if ((uint32_t)(middle_div * I2S_PARAM + 1) == ((uint32_t)middle_div * I2S_PARAM + 1)) {  /* 对float类型向上取整。 */
        bclk_div_num = (uint32_t)middle_div + 1;
    } else {
        bclk_div_num = (uint32_t)middle_div;
    }
    return bclk_div_num;
}

void sio_porting_i2s_pinmux(void)
{
#if defined(CONFIG_DRIVER_SUPPORT_I2S)
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    uapi_pin_set_ie(CONFIG_I2S_DI_PIN, PIN_IE_1);
    uapi_pin_set_ie(CONFIG_I2S_SCLK_PIN, PIN_IE_1);
    uapi_pin_set_ie(CONFIG_I2S_WS_PIN, PIN_IE_1);
#endif /* CONFIG_PINCTRL_SUPPORT_IE */
    uapi_pin_set_mode(CONFIG_I2S_SCLK_PIN, CONFIG_I2S_SCLK_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2S_WS_PIN, CONFIG_I2S_WS_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2S_DO_PIN, CONFIG_I2S_DO_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2S_DI_PIN, CONFIG_I2S_DI_PIN_MODE);
#endif
}

#if defined(CONFIG_I2S_SUPPORT_QUERY_REGS)
#define SIO_ADDR_OFFEST_BYTES     4
void sio_port_get_regs_value(sio_bus_t bus)
{
    uint32_t count = 0;
    uintptr_t sio_bus_addr = sio_porting_base_addr_get(bus);
    osal_printk("sio base regs");
    for (uint32_t addr = sio_bus_addr; addr <= sio_bus_addr + 0xC0; addr += SIO_ADDR_OFFEST_BYTES) {
        if (count % SIO_ADDR_OFFEST_BYTES == 0) {
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

#if defined(CONFIG_I2S_SUPPORT_DMA)
#define I2S_BUS_0_REAL_BASE             (I2S_BUS_0_BASE_ADDR - 0x3c)
#define I2S_MERGE_TX_DATA_ADDR          (I2S_BUS_0_REAL_BASE + 0xc0)
#define I2S_MERGE_RX_DATA_ADDR          (I2S_BUS_0_REAL_BASE + 0xa0)
#define I2S_TX_SPLIT_LEFT_DATA_ADDR     (I2S_BUS_0_REAL_BASE + 0x4c)
#define I2S_TX_SPLIT_RIGHT_DATA_ADDR    (I2S_BUS_0_REAL_BASE + 0x50)
#define I2S_RX_SPLIT_LEFT_DATA_ADDR     (I2S_BUS_0_REAL_BASE + 0x54)
#define I2S_RX_SPLIT_RIGHT_DATA_ADDR    (I2S_BUS_0_REAL_BASE + 0x58)
uint32_t i2s_port_get_dma_trans_src_handshaking(sio_bus_t bus)
{
    unused(bus);
    return HAL_DMA_HANDSHAKING_I2S_RX;
}

uint32_t i2s_port_get_dma_trans_dest_handshaking(sio_bus_t bus)
{
    unused(bus);
    return HAL_DMA_HANDSHAKING_I2S_TX;
}

uintptr_t i2s_porting_tx_merge_data_addr_get(sio_bus_t bus)
{
    unused(bus);
    return I2S_MERGE_TX_DATA_ADDR;
}

uintptr_t i2s_porting_rx_merge_data_addr_get(sio_bus_t bus)
{
    unused(bus);
    return I2S_MERGE_RX_DATA_ADDR;
}
#endif
