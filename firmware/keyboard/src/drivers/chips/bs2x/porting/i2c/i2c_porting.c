/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides i2c port template \n
 *
 * History: \n
 * 2022-08-15， Create file. \n
 */
#include "i2c_porting.h"
#include "hal_i2c_v151_comm.h"
#include "chip_core_irq.h"
#include "pm_clock.h"
#if defined(CONFIG_I2C_SUPPORT_QUERY_REGS)
#include "chip_io.h"
#endif
#include "arch_port.h"
#if defined(CONFIG_I2C_SUPPORT_DMA)
#include "dma_porting.h"
#endif

typedef void (*i2c_porting_irq_handler)(void);

typedef struct i2c_irq_handler {
    uint32_t irq_id;
    i2c_porting_irq_handler irq_handler;
} i2c_irq_handler_t;

uintptr_t g_i2c_base_addrs[I2C_BUS_MAX_NUM] = {
    (uintptr_t)I2C_BUS_0_BASE_ADDR,
#if I2C_BUS_MAX_NUMBER > 1
    (uintptr_t)I2C_BUS_1_BASE_ADDR,
#endif
};

uintptr_t i2c_port_base_addr_get(i2c_bus_t bus)
{
    return g_i2c_base_addrs[bus];
}

static void irq_i2c0_handler(void)
{
    hal_i2c_v151_irq_handler(I2C_BUS_0);
    osal_irq_clear(I2C_0_IRQN);
}

#if I2C_BUS_MAX_NUMBER > 1
static void irq_i2c1_handler(void)
{
    hal_i2c_v151_irq_handler(I2C_BUS_1);
    osal_irq_clear(I2C_1_IRQN);
}
#endif

static i2c_irq_handler_t g_i2c_irq_id[I2C_BUS_MAX_NUM] = {
    {
        I2C_0_IRQN,
        irq_i2c0_handler,
    },
#if I2C_BUS_MAX_NUMBER > 1
    {
        I2C_1_IRQN,
        irq_i2c1_handler,
    },
#endif
};

uint32_t i2c_port_get_clock_value(i2c_bus_t bus)
{
    if (bus >= I2C_BUS_MAX_NUM) {
        return 0;
    }
    return uapi_clock_crg_get_freq(CLOCK_CRG_ID_MCU_PERP_LS);
}

void i2c_port_register_irq(i2c_bus_t bus)
{
    i2c_irq_handler_t irq = g_i2c_irq_id[bus];
    osal_irq_request(irq.irq_id, (osal_irq_handler)irq.irq_handler, NULL, NULL, NULL);
    osal_irq_enable(irq.irq_id);
    osal_irq_set_priority(irq.irq_id, irq_prio(irq.irq_id));
}

void i2c_port_unregister_irq(i2c_bus_t bus)
{
    i2c_irq_handler_t irq = g_i2c_irq_id[bus];
    osal_irq_free(irq.irq_id, NULL);
}

uint32_t i2c_porting_lock(i2c_bus_t bus)
{
    unused(bus);
    return osal_irq_lock();
}

void i2c_porting_unlock(i2c_bus_t bus, uint32_t irq_sts)
{
    unused(bus);
    osal_irq_restore(irq_sts);
}

void i2c_port_clock_enable(i2c_bus_t bus, bool on)
{
    clock_control_type_t control_type;
    clock_mclken_aperp_type_t aperp_type;

    switch (bus) {
        case I2C_BUS_0:
            aperp_type = CLOCK_APERP_I2C0_CLKEN;
            break;

        case I2C_BUS_1:
            aperp_type = CLOCK_APERP_I2C1_CLKEN;
            break;

        default:
            return;
    }

    control_type = (on == true) ? CLOCK_CONTROL_MCLKEN_ENABLE : CLOCK_CONTROL_MCLKEN_DISABLE;
    uapi_clock_control(control_type, aperp_type);
}

#if defined(CONFIG_I2C_SUPPORT_QUERY_REGS)
#define I2C_ADDR_OFFEST_BYTES         4
void i2c_port_get_regs_value(i2c_bus_t bus)
{
    uint32_t count = 0;
    uintptr_t i2c_bus_addr = i2c_port_base_addr_get(bus);
    osal_printk("i2c base regs");
    for (uint32_t addr = i2c_bus_addr; addr <= i2c_bus_addr + 0xC4; addr += I2C_ADDR_OFFEST_BYTES) {
        if (count % I2C_ADDR_OFFEST_BYTES == 0) {
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

#if defined(CONFIG_I2C_SUPPORT_DMA) && (CONFIG_I2C_SUPPORT_DMA == 1)
uint8_t i2c_port_get_dma_trans_dest_handshaking(i2c_bus_t bus)
{
    switch (bus) {
        case I2C_BUS_0:
            return (uint8_t)DMA_HANDSHAKE_I2C_BUS_0_TX;
        case I2C_BUS_1:
            return (uint8_t)DMA_HANDSHAKE_I2C_BUS_1_TX;
        default:
            return (uint8_t)HAL_DMA_HANDSHAKING_MAX_NUM;
    }
}

uint8_t i2c_port_get_dma_trans_src_handshaking(i2c_bus_t bus)
{
    switch (bus) {
        case I2C_BUS_0:
            return (uint8_t)DMA_HANDSHAKE_I2C_BUS_0_RX;
        case I2C_BUS_1:
            return (uint8_t)DMA_HANDSHAKE_I2C_BUS_1_RX;
        default:
            return (uint8_t)HAL_DMA_HANDSHAKING_MAX_NUM;
    }
}
#endif