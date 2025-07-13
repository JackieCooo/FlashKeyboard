/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Application core testsuite && AT init function \n
 *
 * History: \n
 * 2024-03-28, Create file. \n
 */
#ifdef TEST_SUITE
#include "timer.h"
#include "chip_core_irq.h"
#include "los_hwi.h"
#include "test_suite.h"
#include "test_suite_uart.h"
#include "test_auxiliary.h"
#include "chip_io.h"
#endif

#if defined(DEVICE_ONLY) && !defined(TEST_SUITE)
#include "wvt_uart.h"
#endif

#ifdef TEST_SUITE
#ifdef PLT_TEST_ENABLE
#include "test_keyscan.h"
#include "test_i2s.h"
#include "test_qdec.h"
#include "test_usb.h"
#include "test_pdm.h"
#endif
#ifdef FTRACE
#include "test_ftrace.h"
#endif

#if defined(CONFIG_AT_PTS_TEST_SUPPORT) && defined(CONFIG_AT_BLE) && (!defined(DEVICE_ONLY))
void bth_ble_dft_command_register(void);
#endif
#ifdef COREMARK_TEST
#include "core_portme.h"
#endif
#if (ENABLE_LOW_POWER == YES)
#include "pm_veto.h"
#endif
#include "memory_info.h"
#endif

#ifdef TESTSUIT_POS_DIS_ENABLE
#include "test_pos_dis.h"
#endif
#if defined(SUPPORT_CARKEY)
#include "carkey_api.h"
#include "carkey_uapi.h"
#endif

#ifdef AT_COMMAND
#include "at_product.h"
#include "at_porting.h"
#endif

#if defined(CONFIG_SUPPORT_NFC_SERVICE)
void add_nfc_t2t_test_case(void);
#endif

__attribute__((weak)) void main_test_init(void)
{
#if defined(TEST_SUITE)
    uapi_timer_init();
    uapi_timer_adapter(TIMER_INDEX_0, TIMER_0_IRQN, OS_HWI_PRIO_LOWEST - 1);
    test_suite_uart_init();
    uapi_test_suite_init();
#elif defined(AT_COMMAND)
    uapi_at_cmd_init();
#endif

#if defined(DEVICE_ONLY) && !defined(TEST_SUITE)
    wvt_uart_init();
#endif
}

#ifdef TEST_SUITE
#if (ENABLE_LOW_POWER == YES)
static int test_mcu_vote_sleep(int argc, char* argv[])
{
    unused(argc);
    uint8_t vote = (uint8_t)strtol(argv[0], NULL, 0);
    if (vote == 0) {
        uapi_pm_add_sleep_veto(PM_VETO_ID_MCU);
        writel(MEMORY_INFO_CRTL_REG, MEMORY_INFO_CLOSE);
    } else {
        uapi_pm_remove_sleep_veto(PM_VETO_ID_MCU);
    }
    return 0;
}
#endif

__attribute__((weak)) void cmd_main_add_functions(void)
{
    add_auxiliary_functions();
#ifdef AT_COMMAND
    uapi_test_suite_add_function("testsuite_sw_at", "<at>", uapi_testsuite_sw_at);
#endif
#ifdef COREMARK_TEST
    uapi_test_suite_add_function("coremark", "Coremark test Function", coremark_test);
#endif
#if (ENABLE_LOW_POWER == YES)
    uapi_test_suite_add_function("mcu_vote_slp", "MCU vote to sleep or not Function", test_mcu_vote_sleep);
#endif
#ifdef PLT_TEST_ENABLE
#ifndef CONFIG_BT_UPG_ENABLE
    add_usb_test_case();
#endif
#endif
#if defined(CONFIG_SUPPORT_NFC_SERVICE)
    add_nfc_t2t_test_case();
#endif
#ifdef FTRACE
    add_ftrace_test_case();
#endif
#ifdef TESTSUIT_POS_DIS_ENABLE
    add_pos_dis_test_case();
#endif
#ifdef SUPPORT_CARKEY
    add_sle_slem_nv_base_test_case();
    add_sle_slem_nv_alg_test_case();
#endif
}
#endif