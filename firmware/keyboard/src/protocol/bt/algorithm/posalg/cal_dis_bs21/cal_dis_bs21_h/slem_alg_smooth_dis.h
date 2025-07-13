/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024. All rights reserved.
 *
 * Description: cal distance and smooth algorithm \n
 *
 * History: \n
 * 2024-06-21, Create file. \n
 */

#ifndef SLEM_ALG_SMOOTH_DIS_H
#define SLEM_ALG_SMOOTH_DIS_H

#include "slem_disalg.h"
#include "slem_smooth.h"

/**
 * @if Eng
 * @brief  Number of channels.
 * @else
 * @brief  信道数目。
 * @endif
 */
#define ALG_CHANNEL_NUM         79

/**
 * @if Eng
 * @brief  RSSI offset.
 * @else
 * @brief  RSSI偏移量。
 * @endif
 */
#define ALG_RSSI_OFFSET         256

/**
 * @if Eng
 * @brief  RSSI invalid value.
 * @else
 * @brief  RSSI无效值。
 * @endif
 */
#define ALG_RSSI_INVALID        (-128)

/**
 * @if Eng
 * @brief  Number 2.
 * @else
 * @brief  数值2。
 * @endif
 */
#define ALG_NUM_2               2

/**
 * @if Eng
 * @brief  Number 1000.
 * @else
 * @brief  数值1000。
 * @endif
 */
#define ALG_NUM_1000            1000

/**
 * @if Eng
 * @brief  SLEM smooth ranging parameters.
 * @else
 * @brief  SLEM平滑测距参数。
 * @endif
 */
typedef struct {
    uint32_t cur_time;                      /*!< @if Eng Current system time.
                                                 @else   当前系统时间。 @endif */
    slem_alg_iq *iq_dut;                    /*!< @if Eng IQ data received at initiator.
                                                 @else   测距发起方收到的IQ数据。 @endif */
    slem_alg_iq *iq_rtd;                    /*!< @if Eng IQ data received at reflector.
                                                 @else   测距响应方收到的IQ数据。 @endif */
    uint32_t tof_dut;                       /*!< @if Eng ToF data received at initiator.
                                                 @else   测距发起方收到的ToF数据。 @endif */
    uint32_t tof_rtd;                       /*!< @if Eng ToF data received at reflector.
                                                 @else   测距响应方收到的ToF数据。 @endif */
    uint32_t cur_count;                     /*!< @if Eng Count of ranging times.
                                                 @else   测距次数计数。 @endif */
    int tof_calib;                          /*!< @if Eng Calibration value of ToF.
                                                 @else   ToF的校准值。 @endif */
    float calib_val;                        /*!< @if Eng Calibration value of IQ.
                                                 @else   IQ的校准值。 @endif */
    uint8_t rssi_dut;                       /*!< @if Eng Received signal strength indication of initiator.
                                                 @else   测距发起方的接收信号强度。 @endif */
    uint8_t rssi_rtd;                       /*!< @if Eng Received signal strength indication of reflector.
                                                 @else   测距响应方的接收信号强度。 @endif */
    uint8_t key_id;                         /*!< @if Eng ID of key.
                                                 @else   钥匙编号。 @endif */
    slem_para_pair para_limit;              /*!< @if Eng Parameters of ranging algorithm. see @ref slem_para_pair.
                                                 @else   测距算法参数。 参考 @ref slem_para_pair。 @endif */
    slem_dis_alg_type ranging_method;       /*!< @if Eng Selection of ranging algorithm. see @ref slem_dis_alg_type.
                                                 @else   测距算法选择。 参考 @ref slem_dis_alg_type。 @endif */
} slem_alg_para_dis;

/**
 * @if Eng
 * @brief  SLEM smoothed distance results.
 * @else
 * @brief  SLEM 平滑后测距结果。
 * @endif
 */
typedef struct {
    float dis_smoothed;             /*!< @if Eng Smoothed distance value.
                                         @else   平滑后测距值。 @endif */
    float dis_ori;                  /*!< @if Eng Unsmoothed distance value.
                                         @else   未平滑测距值。 @endif */
    float dis_slight_smoothed;      /*!< @if Eng Slight smooth distance value.
                                         @else   轻度平滑测距值。 @endif */
    float prob;                     /*!< @if Eng Confidence of distance value.
                                         @else   测距值的置信度。 @endif */
    float rssi;                     /*!< @if Eng Received signal strength indication.
                                         @else   信号强度。 @endif */
    float height;                   /*!< @if Eng Confidence intermediate information.
                                         @else   置信度中间信息。 @endif */
    uint8_t smooth_num;             /*!< @if Eng Number of consecutive valid ranging times.
                                         @else   连续有效测距的次数。 @endif */
} slem_smoothed_dis_result;

/**
 * @if Eng
 * @brief  SLEM original distance results.
 * @else
 * @brief  SLEM 直接计算出来的原始测距结果。
 * @endif
 */
typedef struct {
    float dis_ori;                  /*!< @if Eng Unsmoothed distance value.
                                         @else   未平滑测距值。 @endif */
    float prob;                     /*!< @if Eng Confidence of distance value.
                                         @else   测距值的置信度。 @endif */
    float rssi;                     /*!< @if Eng Received signal strength indication.
                                         @else   信号强度。 @endif */
    float height;                   /*!< @if Eng Confidence intermediate information.
                                         @else   置信度中间信息。 @endif */
    uint32_t tof_dis;               /*!< @if Eng Distance value from time of flying.
                                         @else   通过飞行时间计算的测距值。 @endif */
} slem_original_dis_result;

/**
 * @if Eng
 * @brief  SLEM smooth ranging algorithm.
 * @par Description:
 * @param  [in]  res           SLEM smoothed ranging result. see @ref slem_smoothed_dis_result.
 * @param  [in]  alg_para      SLEM smooth ranging parameters. see @ref slem_alg_para_dis.
 * @retval error code. see @ref errcode_slem.
 * @par Dependency:
 *            @li slem_disalg.h
 *            @li slem_smooth.h
 * @else
 * @brief  SLEM平滑测距算法。
 * @par 说明:
 * @param  [in]  res            SLEM 平滑的测距结构. see @ref slem_smoothed_dis_result.
 * @param  [in]  alg_para       SLEM 平滑测距参数. see @ref slem_alg_para_dis.
 * @retval 执行结果错误码。参考 @ref errcode_slem。
 * @par 依赖:
 *            @li slem_disalg.h
 *            @li slem_smooth.h
 * @endif
 */
errcode_slem slem_alg_calc_smoothed_dis(slem_smoothed_dis_result *res, slem_alg_para_dis *alg_para);
#endif