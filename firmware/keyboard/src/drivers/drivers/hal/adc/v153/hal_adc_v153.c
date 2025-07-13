/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides V153 HAL adc \n
 *
 * History: \n
 * 2023-08-31， Create file. \n
 */
#include "common_def.h"
#include "pm_pmu.h"
#include "adc_porting.h"
#include "hal_adc_v153.h"

hal_common_sample_info_t const g_common_sample = COMMON_DEFAULT_CONFIG;
hal_amic_sample_info_t const g_amic_sample = AMIC_DEFAULT_CONFIG;
hal_gafe_sample_info_t const g_gafe_sample = GADC_DEFAULT_CONFIG;
static cfg_amux_1_t g_gafe_cfg_amux_1 = { 0 };
static bool g_adc_first_sample = false;
static bool g_calibration_done = false;

static errcode_t hal_adc_v153_init(void)
{
    adc_port_afe_iso_enable(true);
    hal_afe_release_xo32m();
    uapi_tcxo_delay_us(HAL_ADC_V153_CFG_DELAY_30);
    hal_afe_mtcmos_en();
    uapi_tcxo_delay_us(HAL_ADC_V153_CFG_DELAY_50);
    hal_afe_iso_release();
    hal_afe_ana_rstn_release();
    hal_afe_dig_clk_release();
    hal_afe_dig_apb_rstn_release();
    hal_afe_dig_clr();
    hal_afe_dig_start();
    return ERRCODE_SUCC;
}

static errcode_t hal_adc_v153_deinit(void)
{
    adc_port_afe_iso_enable(false);
    return ERRCODE_SUCC;
}

#if defined(CONFIG_ADC_SUPPORT_DIFFERENTIAL)
static errcode_t hal_adc_v153_differential_channel_set(adc_channel_t postive_ch, adc_channel_t negative_ch, bool on)
{
    if (!on) {
        hal_gafe_channel_close();
    } else {
        cfg_amux_2_t cfg_amux_2 = { 0 };
        cfg_amux_2.b.cfg_amuxp_audio_ch_sel = BIT(postive_ch - AIN4);     // Audio can use AIN4~AIN7
        cfg_amux_2.b.cfg_amuxn_audio_ch_sel = BIT(negative_ch - AIN4);    // Audio can use AIN4~AIN7
        g_adc_regs->cfg_amux_2 = cfg_amux_2.d32;
    }
    return ERRCODE_SUCC;
}
#endif

static errcode_t hal_adc_v153_channel_set(adc_channel_t channel, bool on)
{
    if (!on) {
        hal_gafe_channel_close();
    } else {
        g_gafe_cfg_amux_1.b.amuxp_devide_disable = 1;
        g_gafe_cfg_amux_1.b.amuxn_devide_disable = 1;
        g_adc_regs->cfg_amux_1 = g_gafe_cfg_amux_1.d32;
        g_gafe_cfg_amux_1.b.amuxp_sensor_ch_sel = BIT(channel);
        g_gafe_cfg_amux_1.b.amuxn_sensor_ch_sel = BIT(VSSAFE1);
        g_adc_regs->cfg_amux_2 = 0;
    }
    return ERRCODE_SUCC;
}

__attribute__((weak)) void hal_adc_ldo_trim(void)
{
}

static void hal_adc_v153_power_on(void)
{
    hal_adc_ldo_trim();
    hal_afe_afeldo_open();
    hal_afe_adcldo_open();
    hal_afe_vrefldo_open();
}

errcode_t hal_adc_v153_cali(afe_scan_mode_t afe_scan_mode, bool os_cali, bool cdac_cali, bool dcoc_cali)
{
    if (g_calibration_done) {
        if (afe_scan_mode == AFE_GADC_MODE) {
            g_adc_regs->cfg_gadc_data_0 = g_gafe_sample.cfg_adc_data0.d32;
            g_adc_regs->cfg_gadc_data_1 = g_gafe_sample.cfg_adc_data1.d32;
        }
        return ERRCODE_SUCC;
    }
    hal_adc_v153_spd_cali();
    if (os_cali) {
        hal_adc_v153_os_cali();
    }
    if (cdac_cali) {
        hal_adc_v153_cdac_cali();
    }
    if (dcoc_cali == true) {
        hal_adc_v153_dcoc_cali();
    }
    if (afe_scan_mode == AFE_GADC_MODE) {
        g_adc_regs->cfg_gadc_data_0 = g_gafe_sample.cfg_adc_data0.d32;
        g_adc_regs->cfg_gadc_data_1 = g_gafe_sample.cfg_adc_data1.d32;
    }
    UNUSED(afe_scan_mode);
    g_calibration_done = true;
    return ERRCODE_SUCC;
}

static void hal_adc_v153_enable(afe_scan_mode_t afe_scan_mode)
{
    hal_adc_common_enable((hal_common_sample_info_t*)&g_common_sample);
    if (afe_scan_mode == AFE_GADC_MODE) {
        hal_adc_gadc_enable((hal_gafe_sample_info_t*)&g_gafe_sample);
    } else if (afe_scan_mode == AFE_AMIC_MODE) {
        hal_adc_amic_enable((hal_amic_sample_info_t*)&g_amic_sample);
        uapi_pmu_control(PMU_CONTROL_MICLDO_POWER, PMU_CONTROL_POWER_ON);
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_2V0);
    }
    hal_gafe_enable();
}

static void hal_adc_v153_power_off(afe_scan_mode_t afe_scan_mode)
{
    if (afe_scan_mode == AFE_AMIC_MODE) {
        uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_1V1);
        uapi_pmu_control(PMU_CONTROL_MICLDO_POWER, PMU_CONTROL_POWER_OFF);
        g_adc_ana_regs->cfg_ana_1 = 0;
    } else {
        g_adc_regs->cfg_gadc_data_0 = 0;
    }
    hal_afe_vrefldo_off();
    hal_afe_adcldo_off();
    hal_afe_afeldo_off();
    hal_gafe_power_off();
    g_calibration_done = false;
}

static void hal_adc_v153_power_en(afe_scan_mode_t afe_scan_mode, bool on)
{
    if (on) {
        hal_adc_v153_power_on();
        hal_adc_v153_enable(afe_scan_mode);
        g_adc_first_sample = true;
    } else {
        hal_adc_v153_power_off(afe_scan_mode);
    }
}

__attribute__((weak)) void hal_cpu_trace_restart(void)
{
}

static int32_t hal_adc_v153_manual(adc_channel_t channel)
{
    UNUSED(channel);
    return 0;
}

static int32_t hal_adc_v153_auto_sample(adc_channel_t channel)
{
    g_gafe_cfg_amux_1.b.amuxp_sensor_ch_sel = BIT(channel);
    hal_gafe_channel_sel(&g_gafe_cfg_amux_1);
    hal_gadc_iso_on();
    while (!hal_gafe_single_sample_get_sts()) {}
    hal_gadc_iso_off();
    if (g_adc_first_sample) {
        hal_gadc_iso_on();
        while (!hal_gafe_single_sample_get_sts()) {}
        hal_gadc_iso_off();
        g_adc_first_sample = false;
    }
    return hal_gafe_sample_symbol_judge(hal_gafe_single_sample_get_value());
}

static const hal_adc_funcs_t g_hal_adc_v153_funcs = {
    .init = hal_adc_v153_init,
    .deinit = hal_adc_v153_deinit,
    .ch_set = hal_adc_v153_channel_set,
#if defined(CONFIG_ADC_SUPPORT_DIFFERENTIAL)
    .diff_ch_set = hal_adc_v153_differential_channel_set,
#endif
    .power_en = hal_adc_v153_power_en,
    .manual = hal_adc_v153_manual,
    .auto_sample = hal_adc_v153_auto_sample
};

void hal_adc_done_irq_handler(afe_scan_mode_t afe_scan_mode)
{
    unused(afe_scan_mode);
}

void hal_adc_alarm_irq_handler(afe_scan_mode_t afe_scan_mode)
{
    unused(afe_scan_mode);
}

hal_adc_funcs_t *hal_adc_v153_funcs_get(void)
{
    return (hal_adc_funcs_t *)&g_hal_adc_v153_funcs;
}
