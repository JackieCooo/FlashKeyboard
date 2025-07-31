#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
#===============================================================================
set(COMPONENT_NAME "cal_dis")

return_if_not_build()

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/ A1_DIR_VAR)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/ A2_DIR_VAR)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ A3_DIR_VAR)
 
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/KDE.c
    ${CMAKE_CURRENT_SOURCE_DIR}/basicMath.c
    ${CMAKE_CURRENT_SOURCE_DIR}/disalg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/smooth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/alg_smooth_dis.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stdlib/alg_cstdlib.c
    ${A1_DIR_VAR}
    ${A2_DIR_VAR}
    ${A3_DIR_VAR}
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/bs21_dts/
    ${CMAKE_CURRENT_SOURCE_DIR}/cal_dis_bs21_h/
    ${CMAKE_CURRENT_SOURCE_DIR}/cal_dis_bs21_h/cal_dis_matrix_h/
    ${CMAKE_CURRENT_SOURCE_DIR}/stdlib/
)

set(PRIVATE_HEADER
)

set(PRIVATE_DEFINES
)

set(PUBLIC_DEFINES
)

# use this when you want to add ccflags like -include xxx
set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
    # "-Wno-strict-prototypes"
    # "-Wno-parentheses"
    "-Wno-unused-parameter"
    # "-Wno-implicit-function-declaration"
    "-Wno-unused-but-set-variable"
    "-Wno-unused-variable"
    # "-Wno-comment"
    "-Wno-maybe-uninitialized"
    "-Wno-sign-compare"
    "-Wno-unused-value"
    # "-Wno-uninitialized"
    "-Wno-unused-function"
)

set(LIBS
)

execute_process(
    COMMAND ${MKDIR} ${BIN_DIR}/${CHIP}/libs/bluetooth/btc/${TARGET_COMMAND}
)

execute_process(
    COMMAND ${CP} ${LIBS} ${BIN_DIR}/${CHIP}/libs/bluetooth/btc/${TARGET_COMMAND}
)
set(WHOLE_LINK
    true
)

set(MAIN_COMPONENT
    false
)

set(PUBLIC_DEFINES

)

set(PRIVATE_DEFINES
    CONFIG_SLEM_CARKEY
)
install_sdk(${CMAKE_CURRENT_SOURCE_DIR}/cal_dis_bs21_h "*.h")
install_sdk(${CMAKE_CURRENT_SOURCE_DIR}/cal_dis_bs21_h/cal_dis_matrix_h "*.h")
install_sdk(${CMAKE_CURRENT_SOURCE_DIR}/stdlib "*.h")
set(LIB_OUT_PATH ${BIN_DIR}/${CHIP}/libs/bluetooth/btc/${TARGET_COMMAND})
build_component()
