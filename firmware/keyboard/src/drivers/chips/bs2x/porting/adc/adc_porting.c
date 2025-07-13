/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides adc port \n
 *
 * History: \n
 * 2022-09-16， Create file. \n
 */
#include "soc_osal.h"
#include "pm_clock.h"
#include "chip_core_irq.h"
#include "hal_adc_v153.h"
#include "gpio.h"
#include "pinctrl.h"
#include "efuse.h"
#include "arch_port.h"
#include "adc_porting.h"
#if defined(CONFIG_ADC_SUPPORT_QUERY_REGS)
#include "osal_debug.h"
#endif

#define TRIM_EFUSE_BASE             0x57028800
#define TRIM_AFE_BASE               0x57036300
#define TRIM_AFE_OFFSET             0x6C
#define EFUSE0_AFE_BYTE             54
#define EFUSE0_AFE_LEN              2
#define ADC_COMPARATOR_T0_TRIM_BIT  0xE
#define ADC_COMPARATOR_T0_DIR_BIT   0xD
#define ADC_COMPARATOR_T0_VAL_BIT   0xC
#define ADC_COMPARATOR_T0_TRIM_MAX  0x2
#define CDAC_CALI_OFFSET            0x0D4
#define CDAC_CALI_LEN               0x9
#define OS_CALI_OFFSET              0x138
#define DCOC_CALI_OFFSET            0x230
#define SPD_CALI_OFFSET             0x260

typedef struct {
    uint8_t trim_efuse_pos;
    uint8_t trim_reg_addr;
    uint8_t trim_reg_pos;
    uint8_t trim_reg_bits;
} trim_cfg_t;

afe_scan_mode_t g_adc_working_afe = AFE_SCAN_MODE_MAX_NUM;
static const trim_cfg_t g_trim_cfg[] = {
    { 0x0, 0xF0, 0x4, 0x4 }, // efuse: 0x5702886C[0-3], AFELDO_trim: 0x570363F0[4-7]
    { 0x4, 0xE8, 0x4, 0x4 }, // efuse: 0x5702886C[4-7], ADCLDO_trim: 0x570363E8[4-7]
    { 0x8, 0xF4, 0x5, 0x4 }, // efuse: 0x5702886C[8-11], VREFLDO_trim: 0x570363F4[5-8]
};
static afe_config_t g_afe_cail_code = { 0 };

void adc_port_register_hal_funcs(void)
{
    hal_adc_register_funcs(hal_adc_v153_funcs_get());
}

void adc_port_unregister_hal_funcs(void)
{
    hal_adc_unregister_funcs();
}

void adc_port_init_clock(adc_clock_t clock)
{
    unused(clock);
}

void adc_port_clock_enable(bool on)
{
    if (on) {
        uapi_clock_control(CLOCK_CONTROL_XO_OUT_ENABLE, CLOCK_XO2AFE);
    } else {
        uapi_clock_control(CLOCK_CONTROL_XO_OUT_DISABLE, CLOCK_XO2AFE);
    }
}

static void irq_adc_done_handler(void)
{
    hal_adc_done_irq_handler(g_adc_working_afe);
}

static void irq_adc_alarm_handler(void)
{
    hal_adc_alarm_irq_handler(g_adc_working_afe);
}

void adc_port_register_irq(afe_scan_mode_t afe_scan_mode)
{
    if (afe_scan_mode >= AFE_SCAN_MODE_MAX_NUM) {
        return;
    }
    osal_irq_request(GADC_DONE_IRQN, (osal_irq_handler)irq_adc_done_handler, NULL, NULL, NULL);
    osal_irq_request(GADC_ALARM_IRQN, (osal_irq_handler)irq_adc_alarm_handler, NULL, NULL, NULL);
    osal_irq_enable(GADC_DONE_IRQN);
    osal_irq_enable(GADC_ALARM_IRQN);
    osal_irq_set_priority(GADC_DONE_IRQN, irq_prio(GADC_DONE_IRQN));
    osal_irq_set_priority(GADC_ALARM_IRQN, irq_prio(GADC_ALARM_IRQN));
    g_adc_working_afe = afe_scan_mode;
}

void adc_port_unregister_irq(afe_scan_mode_t afe_scan_mode)
{
    if (afe_scan_mode >= AFE_SCAN_MODE_MAX_NUM) {
        return;
    }
    osal_irq_disable(GADC_DONE_IRQN);
    osal_irq_disable(GADC_ALARM_IRQN);
    osal_irq_free(GADC_DONE_IRQN, NULL);
    osal_irq_free(GADC_ALARM_IRQN, NULL);
    g_adc_working_afe = AFE_SCAN_MODE_MAX_NUM;
}

void adc_port_power_on(bool on)
{
    unused(on);
}

afe_config_t adc_get_cali_code(void)
{
    afe_config_t afe_cail_code = { 0 };
    for (uint32_t i = 0; i < CDAC_CALI_LEN; i++) {
        *(&(afe_cail_code.rpt_wcal_c17) + i) = readl(ADC_BASE_ADDR + CDAC_CALI_OFFSET + (i * sizeof(uint32_t)));
    }
    afe_cail_code.cmp_os_code = readl(ADC_BASE_ADDR + OS_CALI_OFFSET);
    afe_cail_code.preamp_os_code = readl(ADC_BASE_ADDR + DCOC_CALI_OFFSET);
    afe_cail_code.spd_dly_code = readl(ADC_BASE_ADDR + SPD_CALI_OFFSET);
    return afe_cail_code;
}

errcode_t adc_set_cali_code(afe_scan_mode_t afe_scan_mode, afe_config_t *afe_config)
{
    if (afe_scan_mode >= AFE_SCAN_MODE_MAX_NUM) {
        return ERRCODE_INVALID_PARAM;
    }
    hal_spd_cali_set_code(afe_config->spd_dly_code);

    hal_os_cali_set_code(afe_config->cmp_os_code);
    g_adc_regs->cfg_cmp_os_11 = 1;

    cfg_cdac_fc0_data1_t cdac_cali_bit = { 0 };
    cdac_cali_bit.b.cfg_gadc_cdac_fc_cap_start = 0xB;
    cdac_cali_bit.b.cfg_gadc_cdac_fc_cap_end = 0x11;
    hal_gadc_gap_start_set(cdac_cali_bit);
    for (uint32_t i = 0; i < CDAC_CALI_LEN; i++) {
        hal_cdac_cali_set_code(i, *(((uint32_t *)&(afe_config->rpt_wcal_c17)) + i));
    }
    hal_cdac_cali_code_sync();

    hal_dcoc_cali_set_code(afe_config->preamp_os_code);
    hal_gafe_sample_info_t gafe_sample = GADC_DEFAULT_CONFIG;
    if (afe_scan_mode == AFE_GADC_MODE) {
        g_adc_regs->cfg_gadc_data_0 = gafe_sample.cfg_adc_data0.d32;
        g_adc_regs->cfg_gadc_data_1 = gafe_sample.cfg_adc_data1.d32;
    }
    return ERRCODE_SUCC;
}

errcode_t adc_calibration(afe_scan_mode_t afe_scan_mode, bool os_cali, bool cdac_cali, bool dcoc_cali)
{
    errcode_t ret = hal_adc_v153_cali(afe_scan_mode, os_cali, cdac_cali, dcoc_cali);
    return ret;
}

void adc_port_ldo_enable(bool enable)
{
    if (enable) {
        uapi_clock_control(CLOCK_CONTROL_XO_OUT_ENABLE, CLOCK_XO2AFE);
        hal_afe_afeldo_open();
        hal_afe_adcldo_open();
        hal_afe_vrefldo_open();
        hal_afe_vrefldo_close_rst();
    } else {
        hal_afe_vrefldo_off();
        hal_afe_adcldo_off();
        hal_afe_afeldo_off();
        uapi_clock_control(CLOCK_CONTROL_XO_OUT_DISABLE, CLOCK_XO2AFE);
    }
}

void hal_adc_ldo_trim(void)
{
    uint8_t adc_trim[EFUSE0_AFE_LEN] = { 0 };
    uapi_efuse_init();
    uapi_efuse_read_buffer(adc_trim, EFUSE0_AFE_BYTE, EFUSE0_AFE_LEN);
    reg32_setbits(TRIM_AFE_BASE + g_trim_cfg[0].trim_reg_addr, g_trim_cfg[0].trim_reg_pos,
                  g_trim_cfg[0].trim_reg_bits, adc_trim[0] & 0xf);
    reg32_setbits(TRIM_AFE_BASE + g_trim_cfg[1].trim_reg_addr, g_trim_cfg[1].trim_reg_pos,
                  g_trim_cfg[1].trim_reg_bits, (adc_trim[0] >>  g_trim_cfg[1].trim_efuse_pos) & 0xf);
    reg32_setbits(TRIM_AFE_BASE + g_trim_cfg[EFUSE0_AFE_LEN].trim_reg_addr, g_trim_cfg[EFUSE0_AFE_LEN].trim_reg_pos,
                  g_trim_cfg[EFUSE0_AFE_LEN].trim_reg_bits, adc_trim[1] & 0xf);
}

#if defined(CONFIG_ADC_SUPPORT_QUERY_REGS)
#define ADC_ADDR_OFFEST_BYTES       4
static void adc_port_get_part_regs_value(void)
{
    uint32_t count = 0;
    osal_printk("\r\naon adc regs");
    for (uint32_t addr = 0x5702c228; addr <= 0x5702c248; addr += ADC_ADDR_OFFEST_BYTES) {
        if (count % ADC_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    count = 0;
    osal_printk("\r\ndiag adc regs");
    for (uint32_t addr = 0x52004100; addr <= 0x5200410C; addr += ADC_ADDR_OFFEST_BYTES) {
        if (count % ADC_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    count = 0;
    for (uint32_t addr = 0x52004200; addr <= 0x5200423C; addr += ADC_ADDR_OFFEST_BYTES) {
        if (count % ADC_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    osal_printk("\r\n");
}

void adc_port_get_regs_value(void)
{
    uint32_t count = 0;
    osal_printk("adc base regs");
    for (uint32_t addr = 0x57036000; addr < 0x570363D0; addr += ADC_ADDR_OFFEST_BYTES) {
        if (count % ADC_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    count = 0;
    osal_printk("\r\nana regs");
    for (uint32_t addr = 0x570363D0; addr <= 0x57036420; addr += ADC_ADDR_OFFEST_BYTES) {
        if (count % ADC_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    count = 0;
    osal_printk("\r\npmu afe_adc regs");
    for (uint32_t addr = 0x57008700; addr <= 0x57008730; addr += ADC_ADDR_OFFEST_BYTES) {
        if (count % ADC_ADDR_OFFEST_BYTES == 0) {
            osal_printk("\r\n0x%x    0x%x", addr, readl(addr));
            count++;
        } else {
            osal_printk("  0x%x", readl(addr));
            count++;
        }
    }
    adc_port_get_part_regs_value();
}
#endif

void adc_port_comparator_t0_trim(void)
{
    if ((reg16(TRIM_EFUSE_BASE + TRIM_AFE_OFFSET) & BIT(ADC_COMPARATOR_T0_TRIM_BIT)) == 0) { return; }
    cfg_ana_2_t cfg_ana_2;
    cfg_ana_2.d32 = g_adc_ana_regs->cfg_ana_2;
    if (reg16(TRIM_EFUSE_BASE + TRIM_AFE_OFFSET) & BIT(ADC_COMPARATOR_T0_DIR_BIT)) {
        if (reg16(TRIM_EFUSE_BASE + TRIM_AFE_OFFSET) & BIT(ADC_COMPARATOR_T0_VAL_BIT)) {
            cfg_ana_2.b.cfg_cmp_vin_rdac_code -= ADC_COMPARATOR_T0_TRIM_MAX;
        } else {
            cfg_ana_2.b.cfg_cmp_vin_rdac_code--;
        }
    } else {
        if (reg16(TRIM_EFUSE_BASE + TRIM_AFE_OFFSET) & BIT(ADC_COMPARATOR_T0_VAL_BIT)) {
            cfg_ana_2.b.cfg_cmp_vin_rdac_code += ADC_COMPARATOR_T0_TRIM_MAX;
        } else {
            cfg_ana_2.b.cfg_cmp_vin_rdac_code++;
        }
    }
    g_adc_ana_regs->cfg_ana_2 = cfg_ana_2.d32;
}

void hal_cpu_trace_restart(void)
{
#if (CONFIG_DRIVER_SUPPORT_CPUTRACE) && defined(SUPPORT_CPU_TRACE)
    uint32_t temp = osal_irq_lock();
    reg16_clrbit(0x52004204, 0x9);
    reg16_clrbit(0x52004100, 0x0);
    writew(0x5200410c, 0x1FF);
    reg16_setbit(0x52004204, 0x0);

    writew(0x52004210, 0x6000);
    writew(0x52004214, 0x5200);
    writew(0x52004218, 0x6400);
    writew(0x5200421c, 0x5200);
    writew(0x52004208, 0x3FE);
    writew(0x5200420c, 0x0);
    writew(0x52004104, 0x0);
    reg16_setbit(0x52004108, 0x1);

    writew(0x52004204, 0x1);
    writew(0x52004204, 0x201);
    writew(0x52004204, 0x301);
    writew(0x52004108, 0x3);
    osal_irq_restore(temp);
#endif
}

void adc_port_afe_iso_enable(bool en)
{
    if (en) {
        reg16_setbit(0x5702C230, 0xA);
    } else {
        reg16_clrbit(0x5702C230, 0xA);
    }
}

static bool adc_channel_vaild_check(adc_channel_t channel)
{
    if (channel >= ADC_CHANNEL_MAX_NUM) { return false; }
    return true;
}

errcode_t adc_port_gadc_entirely_open(adc_channel_t channel, bool self_cali)
{
    if (!adc_channel_vaild_check(channel)) { return ERRCODE_ADC_INVALID_PARAMETER; }
    pin_t adc_pin[] = {S_MGPIO2, S_MGPIO3, S_MGPIO4, S_MGPIO5, S_MGPIO28, S_MGPIO29, S_MGPIO30, S_MGPIO31};
    uapi_adc_init(ADC_CLOCK_NONE);
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    /* ADC管脚无需配置IE使能且管脚默认IE为0，为防止用户修改IE，特在此将IE配置为0 */
    uapi_pin_set_ie(adc_pin[channel], PIN_IE_0);
#endif
    uapi_pin_set_mode(adc_pin[channel], PIN_MODE_0);
    uapi_gpio_set_dir(adc_pin[channel], GPIO_DIRECTION_INPUT);
    uapi_pin_set_pull(adc_pin[channel], PIN_PULL_NONE);
    uapi_adc_power_en(AFE_GADC_MODE, true);
    uapi_adc_open_channel(channel);
    if (self_cali) {
        adc_calibration(AFE_GADC_MODE, true, true, true);
    } else {
        adc_set_cali_code(AFE_GADC_MODE, &g_afe_cail_code);
    }
    return ERRCODE_SUCC;
}

int32_t adc_port_gadc_entirely_sample(adc_channel_t channel)
{
    if (!adc_channel_vaild_check(channel)) { return ERRCODE_ADC_INVALID_PARAMETER; }
    pin_t adc_pin[] = {S_MGPIO2, S_MGPIO3, S_MGPIO4, S_MGPIO5, S_MGPIO28, S_MGPIO29, S_MGPIO30, S_MGPIO31};
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    /* ADC管脚无需配置IE使能且管脚默认IE为0，为防止用户修改IE，特在此将IE配置为0 */
    uapi_pin_set_ie(adc_pin[channel], PIN_IE_0);
#endif
    uapi_pin_set_mode(adc_pin[channel], PIN_MODE_0);
    uapi_gpio_set_dir(adc_pin[channel], GPIO_DIRECTION_INPUT);
    uapi_pin_set_pull(adc_pin[channel], PIN_PULL_NONE);

    uapi_adc_open_channel(channel);
    int32_t adc_value = uapi_adc_auto_sample(channel);
    return ((adc_value * ADC_TICK2VOL_REF_VOLTAGE_MV) >> GAFE_SAMPLE_VALUE_SIGN_BIT);
}

errcode_t adc_port_gadc_entirely_close(adc_channel_t channel)
{
    if (!adc_channel_vaild_check(channel)) { return ERRCODE_ADC_INVALID_PARAMETER; }
    g_afe_cail_code = adc_get_cali_code();
    uapi_adc_close_channel(channel);
    uapi_adc_power_en(AFE_GADC_MODE, false);
    uapi_adc_deinit();
    return ERRCODE_SUCC;
}