#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
#===============================================================================
add_subdirectory_if_exist(delay)
add_subdirectory_if_exist(hash)
add_subdirectory_if_exist(malloc)
add_subdirectory_if_exist(watchdog)
add_subdirectory_if_exist(serial)
add_subdirectory_if_exist(reset)
