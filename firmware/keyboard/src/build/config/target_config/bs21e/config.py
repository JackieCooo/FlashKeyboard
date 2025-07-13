#!/usr/bin/env python3
# encoding=utf-8
# ============================================================================
# @brief    Target Definitions File
# Copyright HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
# ============================================================================

target = {
    'standard-bs21e-1100e': {
        'base_target_name': 'target_bs21e_application_template',
        'pkg_chip': 'bs21e-1100e',
        'defines': [
            'SUPPORT_CFBB_UPG', 'BGLE_TASK_EXIST', 'SUPPORT_MULTI_LIBS', 'SW_UART_DEBUG', 'XO_32M_CALI',
            'SUPPORT_SET_KEYS', 'SUPPORT_SFC_IRQ_LOCK', 'CONFIG_OTA_UPDATE_SUPPORT',
        ],
        'ram_component': [
            # Application Region
            'main_app', 'standard_porting',

            # Middleware Region
            'dfx_porting', 'algorithm', 'app_init', 'lzma_21.07',
            'dfx_file_operation', 'pm_sys',
            'update_common', 'update_local', 'update_storage', 'update_common_porting', 'update_storage_porting',
            'ota_upgrade',

            # Stack Region
            'bg_common', 'bt_host', 'bth_sdk', 'bts_header', 'bth_gle',
            'bt_app', 'bgtp',

            # Drivers Region
            'mips', 'drv_timer', 'hal_timer', 'timer_port',
            'systick_port', 'tcxo_port', 'sfc_porting', 'std_rom_lds_porting',
            'rtc_unified', 'hal_rtc_unified', 'rtc_unified_port',

            # OS Region
            'liteos_208_6_0_b017',

            # Deleted Region
            '-:libboundscheck', '-:test_usb_unified', 'osal_adapt',
            '-:at', '-:at_cmd_port', '-:at_plt_cmd', '-:at_btc_cmd',

            # libm.a of the musl replaces libsegger.a
            '-:segger_b090_fp',
        ],
        'rom_component': ['bgtp_rom', 'libboundscheck', 'bt_host_rom', 'bg_common_rom'],
        'ram_component_set' : [
            'efuse_v151', 'spi', 'qdec', 'sio_v151', 'dmav151', 'std_common_lib',
            '-:connectivity', '-:time_set', 'pm_set', 'dfx_set', 'sfc_flash', 'adc', 'flash', 'pm_clock_set', 'pwm',
        ],
        'liteos_kconfig': 'bs21e',
        'loaderboot_cfg': 'loaderboot-bs21e-1100e',
        'flashboot_cfg': 'flashboot-bs21e-1100e',
        'sector_cfg': 'bs21e-standard',
        'nv_cfg': 'bs21e_nv_default',
        'upg_pkg': ['application'],
    },
}

# custom copy rules, put it in target_group below and it takes effect.
# <root> means root path
# <out_root> means output_root path
# <pack_target> means target_group key_name (like pack_bs21e_standard)
target_copy = {

}

target_group = {

}