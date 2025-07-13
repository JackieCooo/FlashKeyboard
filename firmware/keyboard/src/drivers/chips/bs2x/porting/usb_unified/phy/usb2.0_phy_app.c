/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: USB port for project
 *
 * Create:  2023-01-10
 */

#include "chip_io.h"
#include "common_def.h"
#include "debug_print.h"
#include "usb_phy.h"
#include "tcxo.h"
#include "watchdog.h"
#ifdef PRE_FPGA
#include "clocks_switch.h"
#endif
#if defined(BUILD_APPLICATION_STANDARD)
#include "pm_clock.h"
#endif

#define PMU_CTL_SOFT_USB_GLB_RST_ADDR           0x5700478C
#define PMU_CTL_SOFT_USB_GLB_RST_CLDO_N_MSK     2
#define PMU_CTL_SOFT_USB_GLB_RST_AON_LGC_N      1
#define PMU_CTL_SOFT_USB_GLB_RST_AON_CRG_N      0

#define GLB_CTL_D_USB_SOFT_RST_ADDR             0x57000828
#define GLB_CTL_D_USB_SOFT_RST_AHB_N            1
#define GLB_CTL_D_USB_SOFT_RST_UTMI_N           0
#define DELAY_US200                             200

static usb_sts_sw_cb_t g_usb_status_sw_callback[USB_STATUS_MAX] = { NULL };

void usb_pd_pwr_up(void)
{
    reg32_setbit(PMU_CTL_SOFT_USB_GLB_RST_ADDR, PMU_CTL_SOFT_USB_GLB_RST_CLDO_N_MSK);
    reg32_setbit(PMU_CTL_SOFT_USB_GLB_RST_ADDR, PMU_CTL_SOFT_USB_GLB_RST_AON_CRG_N);
    reg32_setbit(PMU_CTL_SOFT_USB_GLB_RST_ADDR, PMU_CTL_SOFT_USB_GLB_RST_AON_LGC_N);

    reg32_setbit(GLB_CTL_D_USB_SOFT_RST_ADDR, GLB_CTL_D_USB_SOFT_RST_AHB_N);
    reg32_setbit(GLB_CTL_D_USB_SOFT_RST_ADDR, GLB_CTL_D_USB_SOFT_RST_UTMI_N);
}

#define PMU_CTL_GLB_CLKEN_ADDR                  0x52000548
#define PMU_CTL_GLB_CLKEN_USB_PHY_MSK           8
#define PMU_CTL_USB_PHY_CR_ADDR                 0x52000604
#define CR_DIV_NUM_MSK                          5
#define CR_DIV_NUM_LEN                          5
#define CR_DIV_EN_MSK                           4
#define CR_CLK_SEL_MSK                          1
#define CR_CLK_SEL_LEN                          3
#define CR_CH_ENABLE_MSK                        0
#define USB_PHY_CLK                             2

void clk_usb_phy_cfg(uint32_t clk_src, uint32_t div_num)
{
    /* Close CG. */
    reg32_clrbit(PMU_CTL_GLB_CLKEN_ADDR, PMU_CTL_GLB_CLKEN_USB_PHY_MSK);
    /* Close div_en. */
    reg32_clrbit(PMU_CTL_USB_PHY_CR_ADDR, CR_DIV_EN_MSK);
    /* Close channel enable. */
    reg32_clrbit(PMU_CTL_USB_PHY_CR_ADDR, CR_CH_ENABLE_MSK);
    /* Select clk_source. */
    reg32_setbits(PMU_CTL_USB_PHY_CR_ADDR, CR_CLK_SEL_MSK, CR_CLK_SEL_LEN, clk_src);
    /* Select div_num. */
    if (div_num != 1) {
        reg32_setbits(PMU_CTL_USB_PHY_CR_ADDR, CR_DIV_NUM_MSK, CR_DIV_NUM_LEN, div_num);
        reg32_setbit(PMU_CTL_USB_PHY_CR_ADDR, CR_DIV_EN_MSK);
    }
    /* Open enable and CG. */
    reg32_setbit(PMU_CTL_USB_PHY_CR_ADDR, CR_CH_ENABLE_MSK);
    reg32_setbit(PMU_CTL_GLB_CLKEN_ADDR, PMU_CTL_GLB_CLKEN_USB_PHY_MSK);
}

#define GLB_CTL_D_POR_RESET_ADDR                0x57000900
#define GLB_CTL_D_POR_RESET_SYS_RST_COM_N_MSK   1
#define GLB_CTL_D_POR_RESET_POR_RESET_MSK       0
#define GLB_CTL_D_UTMI_RESET_ADDR               0x57000904
#define GLB_CTL_D_UTMI_RESET_UTMI_RESET_0_MSK   0

#ifdef PRE_FPGA
#define FPGA_CTL_RB_BASE                        0x59004000

#define USB_PHY_RST_REG                         (FPGA_CTL_RB_BASE + 0x300)
#define USB_PHY_RST_PHY_RESETN                  0
#define USB_PHY_RST_RESETN                      4

#define USB_PHY_MODE_REG                        (FPGA_CTL_RB_BASE + 0x304)

#define USB_PHY_OUT_EN_REG                      (FPGA_CTL_RB_BASE + 0x308)
#define USB_PHY_OUT_EN_OUT_ENABLE               0
#endif

void usb_phy_reset(void)
{
#ifdef PRE_FPGA
    writew(USB_PHY_RST_REG, 0);

    writew(USB_PHY_MODE_REG, 0x10);
    writew(USB_PHY_OUT_EN_REG, 0x1);

    reg32_setbit(USB_PHY_RST_REG, USB_PHY_RST_RESETN);
    reg32_setbit(USB_PHY_RST_REG, USB_PHY_RST_PHY_RESETN);
#endif
}

static void usb_phy_power_off(void)
{
    uapi_reg_write16(PMU_CTL_SOFT_USB_GLB_RST_ADDR, 0);
    uapi_reg_write16(GLB_CTL_D_USB_SOFT_RST_ADDR, 0);
    uapi_reg_write16(GLB_CTL_D_POR_RESET_ADDR, 1);
    uapi_reg_write16(GLB_CTL_D_UTMI_RESET_ADDR, 1);
    uapi_reg_write16(0x57000824, 0);
    uapi_reg_write16(0x57004788, 0x3);
    uapi_reg_write16(0x57004780, 0);
    uapi_reg_clrbit(0x52000548, 0x9);
    uapi_reg_clrbit(0x52000548, 0x8);
    uapi_reg_clrbit(0x57008410, 0x3);
#if defined(BUILD_APPLICATION_STANDARD)
    (void)uapi_clock_control(CLOCK_CONTROL_USB_FREQ_CONFIG, CLOCK_FREQ_LEVEL_LOW_POWER);
#endif
    PRINT("usb_phy_power_off\n");
}

static void usb_phy_power_on(void)
{
#if defined(BUILD_APPLICATION_STANDARD)
    (void)uapi_clock_control(CLOCK_CONTROL_USB_FREQ_CONFIG, CLOCK_FREQ_LEVEL_HIGH);
#endif
    uapi_reg_setbit(0x57008410, 0x3);
    uapi_reg_setbit(0x52000548, 0x9);
    uapi_reg_setbit(0x52000548, 0x8);
#ifdef PRE_FPGA
    usb_phy_reset();
    usb_pd_pwr_up();
    clk_usb_phy_cfg(CLOCKS_CLK_SRC_TCXO, 2); /* usb phy clk div is 2 */
    uapi_tcxo_delay_us(600); /* usb phy need wait 600 us */
    writel(GLB_CTL_D_POR_RESET_ADDR, 0);
    writel(GLB_CTL_D_UTMI_RESET_ADDR, 0);
    uapi_tcxo_delay_us(600); /* usb phy need wait 600 us */
#else
    writel(0x52000304, 0x0); // usb_can_sel：USB与CAN只有一个能工作
    writel(0x5700478C, 0x0); // USB复位
    writel(0x57004780, 0x3); // USB上电
    uapi_tcxo_delay_us(30); // delay 30us
    writel(0x57004788, 0x0); // USB上电
    while (readl(0x57004784) != 0x1) { }
    writel(0x5700478C, 0x7); // USB解复位
    writel(0x57000910, 0x4); // clk_usb_gt = 0 保持62配置
    writel(0x57000828, 0x2); // soft_usb_ahb_rst_n = 1
    writel(0x57000828, 0x3); // soft_usb_utmi_rst_n = 1
    writel(0x57000900, 0x2); // POR解复位
    writel(0x57000904, 0x0); // utmi解复位
    writel(0x57000824, 0x2); // soft_rst_usb_phy_test_n = 1
    writel(0x57000824, 0x3); // usb_phy_test_clken = 1
    writel(0x57018004, 0x85000000); // usb_phy内部寄存器
    writel(0x57018100, 0x402); // ref_clk选择
    // 优化寄存器
#ifdef USB_SE0
    writel(0x57018008, 0x10f0f);
    writel(0x57018014, 0x8000300);
    while (1) {
        uapi_watchdog_kick();
        uapi_tcxo_delay_ms(2000); // delay 2000ms
        osal_printk("se0 ing\r\n");
    }
#endif
    writel(0x57018000, 0xA33c623);
    writel(0x57018008, 0x210f0f);
    uapi_reg_setbits(0x57018010, 0x4, 0x3, 0x7);
    uapi_reg_setbits(0x57018010, 0, 0x3, 0);
#endif
    uapi_tcxo_delay_us(3000); // delay 3000us
    PRINT("usb_phy_power_on\n");
}

void usb_start_hcd(void)
{
    usb_phy_power_on();
}

void usb_stop_hcd(void)
{
    usb_phy_power_off();
}

void usb_suspend(void)
{
    PRINT("\t usb suspend.\n");
#if defined(PM_SUPPORT_USB_WKUP)
    writew(0x57004700, 0x1);  // usb_wkup_evt_sel: USB PHY utmi_linestate 唤醒
    writew(0x57004724, 0x1);  // Full-Speed模式唤醒使能
    writew(0x57004708, 0x1);  // usb_wkup_clr
    writew(0x57004704, 0x1);  // usb_wkup_en

    pm_usb_suspend_cbk();

    uapi_pm_remove_sleep_veto(PM_VETO_ID_USB);
#endif
    if (g_usb_status_sw_callback[USB_SUSPEND] != NULL) {
        g_usb_status_sw_callback[USB_SUSPEND]();
    }
}

void usb_resume(void)
{
    PRINT("\t usb resume.\n");
#if defined(PM_SUPPORT_USB_WKUP)
    while (readw(0x5700470c) == 0x1) { // usb_wkup_sts
        writew(0x57004708, 0x1);  // usb_wkup_clr
        writew(0x57004724, 0xD);  // utmi_wkup_clr
    }

    pm_usb_resume_cbk();

    uapi_pm_add_sleep_veto(PM_VETO_ID_USB);
#endif
    if (g_usb_status_sw_callback[USB_RESUME] != NULL) {
        g_usb_status_sw_callback[USB_RESUME]();
    }
}

static const usb_phy_ops_t g_usb_phy_ops = {
    .usbStartHcd            = usb_start_hcd,
    .usbStopHcd             = usb_stop_hcd,
    .usbHost2Device         = NULL,
    .usbDevice2Host         = NULL,
    .usbSuspend             = usb_suspend,
    .usbResume              = usb_resume,
};

void usb_register_status_sw_callback(usb_status_t status, usb_sts_sw_cb_t cbk)
{
    if (status == USB_SUSPEND || status == USB_RESUME) {
        g_usb_status_sw_callback[status] = cbk;
    }
}

void usb_unregister_status_sw_callback(usb_status_t status)
{
    if (status == USB_SUSPEND || status == USB_RESUME) {
        g_usb_status_sw_callback[status] = NULL;
    }
}

void usb_phy_init(void)
{
    usb_phy_reg(&g_usb_phy_ops);
}
