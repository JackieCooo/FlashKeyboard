/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024. All rights reserved.
 *
 * Description: SLEM error code.
 */

/**
 * @defgroup slem_error_code Error Code API
 * @ingroup  SLEM
 * @{
 */

#ifndef SLEM_ERRCODE_H
#define SLEM_ERRCODE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @if Eng
 * @brief  SLEM error code.
 * @else
 * @brief  SLEM 错误码（非协议相关）,范围是0x8000A400 ~ 0x8000A800。
 * @endif
 */
typedef enum {
    // 公用错误码范围是0x8000A400 ~ 0x8000A450
    ERRCODE_SLEM_SUCCESS = 0,       /*!< @if Eng error code of success.
                                         @else   执行成功错误码。 @endif */
    ERRCODE_SLEM_FAIL = 0x8000A400, /*!< @if Eng error code of configure fail.
                                         @else   配置失败错误码。 @endif */
    ERRCODE_SLEM_MEMCPY_FAIL,       /*!< @if Eng error code of memcpy fail.
                                         @else   拷贝失败错误码。 @endif */
    ERRCODE_SLEM_MALLOC_FAIL,       /*!< @if Eng error code of malloc fail.
                                         @else   内存申请失败错误码。 @endif */
    ERRCODE_SLEM_ZERO_FAIL,         /*!< @if Eng error code of zero value.
                                          @else   0值错误码。 @endif */
    
    // 测距错误码范围是0x8000A450 ~ 0x8000A550
    ERRCODE_SLEM_RSSI_ABNORMAL = 0x8000A450, /*!< @if Eng error code of rssi abnormal.
                                       @else   RSSI异常。 @endif */
    ERRCODE_SLEM_MARIX_INV_FAIL,             /*!< @if Eng error code of matrix inverse fail.
                                                  @else   矩阵求逆失败错误码。 @endif */

    ERRCODE_SLEM_TOF_IQ_NOTMATCH, /*!< @if Eng The ToF distance is much greater than
                                               the IQ distance.
                                       @else   ToF测距值远大于IQ测距值。 @endif */
    ERRCODE_SLEM_IQ_LOW_ENERGY,   /*!< @if Eng The IQ energy is too low.
                                       @else   IQ能量幅度过低，难以计算距离。 @endif */
    
    // 定位错误码范围是0x8000A550 ~ 0x8000A650
    ERRCODE_SLEM_NOTRIGGER = 0x8000A550,      /*!< @if Eng hint of no trigger of ALGORITHM
                                                   @else   上层算法未触发提示 @endif */
    ERRCODE_SLEM_POS_FAIL, /*!< @if Eng error code of position algorithm fail.
                                @else   定位失败码。 @endif */
    // 开关门错误码范围是0x8000A650 ~ 0x8000A725
    ERRCODE_SLEM_FUSION_FAIL = 0x8000A650 ,           /*!< @if Eng error code of lock and unlock algorithm fail
                                                           @else   开关门算法失败码 @endif */
    // 车内外错误码范围是0x8000A725 ~ 0x8000A800
    ERRCODE_SLEM_CAR_IN_OUT_FAIL = 0x8000A725,       /*!< @if Eng error code of car in out algorithm fail
                                                          @else   开关门算法失败码 @endif */
} errcode_slem;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* SLE_ERRCODE_H */
/**
 * @}
 */
