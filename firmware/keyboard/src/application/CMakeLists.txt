#===============================================================================
# @brief    cmake make file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
#===============================================================================
add_subdirectory_if_exist(keyboard)

if("${CHIP}" STREQUAL "socmn1")
    add_subdirectory_if_exist(melody)
else()
    add_subdirectory_if_exist(${CHIP})
endif()

if("${CHIP}" STREQUAL "socmn1" OR "${CHIP}" STREQUAL "brandy")
    add_subdirectory_if_exist(audio)
endif()
