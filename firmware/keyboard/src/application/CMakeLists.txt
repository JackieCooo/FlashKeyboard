#===============================================================================
# @brief    cmake make file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
#===============================================================================
if("${CHIP}" STREQUAL "socmn1")
    add_subdirectory_if_exist(melody)
else()
    add_subdirectory_if_exist(${CHIP})
endif()
