/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024. All rights reserved.
 *
 * Description: cal distance and smooth algorithm
 *
 * History:
 * 2024-06-21, Create file.
 */

#ifndef SLEM_DISALG_H
#define SLEM_DISALG_H

#include "stdint.h"
#include "slem_errcode.h"

/**
 * @if Eng
 * @brief  IQ data processing threshold.
 * @else
 * @brief  IQ数据解析门限。当前芯片是11bit数据有效位，其中第11bit为符号位，10bit数据位。
 * @endif
 */
#define ALG_IQ_FLIP_BARRIER     0x03ff

/**
 * @if Eng
 * @brief  IQ data processing threshold.
 * @else
 * @brief  IQ数据解析门限。
 * @endif
 */
#define ALG_IQ_FLIP_OFFSET      0x0800

/**
 * @if Eng
 * @brief  IQ energy processing threshold.
 * @else
 * @brief  IQ数据有效门限，当IQ绝对值小于该值被认为是IQ幅度过小，可能是异常需要处理
 * @endif
 */
#define ALG_IQ_LOW_ENERGY_BARRIER   5

/**
 * @if Eng
 * @brief  Classification of SLEM algorithms.
 * @else
 * @brief  SLEM算法分类。
 * @endif
 */
typedef enum {
    METHOD_1M = 1,         /*!< @if Eng 1MHz channel frequency hopping algorithm (upper limit of ranging: 150m).
                                @else   信道1MHz跳频算法(测距上限150m)。 @endif */
    METHOD_2M,             /*!< @if Eng 2MHz channel frequency hopping algorithm.
                                @else   信道2MHz跳频算法。 @endif */
    METHOD_1M_2M,          /*!< @if Eng Low-complexity algorithm for 1MHz channel frequency hopping.
                                 (upper limit of ranging: 75 m)
                                @else   信道1MHz跳频低复杂度算法(测距上限75m)。 @endif */
    METHOD_ADJ_R_END,       /*!< @if Eng Channel 1 MHz frequency hopping dynamic r algorithm
                                 (upper limit of ranging: 150m).
                                @else   信道1MHz跳频动态r算法(测距上限150m)。 @endif */
    METHOD_ADJ_R_END_V2, /*!< @if Eng Channel 1 MHz frequency hopping dynamic r algorithm
                               (version2, upper limit of ranging: 150m).
                              @else   信道1MHz跳频动态r算法(版本2，测距上限150m)。 @endif */
    METHOD_ADJ_R_END_V3         /*!< @if Eng Channel 1 MHz frequency hopping dynamic r algorithm
                            (version3, upper limit of ranging: 150m).
                                     @else   信道1MHz跳频动态r算法(版本3，测距上限150m)。 @endif */
} slem_dis_alg_type;

/**
 * @if Eng
 * @brief  Parameters of ranging algorithm.
 * @else
 * @brief  测距算法参数。
 * @endif
 */
typedef struct {
    int8_t rssi_limit;                /*!< @if Eng RSSI abnormal value exclusion threshold.
                                         @else   RSSI异常值剔除门限。 @endif */
    uint8_t r_start;                /*!< @if Eng Ranging paramater 2, default value 2.
                                         @else   测距参数2, 默认值2。 @endif */
    float threshold_cond2;          /*!< @if Eng Ranging paramater 1, default value 20.
                                         @else   测距参数1, 默认值20。 @endif */
} slem_para_pair;

/**
 * @if Eng
 * @brief  IQ data.
 * @else
 * @brief  IQ数据。
 * @endif
 */
typedef struct {
    uint16_t i_data;          /*!< @if Eng I data.
                                   @else   I数据。 @endif */
    uint16_t q_data;          /*!< @if Eng Q data.
                                   @else   Q数据。 @endif */
} slem_alg_iq;

/**
 * @if Eng
 * @brief  SLEM ranging algorithm input parameters.
 * @else
 * @brief  SLEM测距算法入参。
 * @endif
 */
typedef struct {
    slem_para_pair para_limit;              /*!< @if Eng Parameters of ranging algorithm. see @ref slem_para_pair.
                                                 @else   测距算法参数。 参考 @ref slem_para_pair。 @endif */
    slem_alg_iq *iq_dut;                         /*!< @if Eng IQ data received at initiator.
                                                 @else   测距发起方收到的IQ数据。 @endif */
    slem_alg_iq *iq_rtd;                         /*!< @if Eng IQ data received at reflector.
                                                 @else   测距响应方收到的IQ数据。 @endif */
    int rssi_dut;                           /*!< @if Eng Received signal strength indication of initiator.
                                                 @else   测距发起方的接收信号强度。 @endif */
    int rssi_rtd;                           /*!< @if Eng Received signal strength indication of reflector.
                                                 @else   测距响应方的接收信号强度。 @endif */
    int tof_result;                         /*!< @if Eng ToF data.
                                                 @else   ToF数据。 @endif */
    int tof_calib;                          /*!< @if Eng Calibration value of ToF.
                                                 @else   ToF的校准值。 @endif */
    float calib_val;                        /*!< @if Eng Calibration value of IQ.
                                                 @else   IQ的校准值。 @endif */
    slem_dis_alg_type ranging_method;       /*!< @if Eng Selection of ranging algorithm. see @ref slem_dis_alg_type.
                                                 @else   测距算法选择。 参考 @ref slem_dis_alg_type。 @endif */
} slem_para_dis_calc;

/**
 * @if Eng
 * @brief  SLEM distance results.
 * @else
 * @brief  SLEM测距结果。
 * @endif
 */
typedef struct {
    float dist_first;               /*!< @if Eng Preferred distance value.
                                         @else   首选测距值。 @endif */
    float dist_second;              /*!< @if Eng Candidate distance value.
                                         @else   候补测距值。 @endif */
    float prob;                     /*!< @if Eng Confidence of distance value.
                                         @else   测距值的置信度。 @endif */
    float rssi;                     /*!< @if Eng RSSI.
                                         @else   信号强度。 @endif */
    float height;                   /*!< @if Eng Confidence intermediate information.
                                         @else   置信度中间信息。 @endif */
    int rssi_dut;                   /*!< @if Eng Received signal strength indication of initiator.
                                         @else   测距发起方的接收信号强度。 @endif */
    int rssi_rtd;                   /*!< @if Eng Received signal strength indication of reflector.
                                         @else   测距响应方的接收信号强度。 @endif */
} slem_dis_result;

/**
 * @if Eng
 * @brief  SLEM smooth ranging algorithm.
 * @par Description:
 * @param  [in]  result     SLEM ranging result. see @ref slem_dis_result.
 * @param  [in]  para_ec    SLEM ranging algorithm input parameters. see @ref slem_para_dis_calc.
 * @retval error code. see @ref errcode_slem.
 * @else
 * @brief  SLEM测距算法。
 * @par Description:
 * @param  [in]  result      SLEM测距结果。 参考 @ref slem_dis_result.
 * @param  [in]  para_ec     SLEM测距算法入参。 参考 @ref slem_para_cur_info。
 * @retval 执行结果错误码。参考 @ref errcode_slem。
 * @endif
 */
errcode_slem slem_alg_calc_dist_by_iq(slem_dis_result *result, slem_para_dis_calc *para_ec);

/**
 * @if Eng
 * @brief  SLEM raning by TOF & phase.
 * @par Description:
 * @param  [in]  result     SLEM ranging result. see @ref slem_dis_result.
 * @param  [in]  tof_result SLEM tof result
 * @param  [in]  tof_calib  SLEM calibration value
 * @retval error code. see @ref errcode_slem.
 * @else
 * @brief  SLEM 通过IQ和TOF测距值移位融合，防中继保护。
 * @par Description:
 * @param  [in]  result     SLEM测距结果。 参考 @ref slem_dis_result.
 * @param  [in]  tof_result SLEM 计算的TOF距离
 * @param  [in]  tof_calib  SLEM 传入的TOF校准值
 * @retval 执行结果错误码。参考 @ref errcode_slem。
 * @endif
 */
// IQ和TOF测距值移位融合，防中继保护
errcode_slem iq_tof_dis_merge(slem_dis_result *result, int tof_result, int tof_calib);
#endif
