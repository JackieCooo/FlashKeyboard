#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
#===============================================================================
add_subdirectory_if_exist(flashboot_linker)
add_subdirectory_if_exist(loaderboot_linker)
add_subdirectory_if_exist(standard)
add_subdirectory_if_exist(turnkey_mouse)