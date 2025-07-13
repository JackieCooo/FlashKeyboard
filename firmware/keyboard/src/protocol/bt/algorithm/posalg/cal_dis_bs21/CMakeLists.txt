#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
#===============================================================================
set(COMPONENT_NAME "cal_dis")

return_if_not_build()

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/KDE.c
    ${CMAKE_CURRENT_SOURCE_DIR}/basicMath.c
    ${CMAKE_CURRENT_SOURCE_DIR}/disalg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/smooth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/alg_smooth_dis.c
    
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cher2k_subfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/ctrmm_subfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cgemm_subfuc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/caxpy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/ccopy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cdotc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cdotu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cgemm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cgemv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cgerc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/chemv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cher2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cher2k.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cscal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/csscal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/cswap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/ctrmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/ctrmv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/ctrsv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/digits.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/epsilon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/huge.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/icamax.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/isamax.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/lsame.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/maxexponent.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/minexponent.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/radix.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/scabs1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/scasum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/scnrm2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/sscal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_blas_c/tiny.c

    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cgebak.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cgebal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cgeev.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cgehd2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cgehrd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cheev.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cheev_check.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/chetd2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/chetrd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/chseqr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clacgv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clacpy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cladiv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clahqr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clahr2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clange.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clanhe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/claqr0.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/claqr1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/claqr2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/claqr3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/claqr4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/claqr5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clarf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clarfb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clarfg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clarft.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clartg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clascl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/claset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clasr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/classq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clatrd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/clatrs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/crot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/csteqr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ctrevc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ctrevc3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ctrexc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cung2l.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cung2r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cunghr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cungql.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cungqr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cungtr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cunm2r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cunmhr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/cunmqr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ieeeck.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ilaclc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ilaclr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ilaenv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/iparmq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/sisnan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slabad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/sladiv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slae2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slaev2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slaisnan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slamch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slanst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slapy2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slapy3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slartg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slascl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slasrt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/slassq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/sroundup_lwork.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/ssterf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_lapack_c/xerbla.c

    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/cabs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/copysign.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/c_abs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/c_div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/c_sqrt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/i_len.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/i_nint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/pow_ci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/pow_ri.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/r_cnjg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/r_imag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/r_lg10.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/r_max.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/r_sign.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/s_cat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/s_cmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_f2c_c/s_copy.c
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/cal_dis_bs21_h/
    ${CMAKE_CURRENT_SOURCE_DIR}/bs21_dts/
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
set(LIB_OUT_PATH ${BIN_DIR}/${CHIP}/libs/bluetooth/btc/${TARGET_COMMAND})
build_component()
