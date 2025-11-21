#===============================================================================
# @brief    cmake make file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
#===============================================================================
add_subdirectory_if_exist(efuse)
add_subdirectory_if_exist(tcxo)
add_subdirectory_if_exist(sfc)
add_subdirectory_if_exist(watchdog)