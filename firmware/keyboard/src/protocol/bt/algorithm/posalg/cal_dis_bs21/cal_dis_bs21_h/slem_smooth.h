/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024. All rights reserved.
 *
 * Description: cal distance and smooth algorithm \n
 *
 * History: \n
 * 2024-06-21, Create file. \n
 */

#ifndef SLEM_SMOOTH_H
#define SLEM_SMOOTH_H

#include <stdint.h>

/**
 * @if Eng
 * @brief  SLEM ranging smooth buffer size.
 * @else
 * @brief  SLEM测距平滑缓存长度。
 * @endif
 */
#define SMOOTH_BUFFER_BUTT 1

/**
 * @if Eng
 * @brief  SLEM ranging time and times information.
 * @else
 * @brief  SLEM测距时间和次数信息。
 * @endif
 */
typedef struct {
    uint64_t cur_time;              /*!< @if Eng Current system time.
                                         @else   当前系统时间。 @endif */
    uint32_t cur_count;             /*!< @if Eng Count of ranging times.
                                         @else   测距次数计数。 @endif */
} slem_para_cur_info;

/**
 * @if Eng
 * @brief  SLEM ranging results.
 * @else
 * @brief  SLEM测距结果。
 * @endif
 */
typedef struct {
    float dist_first;               /*!< @if Eng Preferred distance value.
                                         @else   首选测距值。 @endif */
    float dist_second;              /*!< @if Eng Candidate distance value.
                                         @else   候补测距值。 @endif */
} slem_cal_dis_res;

/**
 * @if Eng
 * @brief  Distance value smoothing.
 * @par Description:
 * @attention  1. smoothing mode flag = 1
 * @param  [in]  cur_info    SLEM ranging time and times information. see @ref slem_para_cur_info.
 * @param  [in]  dis_res     SLEM ranging results. see @ref slem_cal_dis_res.
 * @param  [in]  key_id      ID of key.
 * @retval Smoothed distance value.
 * @else
 * @brief  测距值平滑。
 * @par 说明:
 * @attention  1. 平滑方式 flag = 1。
 * @param  [in]  cur_info    SLEM测距时间和次数信息。 参考 @ref slem_para_cur_info。
 * @param  [in]  dis_res     SLEM测距结果。 参考 @ref slem_cal_dis_res。
 * @param  [in]  key_id      钥匙编号。
 * @retval 平滑后的测距值。
 * @endif
 */
float slem_alg_smooth_dis(slem_para_cur_info *cur_info, slem_cal_dis_res *dis_res, uint8_t key_id);

/**
 * @if Eng
 * @brief  confidence smoothing.
 * @par Description:
 * @attention  Confidence of the distance value. The value range is [0, 1].
               The closer to 1, the more reliable the distance value is.
 * @param  [in]  cur_info    SLEM ranging time and times information. see @ref slem_para_cur_info.
 * @param  [in]  prob        SLEM confidence of distance value.
 * @param  [in]  key_id      ID of key.
 * @retval Smoothed confidence.
 * @else
 * @brief  置信度平滑。
 * @par 说明:
 * @attention  测距值的置信度，取值范围[0,1]，越接近1表示测距值越可靠。
 * @param  [in]  cur_info    SLEM测距时间和次数信息。 参考 @ref slem_para_cur_info。
 * @param  [in]  prob        SLEM测距值置信度。
 * @param  [in]  key_id      钥匙编号。
 * @retval 平滑后的置信度。
 * @endif
 */
float slem_alg_smooth_prob(slem_para_cur_info *cur_info, float prob, uint8_t key_id);

/**
 * @if Eng
 * @brief  Smoothing of confidence intermediate information.
 * @par Description:
 * @attention  1. Used for car in out algorithm.
 * @param  [in]  height        SLEM confidence intermediate information.
 * @param  [in]  key_id        id of key.
 * @retval Smoothed confidence intermediate information.
 * @else
 * @brief  置信度中间信息平滑。
 * @par 说明:
 * @attention  1. 用于车内外检测。
 * @param  [in]  height        SLEM置信度中间信息。
 * @param  [in]  key_id        钥匙编号。
 * @retval 平滑后的置信度中间信息。
 * @endif
 */
float slem_alg_smooth_height(float height, uint8_t key_id);

/**
 * @if Eng
 * @brief  Smoothing of ToF distance value.
 * @par Description:
 * @param  [in]  tof_result     ToF distance value.
 * @param  [in]  key_id         ID of key.
 * @retval Smoothed ToF distance value.
 * @else
 * @brief  ToF测距值平滑。
 * @par 说明:
 * @param  [in]  tof_result     ToF测距值。
 * @param  [in]  key_id         钥匙编号。
 * @retval 平滑后的ToF测距值。
 * @endif
 */
uint32_t slem_alg_smooth_tof(uint32_t tof_result, uint8_t key_id);

/**
 * @if Eng
 * @brief  Get candidate distance value.
 * @par Description:
 * @param  [in]  key_id        ID of key.
 * @retval Candidate distance value.
 * @else
 * @brief  获取候选测距值。
 * @par 说明:
 * @param  [in]  key_id         钥匙编号。
 * @retval 候选测距值。
 * @endif
 */
float slem_alg_get_dis_double(uint8_t key_id);

/**
 * @if Eng
 * @brief  Number of consecutive valid ranging times.
 * @par Description:
 * @param  [in]  key_id      ID of key.
 * @retval Number of consecutive valid ranging times.
 * @else
 * @brief  连续有效测距的次数。
 * @par 说明:
 * @param  [in]  key_id      钥匙编号。
 * @retval 连续有效测距的次数。
 * @endif
 */
uint8_t slem_get_smooth_num(uint8_t key_id);

/**
 * @if Eng
 * @brief  Maximum number of abnormal values.
 * @par Description:
 * @attention  1. If the number of consecutive abnormal values reaches this value, the abnormal values are deleted.
 * @retval Maximum number of abnormal values.
 * @else
 * @brief  异常值次数上限。
 * @par 说明:
 * @attention  1. 异常值连续计数达到该数目，则进行剔除。
 * @retval 异常值次数上限。
 * @endif
 */
uint8_t slem_get_min_smooth_num(void);

/**
 * @if Eng
 * @brief  Restart smoothing algorithm.
 * @retval If the restart is success.
 * @else
 * @brief  重启平滑算法。
 * @endif
 */
void slem_init_smooth(void);

/**
 * @if Eng
 * @brief  Query the version number of the posalg.
 * @retval Posalg version number.
 * @else
 * @brief  查询测距算法版本号。
 * @endif
 */
int slem_get_posalg_version_number(void);

#endif