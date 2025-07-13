/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides spi port UT \n
 *
 * History: \n
 * 2022-08-18， Create file. \n
 */
#include "hal_spi_v151_regs_op.h"
#include "hal_spi_v151.h"
#include "chip_io.h"
#include "osal_interrupt.h"
#include "arch_port.h"
#include "chip_core_irq.h"
#include "spi_porting.h"
#if defined(CONFIG_SPI_SUPPORT_LPC)
#include "pm_clock.h"
#endif
#if defined(CONFIG_SPI_SUPPORT_QUERY_REGS)
#include "osal_debug.h"
#endif

/** -----------------------------------------------------
 *          Reg Bit fild Max and Shift Value
 * ---------------------------------------------------- */
 /**
  * @brief  CTRLR0 : Control register 0.
  */
#define HAL_SPI_CTRLR0_REG_MAX       0xFFFFFFFF
#define HAL_SPI_CE_LIN_TOGGLE_ENABLE (BIT(24))

/**
 * @brief  FRF : Frame format.
 */
#define HAL_SPI_FRAME_FORMAT_MAX   0x03
#define HAL_SPI_FRAME_FORMAT_SHIFT 0x15

/**
 * @brief  DFS : Data frame size.
 */
#define HAL_SPI_FRAME_SIZE_MAX   0x1F
#define HAL_SPI_FRAME_SIZE_SHIFT 0x10
#define HAL_SPI_FRAME_SIZE_8     0x07
#define HAL_SPI_FRAME_SIZE_16    0x0F
#define HAL_SPI_FRAME_SIZE_32    0x1F

/**
 * @brief  TMOD : Trans mode.
 */
#define HAL_SPI_TRANS_MODE_MAX    0x03
#define HAL_SPI_TRANS_MODE_SHIFT  0x08
#define HAL_SPI_TRANS_MODE_TXRX   0x00
#define HAL_SPI_TRANS_MODE_TX     0x01
#define HAL_SPI_TRANS_MODE_RX     0x02
#define HAL_SPI_TRANS_MODE_EEPROM 0x03

/**
 * @brief  CLOCK : SCPOL and SCPH.
 */
#define HAL_SPI_CLKS_MODE_MAX   0x03
#define HAL_SPI_CLKS_MODE_SHIFT 0x06

/**
 * @brief  CTRLR1 : Control register 1.
 */
#define HAL_SPI_RECEIVED_DATA_REG_MAX 0xFFFF

/**
 * @brief  SSIENR : SSI enable register.
 */
#define HAL_SPI_ENABLE 0x01

/**
 * @brief  SER : Slave enable register.
 */
#define HAL_SPI_SLAVE_ENABLE_REG_MAX 0xFFFFFFFF

/**
 * @brief  BAUDR : Baud rate select.
 */
#define HAL_SPI_CLK_DIV_REG_MAX 0xFFFF

/**
 * @brief  SR : Status register.
 */
#define HAL_SPI_RX_FIFO_FULL_FLAG      (BIT(4))
#define HAL_SPI_RX_FIFO_NOT_EMPTY_FLAG (BIT(3))
#define HAL_SPI_TX_FIFO_EMPTY_FLAG     (BIT(2))
#define HAL_SPI_TX_FIFO_NOT_FULL_FLAG  (BIT(1))
#define HAL_SPI_BUSY_FLAG              (BIT(0))

/**
 * @brief IMR : Interrupt mask register.
 */
#define HAL_SPI_INTERRUPT_REG_MAX 0x3F


/**
 * @brief  SPI_CTRLR0:  SPI control register.
 */
#define HAL_QSPI_CTRLR0_REG_MAX 0xFFFFFFFF

/**
 * @brief  Wait cycles.
 */
#define HAL_QSPI_WAIT_CYCLE_MAX   0x1F
#define HAL_QSPI_WAIT_CYCLE_SHIFT 0x0B
#define HAL_QSPI_WAIT_CYCLE_6     0x06
#define HAL_QSPI_WAIT_CYCLE_4     0x04
#define HAL_QSPI_WAIT_CYCLE_2     0x02

#define HAL_SPI_RX_SAMPLE_DLY_MAX   0xFF
#define HAL_SPI_RX_SAMPLE_DLY_SHIFT 0

/**
 * @brief  Command length.
 */
#define HAL_QSPI_CMD_LENTH_MAX   0x03
#define HAL_QSPI_CMD_LENTH_SHIFT 0x08
#define HAL_QSPI_CMD_LENTH_8     0x02

/**
 * @brief  Address length.
 */
#define HAL_QSPI_ADDR_LENTH_MAX   0x0F
#define HAL_QSPI_ADDR_LENTH_SHIFT 0x02
#define HAL_QSPI_ADDR_LENTH_24    0x06

/**
 * @brief  Trans type.
 */
#define HAL_QSPI_TRANS_TYPES_MAX          0x03
#define HAL_QSPI_TRANS_TYPES_SHIFT        0x00
#define HAL_QSPI_TRANS_TYPES_CMD_S_ADDR_Q 0x01

#define HAL_SPI_BUS_MAX_NUM (3)

#define HAL_SPI_DR_REG_SIZE 36

/** -----------------------------------------------------
 *          SPI Register Address
 * ----------------------------------------------------
 */
#define HAL_SPI_BUS_1_DEVICE_MODE_SET_BIT       0x00
#define HAL_SPI_BUS_1_DEVICE_MODE_SET_BIT_MAX   0x01
#define HAL_SPI_BUS_1_DEVICE_MODE_SET_BIT_SHIFT 0x00
#define HAL_SPI_BUS_2_DEVICE_MODE_MASTER        0x01
#define HAL_SPI_BUS_2_DEVICE_MODE_SET_BIT_MAX   0x01
#define HAL_SPI_BUS_2_DEVICE_MODE_SET_BIT_SHIFT 0x01

#define MCU_PERP_SPI_CR                    (0x5200055c)
#define MCU_PERP_SPI_CR_CLK_EN             (0x3)

#define HAL_QSPI_DMA_CFG        (*(volatile unsigned short *)(0x5C000404))
#define HAL_QSPI_1_DMA_CFG_MASK 0x0FU
#define HAL_QSPI_1_DMA_SEC_CORE 0x05
#define HAL_QSPI_1_DMA_APP_CORE 0x0A
#define HAL_QSPI_1_DMA_DSP_CORE 0x0F
#define HAL_QSPI_2_DMA_CFG_MASK 0xF0U
#define HAL_QSPI_2_DMA_SEC_CORE 0x50
#define HAL_QSPI_2_DMA_APP_CORE 0xA0
#define HAL_QSPI_2_DMA_DSP_CORE 0xF0

#define HAL_QSPI_INT_CFG             (*(volatile unsigned short *)(0x5C000408))
#define HAL_QSPI_1_INT_SEC_CORE_MASK (BIT(0))
#define HAL_QSPI_1_INT_APP_CORE_MASK (BIT(2))
#define HAL_QSPI_2_INT_SEC_CORE_MASK (BIT(4))
#define HAL_QSPI_2_INT_APP_CORE_MASK (BIT(6))

#define HAL_SPI_DATA_FRAME_SIZE_BIT     16
#define HAL_SPI_DATA_FRAME_SIZE_BITFILD 5
#define hal_spi_frame_size_trans_to_frame_bytes(x) (((x) + 1) >> 0x03)

#define HAL_SPI_MINUMUM_CLK_DIV 2
#define HAL_SPI_MAXIMUM_CLK_DIV 65534

#define hal_spi_mhz_to_hz(x) ((x) * 1000000)

#define HAL_SPI_RXDS_EN     BIT(18)
#define HAL_SPI_INST_DDR_EN BIT(17)
#define HAL_SPI_DDR_EN      BIT(16)
#define HAL_SPI_INST_L_POSE 8
#define HAL_SPI_ADDR_L_POSE 2

#define HAL_SPI_RSVD_NONE   0
#define HAL_SPI_RSVD_X8     0
#define HAL_SPI_RSVD_X8_X8  1
#define HAL_SPI_RSVD_X16    0x11
#define HAL_SPI_CLK_DIV_2   2
#define HAL_SPI_CLK_DIV_4   4
#define HAL_SPI_CLK_DIV_20  20

#define HAL_SPI_BUS_1_DEVICE_MODE_SET_BIT       0x00
#define HAL_SPI_BUS_1_DEVICE_MODE_SET_BIT_MAX   0x01
#define HAL_SPI_BUS_1_DEVICE_MODE_SET_BIT_SHIFT 0x00
#define HAL_SPI_BUS_2_DEVICE_MODE_MASTER        0x01
#define HAL_SPI_BUS_2_DEVICE_MODE_SET_BIT_MAX   0x01
#define HAL_SPI_BUS_2_DEVICE_MODE_SET_BIT_SHIFT 0x01

#define HAL_SPI_BUS_LINE_NUM_SHIFT              4
#define SPI_REC_OFFSET 1

#define SPI_DMA_TX_DATA_LEVEL_4     4
#define QSPI_DMA_TX_DATA_LEVEL_8    8

#define DMA_CFG_OFFSET              0x2c0
#define SPI2_TX_HANDSHARK_BIT       2
#define SPI2_RX_HANDSHARK_BIT       3

spi_v151_regs_t *g_spi_base_addrs[SPI_BUS_MAX_NUM] = {
    (spi_v151_regs_t *)SPI_BUS_0_BASE_ADDR,
#if (SPI_BUS_MAX_NUMBER > 1)
    (spi_v151_regs_t *)SPI_BUS_1_BASE_ADDR,
#endif
#if (SPI_BUS_MAX_NUMBER > 2)
    (spi_v151_regs_t *)SPI_BUS_2_BASE_ADDR,
#endif
};

#if defined(CONFIG_SPI_SUPPORT_INTERRUPT) && (CONFIG_SPI_SUPPORT_INTERRUPT == 1)
static int irq_spi0_handler(int i, void *p);
#if SPI_BUS_MAX_NUMBER > 1
static int irq_spi1_handler(int i, void *p);
#endif
#if SPI_BUS_MAX_NUMBER > 2
static int irq_spi2_handler(int i, void *p);
#endif

typedef struct spi_interrupt {
    core_irq_t irq_num;
    osal_irq_handler irq_func;
}
hal_spi_interrupt_t;

static const hal_spi_interrupt_t g_spi_interrupt_lines[SPI_BUS_MAX_NUMBER] = {
    {SPI_M_S_0_IRQN, irq_spi0_handler},
#if SPI_BUS_MAX_NUMBER > 1
    {SPI_M_S_1_IRQN, irq_spi1_handler},
#endif
#if SPI_BUS_MAX_NUMBER > 2
    {SPI_M_IRQN, irq_spi2_handler},
#endif
};

static int irq_spi0_handler(int i, void *p)
{
    unused(i);
    unused(p);
    hal_spi_v151_irq_handler(SPI_BUS_0);
    osal_irq_clear(g_spi_interrupt_lines[SPI_BUS_0].irq_num);
    return 0;
}

#if SPI_BUS_MAX_NUMBER > 1
static int irq_spi1_handler(int i, void *p)
{
    unused(i);
    unused(p);
    hal_spi_v151_irq_handler(SPI_BUS_1);
    osal_irq_clear(g_spi_interrupt_lines[SPI_BUS_1].irq_num);
    return 0;
}
#endif

#if SPI_BUS_MAX_NUMBER > 2
static int irq_spi2_handler(int i, void *p)
{
    unused(i);
    unused(p);
    hal_spi_v151_irq_handler(SPI_BUS_2);
    osal_irq_clear(g_spi_interrupt_lines[SPI_BUS_2].irq_num);
    return 0;
}
#endif

void spi_port_register_irq(spi_bus_t bus)
{
    osal_irq_request(g_spi_interrupt_lines[bus].irq_num, g_spi_interrupt_lines[bus].irq_func, NULL, NULL, NULL);
    osal_irq_set_priority(g_spi_interrupt_lines[bus].irq_num, irq_prio(g_spi_interrupt_lines[bus].irq_num));
    osal_irq_enable(g_spi_interrupt_lines[bus].irq_num);
}

void spi_port_unregister_irq(spi_bus_t bus)
{
    osal_irq_disable(g_spi_interrupt_lines[bus].irq_num);
    osal_irq_free(g_spi_interrupt_lines[bus].irq_num, NULL);
}
#endif

uintptr_t spi_porting_base_addr_get(spi_bus_t index)
{
    return (uintptr_t)g_spi_base_addrs[index];
}

uint32_t spi_porting_max_slave_select_get(spi_bus_t bus)
{
    unused(bus);
    return (uint32_t)SPI_SLAVE1;
}

void spi_porting_set_device_mode(spi_bus_t bus, spi_mode_t mode)
{
    if (mode == SPI_MODE_MASTER) {
        reg32_setbit(HAL_SPI_DEVICE_MODE_SET_REG, bus);
    } else {
        reg32_clrbit(HAL_SPI_DEVICE_MODE_SET_REG, bus);
    }
}

spi_mode_t spi_porting_get_device_mode(spi_bus_t bus)
{
    spi_mode_t mode = SPI_MODE_NONE;
    mode = reg32_getbit(HAL_SPI_DEVICE_MODE_SET_REG, bus);
    if (mode == 0) {
        return SPI_MODE_SLAVE;
    }
    return SPI_MODE_MASTER;
}

void spi_porting_set_sspi_mode(spi_bus_t bus, bool val)
{
    if (val) {
        reg32_setbit(HAL_SPI_DEVICE_MODE_SET_REG, bus + HAL_SPI_BUS_LINE_NUM_SHIFT);
    } else {
        reg32_clrbit(HAL_SPI_DEVICE_MODE_SET_REG, bus + HAL_SPI_BUS_LINE_NUM_SHIFT);
    }
}

bool spi_porting_is_sspi_mode(spi_bus_t bus)
{
    return (bool)reg32_getbit(HAL_SPI_DEVICE_MODE_SET_REG, bus + HAL_SPI_BUS_LINE_NUM_SHIFT);
}

void spi_porting_set_sspi_waite_cycle(spi_bus_t bus, uint32_t waite_cycle)
{
    g_spi_base_addrs[bus]->spi_rsvd = waite_cycle;
}

void spi_porting_set_rx_mode(spi_bus_t bus, uint16_t num)
{
    if (spi_porting_is_sspi_mode(bus)) {
        return;
    }
    g_spi_base_addrs[bus]->spi_er = 0;
    hal_spi_v151_spi_ctra_set_trsm(bus, HAL_SPI_TRANS_MODE_EEPROM);
    hal_spi_v151_spi_ctrb_set_nrdf(bus, num - 1);
    g_spi_base_addrs[bus]->spi_er = 1;
}

void spi_porting_set_tx_mode(spi_bus_t bus)
{
    if (spi_porting_is_sspi_mode(bus)) {
        return;
    }
    g_spi_base_addrs[bus]->spi_er = 0;
    hal_spi_v151_spi_ctra_set_trsm(bus, HAL_SPI_TRANS_MODE_TX);
    g_spi_base_addrs[bus]->spi_er = 1;
}

void spi_porting_set_txrx_mode(spi_bus_t bus)
{
    if (spi_porting_is_sspi_mode(bus)) {
        return;
    }
    g_spi_base_addrs[bus]->spi_er = 0;
    hal_spi_v151_spi_ctra_set_trsm(bus, HAL_SPI_TRANS_MODE_TXRX);
    g_spi_base_addrs[bus]->spi_er = 1;
}

void spi_porting_clock_en(void)
{
    writel(MCU_PERP_SPI_CR, MCU_PERP_SPI_CR_CLK_EN);
}

uint32_t spi_porting_lock(spi_bus_t bus)
{
    unused(bus);
    return osal_irq_lock();
}

void spi_porting_unlock(spi_bus_t bus, uint32_t irq_sts)
{
    unused(bus);
    osal_irq_restore(irq_sts);
}

#if defined(CONFIG_SPI_SUPPORT_DMA) && (CONFIG_SPI_SUPPORT_DMA == 1)
uint8_t spi_port_get_dma_trans_dest_handshaking(spi_bus_t bus)
{
    switch (bus) {
        case SPI_BUS_0:
            return (uint8_t)DMA_HANDSHAKE_SPI_BUS_0_TX;
#if (SPI_BUS_MAX_NUMBER > 1)
        case SPI_BUS_1:
            return (uint8_t)DMA_HANDSHAKE_SPI_BUS_1_TX;
#endif
#if (SPI_BUS_MAX_NUMBER > 2)
        case SPI_BUS_2:
            reg16_setbit(M_CTL_RB_BASE + DMA_CFG_OFFSET, SPI2_TX_HANDSHARK_BIT);     // i2s default, need change to spi2
            return (uint8_t)DMA_HANDSHAKE_SPI_BUS_2_TX;
#endif
        default:
            return (uint8_t)HAL_DMA_HANDSHAKING_MAX_NUM;
    }
}

uint8_t spi_port_get_dma_trans_src_handshaking(spi_bus_t bus)
{
    switch (bus) {
        case SPI_BUS_0:
            return (uint8_t)DMA_HANDSHAKE_SPI_BUS_0_RX;
#if (SPI_BUS_MAX_NUMBER > 1)
        case SPI_BUS_1:
            return (uint8_t)DMA_HANDSHAKE_SPI_BUS_1_RX;
#endif
#if (SPI_BUS_MAX_NUMBER > 2)
        case SPI_BUS_2:
            reg16_setbit(M_CTL_RB_BASE + DMA_CFG_OFFSET, SPI2_RX_HANDSHARK_BIT);     // i2s default, need change to spi2
            return (uint8_t)DMA_HANDSHAKE_SPI_BUS_2_RX;
#endif
        default:
            return (uint8_t)HAL_DMA_HANDSHAKING_MAX_NUM;
    }
}

uint8_t spi_port_tx_data_level_get(spi_bus_t bus)
{
    switch (bus) {
        case SPI_BUS_0:
        case SPI_BUS_1:
        case SPI_BUS_2:
            return (uint8_t)SPI_DMA_TX_DATA_LEVEL_4;
        default:
            return 0;
    }
}

uint8_t spi_port_rx_data_level_get(spi_bus_t bus)
{
    unused(bus);
    return 0;
}
#endif

#if defined(CONFIG_SPI_SUPPORT_QUERY_REGS)
#define SPI_ADDR_OFFEST_BYTES        4
void spi_port_get_regs_value(spi_bus_t bus)
{
    uint32_t count = 0;
    uintptr_t spi_bus_addr = spi_porting_base_addr_get(bus);
    osal_printk("spi base regs");
    for (uint32_t addr = spi_bus_addr; addr <= spi_bus_addr + 0x28; addr += SPI_ADDR_OFFEST_BYTES) {
        if (count % SPI_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    count = 0;
    for (uint32_t addr = spi_bus_addr + 0xBC; addr <= spi_bus_addr + 0xFC; addr += SPI_ADDR_OFFEST_BYTES) {
        if (count % SPI_ADDR_OFFEST_BYTES == 0) {
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

#if defined(CONFIG_SPI_SUPPORT_LPC)
void spi_port_clock_enable(spi_bus_t bus, bool on)
{
    clock_control_type_t control_type;
    clock_mclken_aperp_type_t aperp_type;

    switch (bus) {
        case SPI_BUS_0:
            aperp_type = CLOCK_APERP_SPI0_M_CLKEN;
            break;
#if (SPI_BUS_MAX_NUMBER > 1)
        case SPI_BUS_1:
            aperp_type = CLOCK_APERP_SPI1_M_CLKEN;
            break;
#endif
#if (SPI_BUS_MAX_NUMBER > 2)
        case SPI_BUS_2:
            aperp_type = CLOCK_APERP_SPI2_M_CLKEN;
            break;
#endif
        default:
            return;
    }

    if (on) {
        control_type = CLOCK_CONTROL_MCLKEN_ENABLE;
    } else {
        control_type = CLOCK_CONTROL_MCLKEN_DISABLE;
    }
    clock_ccrg_spi_enable((clock_ccrg_spi_used_t)(bus), on);
    uapi_clock_control(control_type, aperp_type);
}
#endif
