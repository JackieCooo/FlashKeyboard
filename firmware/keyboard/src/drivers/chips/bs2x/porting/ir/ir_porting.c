/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Provides ir port template \n
 *
 * History: \n
 * 2024-03-15， Create file. \n
 */
#include "chip_core_irq.h"
#include "common_def.h"
#include "timer_porting.h"
#include "arch_port.h"
#include "sfc.h"
#include "adc.h"
#include "gpio.h"
#include "pinctrl.h"
#include "ir_porting.h"

/* gpio register */
#define IR_GPIO_BASE                    GPIO0_BASE_ADDR

#define IR_GPIO_OUT                     0x00
#define IR_GPIO_OEN                     (IR_GPIO_BASE + 0x04)
#define IR_GPIO_CTRL                    (IR_GPIO_BASE + 0x08)
#define IR_GPIO_INT_EN                  (IR_GPIO_BASE + 0x0C)
#define IR_GPIO_INT_MASK                (IR_GPIO_BASE + 0x10)
#define IR_GPIO_INT_TYPE                (IR_GPIO_BASE + 0x14)
#define IR_GPIO_INT_POLARITY            (IR_GPIO_BASE + 0x18)
#define IR_GPIO_INT_DEDGE               (IR_GPIO_BASE + 0x1C)
#define IR_GPIO_INT_DEBOUNCE            (IR_GPIO_BASE + 0x20)
#define IR_GPIO_INT_RAW                 (IR_GPIO_BASE + 0x24)
#define IR_GPIO_INTR                    (IR_GPIO_BASE + 0x28)
#define IR_GPIO_INT_EOI                 (IR_GPIO_BASE + 0x2C)
#define IR_GPIO_DATA_SET                (IR_GPIO_BASE + 0x30)
#define IR_GPIO_DATA_CLR                (IR_GPIO_BASE + 0x34)

/* timer register */
#define IR_TIMER0_BASE                  (TIMER_BASE_ADDR + 0x100)
#define IR_TIMER1_BASE                  (TIMER_BASE_ADDR + 0x200)

#define IR_TIMER_LOAD_COUNT             0x00
#define IR_TIMER_LOAD_COUNT_L           0x00
#define IR_TIMER_LOAD_COUNT_H           0x04
#define IR_TIMER_CURRENT_VALUE_L        0x08
#define IR_TIMER_CURRENT_VALUE_H        0x0C
#define IR_TIMER_CONTROL_REG            0x10
#define IR_TIMER_EOI                    0x14
#define IR_TIMER_INT_STATUS             0x18

#define IR_TIMER_ENABLE_USRMODE         0x05
#define IR_TICK_PER_SECOND              1000
#define IR_CYCLE_PER_TICK_PRI           ((CONFIG_TIMER_CLOCK_VALUE) / (IR_TICK_PER_SECOND))

#define IR_CFG_ANA_3_POS                12
#define IR_CFG_ANA_3_BITS               8
#define IR_CFG_AMUX_1_BITS              8
#define IR_CFG_AMUX_2_POS               4
#define IR_CFG_AMUX_2_BITS              4
#define IR_CFG_AMUX_2_OFFSET            4

#if defined(CONFIG_SAMPLE_SUPPORT_IR_STUDY)

static bool g_cmp_high_flag = true;

void ir_port_gpio_init(void)
{
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    uapi_pin_set_ie(CONFIG_SAMPLE_IR_STUDY_PIN, PIN_IE_1);
#endif /* CONFIG_PINCTRL_SUPPORT_IE */
    uapi_pin_set_mode(CONFIG_SAMPLE_IR_STUDY_PIN, 0);
    uapi_reg_clrbit(IR_GPIO_INT_DEBOUNCE, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_setbit(IR_GPIO_OEN, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_setbit(IR_GPIO_INT_MASK, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_setbit(IR_GPIO_INT_EOI, CONFIG_SAMPLE_IR_STUDY_PIN);
    /* falling trigger */
    uapi_reg_setbit(IR_GPIO_INT_TYPE, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_clrbit(IR_GPIO_INT_POLARITY, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_clrbit(IR_GPIO_INT_DEDGE, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_clrbit(IR_GPIO_INT_MASK, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_setbit(IR_GPIO_INT_EN, CONFIG_SAMPLE_IR_STUDY_PIN);
}

void ir_port_gpio_mask_interrupt(void)
{
    uapi_reg_setbit(IR_GPIO_INT_MASK, CONFIG_SAMPLE_IR_STUDY_PIN);
    uapi_reg_setbit(IR_GPIO_INT_EOI, CONFIG_SAMPLE_IR_STUDY_PIN);
}

void ir_port_gpio_eoi_clr_all(void)
{
    uapi_reg_write(IR_GPIO_INT_EOI, 0xFFFFFFFF);
}

uint32_t ir_port_gpio_get_intr_state(void)
{
    return uapi_reg_getbits(IR_GPIO_INTR, CONFIG_SAMPLE_IR_STUDY_PIN, 1);
}

uint32_t ir_port_cmp_irq_clear(void)
{
    uapi_reg_write(IR_CFG_ISO, 0x00000001);
    uapi_reg_write(IR_CFG_CMP_1, 0x00000001);
    uapi_reg_write(IR_CFG_CMP_1, 0x00000000);
    if (g_cmp_high_flag) {
        uapi_reg_setbit(IR_CFG_ANA_3, 0);
        g_cmp_high_flag = false;
    } else {
        uapi_reg_clrbit(IR_CFG_ANA_3, 0);
        g_cmp_high_flag = true;
        uapi_reg_write(IR_CFG_ISO, 0x00000000);
        osal_irq_clear(SEC_INT_IRQN);
        return 1;
    }
    uapi_reg_write(IR_CFG_ISO, 0x00000000);
    osal_irq_clear(SEC_INT_IRQN);
    return 0;
}

void ir_port_cmp_init(void)
{
    g_cmp_high_flag = true;
    uapi_pin_set_pull(CONFIG_SAMPLE_IR_SEND_PIN, PIN_PULL_DOWN);
    uapi_gpio_set_dir(S_MGPIO31, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(S_MGPIO31, GPIO_LEVEL_LOW);

    /* 比较器配置 */
    uapi_adc_init(ADC_CLOCK_NONE);
    uapi_adc_power_en(AFE_AMIC_MODE, true);
    /* 解数模接口ISO */
    uapi_reg_write(IR_CFG_ISO, 0x00000001);
    /* [8]pga_en [4]pga_vcm_en [0]preamp_os_dac_en */
    uapi_reg_write(IR_CFG_ANA_1, 0x00000110);
    /* [1]sel_pga [0]sel_preamp */
    uapi_reg_write(IR_CFG_FREG_9, 0x00000002);
    /* [11]vin_short [10]vip_short [9]rc_en [8]bypass_en [7:0]gain_sel */
    uapi_reg_write(IR_CFG_FREG_3, 0x00000100);
    /* [12:10]cc_sel [9:8]rz_sel [7:6]ibias_sel [5:3]vicm_ref_sel [2:0]vocm_ref_sel */
    uapi_reg_write(IR_CFG_FREG_4, 0x00000000);
    /* [7]rc_en [6]bypass_en [5:4]chop_mode_sel [3:0]gain_sel */
    uapi_reg_write(IR_CFG_FREG_5, 0x00000088);
    /* [9:7]cc_sel [6:5]rz_sel [4:3]ibias_sel [2:0]vocm_ref_sel */
    uapi_reg_write(IR_CFG_FREG_6, 0x000001CA);
    /* [15:6]topbias_trim [5:4]chopclk_div_freq_sel [3:2]chopclk_div_dly_sel [1]sel_preamp [0]sel_pga */
    uapi_reg_write(IR_CFG_FREG_7, 0x00005544);
    /* [9:7]bufp_cc_sel [6:5]rz_sel [4]ibias_sel [3:2]chop_mode_sel [1]prechg_en [0]bypass_en */
    uapi_reg_write(IR_CFG_FREG_1, 0x000001A1);
    /* [9:7]bufp_cc_sel [6:5]rz_sel [4]ibias_sel [3:2]chop_mode_sel [1]prechg_en [0]bypass_en */
    uapi_reg_write(IR_CFG_FREG_2, 0x000001A1);
    /* [12]cmpen [10:8]ref_sel<2:0>:<10>vin;<9>ipoly;<8>vbat [5:0]rdac_code<5:0> */
    uapi_reg_write(IR_CFG_ANA_2, 0x00001239);
    /* [29:28]difmod_sel<1:0>:<29>vin;<28>rdac_out [24]hys_en  [19:12]chp_sel<7:0> [11:4]chn_sel<7:0>
    <2:0>:<2>rdac_test_en;<1>rdac_en;<0>pole_sel */
    /* bit0 0:高电平触发 1:低电平触发 */
    uapi_reg_write(IR_CFG_ANA_3, 0x10001003);
    uapi_reg_setbits(IR_CFG_ANA_3, IR_CFG_ANA_3_POS, IR_CFG_ANA_3_BITS, 1 << CONFIG_IR_STUDY_RX_OUT);
    /* [23:12]传感器AMUXP通道选择静态配置：<23>2/5分压选择，低电平分压；
    <22:12>从高到低依次对应vicmref,VSSAFE1,VSSAFE3,AIO7~AIO0;[11:0]传感器AMUXN通道选择静态配置：<11>2/5分压选择，低电平分压；
    <10:0>从高到低依次对应vicmref,VSSAFE1,VSSAFE3,AIO7~AIO0 */
    uapi_reg_write(IR_CFG_AMUX_1, 0x00800801);
    uapi_reg_setbits(IR_CFG_AMUX_1, 0, IR_CFG_AMUX_1_BITS, 1 << CONFIG_IR_STUDY_RX_OUT);
    /* [7:4]音频AMUXP通道选择静态配置：<7:4>从高到低依次对应AIO7~AIO4;[3:0];[3:0}音频AMUXN通道选择静态配置：
    <3:0>从高到低依次对应AIO7~AIO4 */
    uapi_reg_write(IR_CFG_AMUX_2, 0x00000012);
    uapi_reg_setbits(IR_CFG_AMUX_2, IR_CFG_AMUX_2_POS, IR_CFG_AMUX_2_BITS,
                     1 << (CONFIG_IR_STUDY_RX_IN - IR_CFG_AMUX_2_OFFSET));
    uapi_reg_setbits(IR_CFG_AMUX_2, 0, IR_CFG_AMUX_2_BITS, 1 << (CONFIG_IR_STUDY_MIC_N - IR_CFG_AMUX_2_OFFSET));
    /* [0]cmp_out告警清零，1表示清零 */
    uapi_reg_write(IR_CFG_CMP_1, 0x00000001);
    uapi_reg_write(IR_CFG_CMP_1, 0x00000000);
    /* cmp_even模式切换 */
    uapi_reg_write(IR_CFG_CMP_0, 0x0000001F);
    /* [20]cfg_gadc_clken_ctrl;[16]cfg_gadc_clken_prechg;[12]cfg_gadc_clken_byp;[8]cfg_gadc_clken_fc;
    [4]cfg_gadc_clken_bc;[0]cfg_clken_tst */
    uapi_reg_write(IR_CFG_CLKEN, 0x00111111);
    /* 解数模接口ISO */
    uapi_reg_write(IR_CFG_ISO, 0x00000000);
}

void ir_port_cmp_deinit(void)
{
    /* 解数模接口ISO */
    uapi_reg_write(IR_CFG_ISO, 0x00000001);
    uapi_reg_write(IR_CFG_ANA_1, 0x00011111);
    uapi_reg_write(IR_CFG_FREG_9, 0x00000000);
    uapi_reg_write(IR_CFG_FREG_3, 0x00000280);
    uapi_reg_write(IR_CFG_FREG_4, 0x00001212);
    uapi_reg_write(IR_CFG_FREG_5, 0x00000088);
    uapi_reg_write(IR_CFG_FREG_6, 0x000001CA);
    uapi_reg_write(IR_CFG_FREG_7, 0x00005544);
    uapi_reg_write(IR_CFG_FREG_1, 0x000001A0);
    uapi_reg_write(IR_CFG_FREG_2, 0x000001A0);
    uapi_reg_write(IR_CFG_ANA_2, 0x00000020);
    uapi_reg_write(IR_CFG_ANA_3, 0x10000000);
    uapi_reg_write(IR_CFG_AMUX_1, 0x00800800);
    uapi_reg_write(IR_CFG_AMUX_2, 0x00000084);
    uapi_reg_write(IR_CFG_CMP_1, 0x00000000);
    uapi_reg_write(IR_CFG_CMP_0, 0x00000008);
    uapi_reg_write(IR_CFG_CLKEN, 0x00111111);
    uapi_reg_write(IR_CFG_ISO, 0x00000000);
    uapi_adc_power_en(AFE_AMIC_MODE, false);
    uapi_adc_deinit();
}
#endif /* CONFIG_SAMPLE_SUPPORT_IR_STUDY */

void ir_port_timer1_init(uint32_t us)
{
    /* before config timer, disable it */
    uapi_reg_write((IR_TIMER1_BASE + IR_TIMER_CONTROL_REG), 0);

    uapi_reg_write((IR_TIMER1_BASE + IR_TIMER_LOAD_COUNT), timer_porting_us_2_cycle(us));
    /* user define mode and enable it */
    uapi_reg_write((IR_TIMER1_BASE + IR_TIMER_CONTROL_REG), IR_TIMER_ENABLE_USRMODE);
}

void ir_port_timer0_init(uint32_t us)
{
    /* before config timer, disable it */
    uapi_reg_write((IR_TIMER0_BASE + IR_TIMER_CONTROL_REG), 0);

    uapi_reg_write((IR_TIMER0_BASE + IR_TIMER_LOAD_COUNT), timer_porting_us_2_cycle(us));
    /* user define mode and enable it */
    uapi_reg_write((IR_TIMER0_BASE + IR_TIMER_CONTROL_REG), IR_TIMER_ENABLE_USRMODE);
}

void ir_port_tick_timer0_disable(void)
{
    uapi_reg_write((IR_TIMER0_BASE + IR_TIMER_CONTROL_REG), 0);
}

void ir_port_tick_timer1_disable(void)
{
    uapi_reg_write((IR_TIMER1_BASE + IR_TIMER_CONTROL_REG), 0);
}

void ir_port_tick_timer0_enable(void)
{
    uapi_reg_write((IR_TIMER0_BASE + IR_TIMER_LOAD_COUNT), IR_CYCLE_PER_TICK_PRI);
    /* user define mode and enable it */
    uapi_reg_write((IR_TIMER0_BASE + IR_TIMER_CONTROL_REG), IR_TIMER_ENABLE_USRMODE);
}

void ir_port_tick_timer1_enable(void)
{
    uapi_reg_write((IR_TIMER1_BASE + IR_TIMER_LOAD_COUNT), IR_CYCLE_PER_TICK_PRI);
    /* user define mode and enable it */
    uapi_reg_write((IR_TIMER1_BASE + IR_TIMER_CONTROL_REG), IR_TIMER_ENABLE_USRMODE);
}

void ir_port_tick_timer0_eoi_clr(void)
{
    uapi_reg_write(IR_TIMER0_BASE + IR_TIMER_EOI, 1);
}

void ir_port_tick_timer1_eoi_clr(void)
{
    uapi_reg_write(IR_TIMER1_BASE + IR_TIMER_EOI, 1);
}

void ir_port_register_irq(uint32_t irq_id, osal_irq_handler ir_handler)
{
    uint32_t ret = osal_irq_request(irq_id, ir_handler, NULL, NULL, NULL);
    osal_irq_set_priority(irq_id,  irq_prio(irq_id));
    osal_irq_enable(irq_id);
    if (ret != 0) {
        osal_printk("oal_int_create ret:0x%x, irq_id:%d\n", ret, irq_id);
    }
}

void ir_port_unregister_irq(uint32_t irq_id)
{
    osal_irq_free(irq_id, NULL);
}

errcode_t ir_port_flash_reg_write(uint32_t flash_addr, uint8_t *write_data, uint32_t write_size)
{
    return uapi_sfc_reg_write(flash_addr, write_data, write_size);
}

errcode_t ir_port_flash_reg_read(uint32_t flash_addr, uint8_t *read_buffer, uint32_t read_size)
{
    return uapi_sfc_reg_read(flash_addr, read_buffer, read_size);
}

errcode_t ir_port_flash_reg_erase(uint32_t flash_addr, uint32_t erase_size)
{
    return uapi_sfc_reg_erase(flash_addr, erase_size);
}