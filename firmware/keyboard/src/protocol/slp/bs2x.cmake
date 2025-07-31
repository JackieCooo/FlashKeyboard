#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
#===============================================================================
set(COMPONENT_NAME "slp_client")

set(MODULE_NAME "slp")
set(AUTO_DEF_FILE_ID TRUE)


#加载子目录
add_subdirectory_if_exist(narrow_band)
add_subdirectory_if_exist(common)
add_subdirectory_if_exist(system)

set(SOURCES
    ${GLP_CLIENT_LIST}
)

set(PUBLIC_HEADER
    ${GLP_CLIENT_PUBLIC_HEADER_LIST}
    ${GLP_SYSTEM_HEADER_LIST}
)

set(PRIVATE_HEADER
    ${GLP_CLIENT_HEADER_LIST}
    ${GLP_COMMON_HEADER_LIST}
)

set(PUBLIC_DEFINES
)

set(PRIVATE_DEFINES
)

# use this when you want to add ccflags like -include xxx
set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
)

set(WHOLE_LINK
    true
)

set(MAIN_COMPONENT
    false
)

set(BUILD_AS_OBJ
    false
)

set(LOG_DEF
    ${CMAKE_CURRENT_SOURCE_DIR}/../../middleware/chips/bs2x/dfx/include/log_def_slp.h
)

#指定静态库生成位置
set(LIB_OUT_PATH ${BIN_DIR}/${CHIP}/libs/slp/${TARGET_COMMAND})
return_if_not_build()

set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_COMMAND}/lib${COMPONENT_NAME}.a")
if(EXISTS ${LIB_PATH})
    set(LIBS ${LIB_PATH})
else()
    message(WARNING "Library ${LIB_PATH} does not exist, skipping.")
endif()

build_component()


