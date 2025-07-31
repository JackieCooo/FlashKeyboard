/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * Description: app_alg api change\n
 * Author:  \n
 * History: \n
 * 2024-04-10, Create file. \n
 */
#ifndef SLEM_ALG_LOCATE_KEY_H
#define SLEM_ALG_LOCATE_KEY_H

#include "posalg.h"
#include "slem_alg_common_para.h"
#include "slem_errcode.h"
/**
 * @if Eng
 * @brief  SLEM distance required for opening the door.
 * @else
 * @brief  SLEM 开关门算法的开门距离。
 * @endif
 */
#define UNLOCK_DIS 3
/**
 * @if Eng
 * @brief  SLEM distance required for closing the door.
 * @else
 * @brief  SLEM 开关门算法的关门距离。
 * @endif
 */
#define LOCK_DIS 7
/**
 * @if Eng
 * @brief  SLEM distance required for starting the inside and outside car identification algorithm.
 * @else
 * @brief  SLEM 车内外识别算法的启动距离。
 * @endif
 */
#define IN_OUT_START_DIS 3
/**
 * @if Eng
 * @brief  SLEM distance required for disabling the inside and outside car identification algorithm.
 * @else
 * @brief  SLEM 车内外识别算法的停用距离。
 * @endif
 */
#define IN_OUT_STOP_DIS 5
/**
 * @if Eng
 * @brief  SLEM number of slave anchors required for
 *         starting the inside and outside car identification algorithm.
 * @else
 * @brief  SLEM 启动车内外识别算法所需从锚点个数。
 * @endif
 */
#define ALG_CAR_IN_OUT_TRIGGER_NUM      2

/**
 * @if Eng
 * @brief  SLEM number of secondary anchors required for starting the positioning algorithms.
 * @else
 * @brief  SLEM 启动定位算法所需从锚点个数
 * @endif
 */
#define ALG_POS_TRIGGER_NUM             3

/**
 * @if Eng
 * @brief  SLEM number of anchors required for starting the positioning algorithms.
 * @else
 * @brief  SLEM 启动融合算法所需锚点个数。
 * @endif
 */
#define ALG_FUSION_TRIGGER_NUM          1
/**
 * @if Eng
 * @brief  SLEM number of training features required for inside
 *         and outside car identification algorithm.
 * @else
 * @brief  SLEM 车内外识别算法训练特征个数。
 * @endif
 */
#define ALG_ALL_FEATURE_NUM             15
/**
 * @if Eng
 * @brief  SLEM number of training features required for inside
 *         and outside car identification algorithm.
 * @else
 * @brief  SLEM 车内外识别算法训练特征个数。
 * @endif
 */
#define ALG_NV_NUM_OF_R                 12
/**
 * @if Eng
 * @brief  SLEM maximum number of anchor points that can be configured for the nv.
 * @else
 * @brief  SLEM NV可以配置的最大锚点数量。
 * @endif
 */
#define ALG_NV_ANCHOR_NUM               7
/**
 * @if Eng
 * @brief  SLEM number of parameters configured for inside and outside identification.
 * @else
 * @brief  SLEM 车内外识别配置参数个数。
 * @endif
 */
#define CAR_IN_OUT_CFG_NUM              6
/**
 * @if Eng
 * @brief  SLEM dimension of positioning algorithm.
 * @else
 * @brief  SLEM 定位算法维度。
 * @endif
 */
#define POSALG_MEASURE_DEMISION         3
/**
 * @if Eng
 * @brief  SLEM the limit of anchor distance information failure.
 * @else
 * @brief  SLEM 锚点距离信息失效的界限。
 * @endif
 */
#define ALG_DIS_DATA_CLEAR_TRIGGER_NUM  3
/**
 * @if Eng
 * @brief  SLEM number of Invalid Location Attempts Triggering EKF Restart.
 * @else
 * @brief  SLEM 触发EKF重启的无效定位次数。
 * @endif
 */
#define ALG_EKF_INIT_THRESHOLD          6
/**
 * @if Eng
 * @brief  SLEM x-axis index.
 * @else
 * @brief  SLEM x轴脚标。
 * @endif
 */
#define AXIS_X_INDEX                    0
/**
 * @if Eng
 * @brief  SLEM y-axis index.
 * @else
 * @brief  SLEM y轴脚标。
 * @endif
 */
#define AXIS_Y_INDEX                    1
/**
 * @if Eng
 * @brief  SLEM z-axis index.
 * @else
 * @brief  SLEM z轴脚标。
 * @endif
 */
#define AXIS_Z_INDEX                    2
/**
 * @if Eng
 * @brief  SLEM anchor A index.
 * @else
 * @brief  SLEM 锚点A脚标。
 * @endif
 */
#define ANCHOR_A_INDEX                  0
/**
 * @if Eng
 * @brief  SLEM anchor B index.
 * @else
 * @brief  SLEM 锚点B脚标。
 * @endif
 */
#define ANCHOR_B_INDEX                  1
/**
 * @if Eng
 * @brief  SLEM anchor C index.
 * @else
 * @brief  SLEM 锚点C脚标。
 * @endif
 */
#define ANCHOR_C_INDEX                  2
/**
 * @if Eng
 * @brief  SLEM number 1000, used for the print of float number.
 * @else
 * @brief  SLEM 数字1000，用于log的float形式数据的打印。
 * @endif
 */
#define ALG_NUM_CARRY_1000              1000
/**
 * @if Eng
 * @brief  SLEM minimum precision, which is used to determine the value of the float type.
 * @else
 * @brief  SLEM 最小精度，用于float类型数值判断。
 * @endif
 */
#define POSALG_POS_ERR_ACCURACY         0.0001
/**
 * @if Eng
 * @brief  SLEM float 0.
 * @else
 * @brief  SLEM float类型数字0。
 * @endif
 */
#define FLOAT_ZERO                      0.
/**
 * @if Eng
 * @brief  SLEM upper threshold for the number of invalid location results,
 *         which is one of the conditions for triggering forcible location and releasing space.
 * @else
 * @brief  SLEM 无效定位结果计数上限，触发强制定位以及释放空间的条件之一。
 * @endif
 */
#define COUNTER_NAN_POS_UPPER_LIMIT    15
/**
 * @if Eng
 * @brief  SLEM upper limit of the number of invalid in-vehicle and out-vehicle recognition results,
           which is one of the conditions for triggering space release.
 * @else
 * @brief  SLEM 无效车内外识别结果计数上限，触发释放空间的条件之一。
 * @endif
 */
#define COUNTER_CAR_IN_UPPER_LIMIT     7
/**
 * @if Eng
 * @brief  SLEM one of the conditions for triggering forcible positioning and releasing space.
 * @else
 * @brief  SLEM 触发强制定位以及释放空间的条件之一。
 * @endif
 */
#define ALG_NAN_POS_TRIGGER_MINDIS     3

/**
 * @if Eng
 * @brief  SLEM the latest data infomation.
 * @else
 * @brief  SLEM 最新到来的数据信息。
 * @endif
 */
typedef struct {
    uint8_t key_id;        /*!< @if Eng No. of current key.
                                             @else   当前钥匙的编号。 @endif */
    uint8_t anc_id;        /*!< @if Eng No. of current anchor.
                                             @else   当前锚点的编号。 @endif */
    bool refresh_ekf_flag; /*!< @if Eng whether restart positioning filter.
                                             @else   是否重启定位平滑。 @endif */
} slem_alg_data_info;

/**
 * @if Eng
 * @brief  SLEM information required by the location algorithm
 * @else
 * @brief  SLEM 定位算法所需信息
 * @endif
 */
typedef struct {
    float x[ALG_NV_ANCHOR_NUM];                 /*!< @if Eng x-axis information of each anchor point.
                                                     @else   各个锚点x轴信息。 @endif */
    float y[ALG_NV_ANCHOR_NUM];                 /*!< @if Eng y-axis information of each anchor point.
                                                     @else   各个锚点y轴信息。 @endif */
    float z[ALG_NV_ANCHOR_NUM];                 /*!< @if Eng z-axis information of each anchor point.
                                                     @else   各个锚点z轴信息。 @endif */
    float adjust_dis_pos[ALG_NV_ANCHOR_NUM];    /*!< @if Eng positioning correction value information.
                                                      of each anchor point //revise adjustment/correction.
                                                     @else   各个锚点定位修正值信息。 @endif */
} slem_alg_config_pos;

/**
 * @if Eng
 * @brief  SLEM information required by the locate key algorithm
 * @else
 * @brief  SLEM 搜索钥匙算法返回结果，profile层中引用了此结构体作为消息payload
 * @endif
 */
typedef struct {
    slem_tag_pos pos_result; /*!< @if Eng positioning result.
                                  @else   定位算法结果。 @endif */
    int inout_result;        /*!< @if Eng the inside and outside of the car detection result.
                                  @else   车内外识别算法结果。 @endif */
    float lock_unlock_dis;        /*!< @if Eng distance result of the door opening and closing algorithm.
                                  @else   开关门算法距离结果。 @endif */
    int lock_unlock_key;          /*!< @if Eng optimal anchor point selected by door opening and closing algorithm.
                                  @else   开关门算法选定的最优锚点。 @endif */
    bool lock_unlock_state; /*!< @if Eng the state of door opening and closing decided by door opening and closing
                            algorithm.
                            @else   开关门算法决定的开关门状态。 @endif */
    bool pos_flag; /*!< @if Eng Record whether the positioning algorithm has been triggered.
                            @else   记录定位算法是否被触发。 @endif */
    bool inout_flag; /*!< @if Eng the Record whether the car detection algorithm has been triggered.
                            @else   记录车内外识别算法是否被触发。 @endif */
    bool lock_unlock_flag; /*!< @if Eng Record whether the door opening and closing algorithm has been triggered.
                            @else   记录开关门算法是否被触发。 @endif */
} slem_alg_locate_key_result;

typedef struct {
    slem_alg_config_car_in_out *parameters;              /*!< @if Eng parameters for configuring inside
                                                                     and outside identification algorithm.
                                                              @else   车内外识别算法配置参数。 @endif */
    slem_alg_config_pos *con_ext;                        /*!< @if Eng configuration parameters of positioning algorithm.
                                                                      see @ref slem_alg_config_pos.
                                                              @else   定位算法配置参数。
                                                                      参考 @ref slem_alg_config_pos。 @endif */
    bool enable_det_in_out;                              /*!< @if Eng whether to enable the inside and outside
                                                                 car identification algorithm.
                                                              @else   是否开启车内外识别算法。 @endif */
    uint8_t anchor_used_num;                             /*!< @if Eng configuration parameters of door opening/closing
                                                                     algorithm.
                                                              @else   开关门算法配置参数。 @endif */
    uint8_t select_door_idx_method;                      /*!< @if Eng independent door opening/closing mode.
                                                              @else   独立开关门方式。 @endif */ // g_discbk_flag
} slem_alg_config_pack;

/**
 * @if Eng
 * @brief  SLEM information required by the lock and unlock car algorithm
 * @else
 * @brief  SLEM 开挂们算法返回结果
 * @endif
 */
typedef struct {
    int lock_unlock_key;          /*!< @if Eng optimal anchor point selected by door opening and closing algorithm.
                                @else   开关门算法选定的最优锚点。 @endif */
    float lock_unlock_dis; /*!< @if Eng distance result of the door opening and closing algorithm.
                           @else   开关门算法距离结果。 @endif */
    bool lock_unlock_state; /*!< @if Eng the state of door opening and closing decided by door opening and closing
                            algorithm.
                            @else   开关门算法决定的开关门状态。 @endif */
} slem_lock_unlock_res;

/**
 * @if Eng
 * @brief  Interface for merging and triggering the inside and outside positioning algorithm,
           inside and outside identification algorithm, and ranging lock_unlock algorithm.
 * @par Description:
 * @attention  1.Not all three algorithms have results in each round.
 * @attention  2.If the positioning algorithm is disabled, the result accuracy of the
 *               ranging lock_unlock algorithm is affected.
 * @param  [in]  dis                  basic information such as the current distance measurement
 *                                    and confidence of each anchor. see @ref slem_alg_dis_struct_t.
 * @param  [in]  conf_p               nv configuration algorithm information. see @ref slem_alg_config_pack.
 * @param  [in]  prifile_info         control parameters sent by the profile layer. see @ref slem_alg_profile_info.
 * @param  [in]  locate_key_result    Positioning results, inside and outside identification results, and distance
 *                                    values for opening and closing doors. see @ref slem_alg_locate_key_result.
 * @retval Execution Result Error Code.
 * @par Dependency:
 *            @li posalg.h
 *            @li pipeline.h
 *            @li slem_alg_common_para.h
 * @else
 * @brief  合并触发车内外定位算法，车内外识别算法以及测距融合算法接口。
 * @par 说明:
 * @attention  1.不一定每一轮三个算法都有结果。
 * @attention  2.若关闭定位算法则会影响测距融合算法结果精度。
 * @param  [in]  dis                各个锚点本轮测距，置信度等基础信息，参考 @ref slem_alg_dis_struct。
 * @param  [in]  conf_p             at配置算法信息，参考 @ref slem_alg_config_pack。
 * @param  [in]  prifile_info       profile层下发的控制参数，参考 @ref slem_alg_profile_info。
 * @param  [in]  locate_key_result  本轮定位结果，车内外识别结果以及用于开关门的距离值，参考 @ref
 *                                  locate_key_result。
 * @par Description:
 * @retval 执行结果错误码。
 * @par 依赖:
 *            @li posalg.h
 *            @li pipeline.h
 *            @li slem_alg_common_para.h
 * @endif
 */
errcode_slem slem_alg_locate_key(slem_alg_dis_info *dis, slem_alg_config_pack *conf_p, slem_alg_data_info *data_info);

/**
 * @if Eng
 * @brief  Interface for triggering the positioning algorithm.
 * @par Description:
 * @attention  1.Algorithms may not have results in each round.
 * @param  [in]  pos_result    Positioning results.
 * @retval Execution Result Error Code.
 * @par Dependency:
 *            @li posalg.h
 *            @li pipeline.h
 *            @li slem_alg_common_para.h
 * @else
 * @brief  触发定位算法接口。
 * @par 说明:
 * @attention  1.不一定每一轮算法都有结果。
 * @param  [in]  pos_result  本轮定位结果，参考 @ref
 *                                  errcode_slem_locate_key。
 * @par Description:
 * @retval 执行结果错误码。
 * @par 依赖:
 *            @li slem_alg_common_para.h
 * @endif
 */
errcode_slem slem_alg_get_position(slem_tag_pos *pos_res);

/**
 * @if Eng
 * @brief  Interface for triggering the lock and unlock car algorithm.
 * @par Description:
 * @attention  1.Algorithms may not have results in each round.
 * @attention  2.If the positioning algorithm is disabled, the result accuracy of the
 *               lock and unlock car algorithm is affected.
 * @param  [in]  pos_result    The distance values for locking and unlocking doors.
 *                             see @ref slem_alg_locate_key_result.
 * @retval Execution Result Error Code.
 * @par Dependency:
 *            @li posalg.h
 *            @li pipeline.h
 *            @li slem_alg_common_para.h
 * @else
 * @brief  触发解闭锁算法接口。
 * @par 说明:
 * @attention  1.不一定每一轮算法都有结果。
 * @attention  2.若关闭定位算法则会影响解闭锁算法结果精度。
 * @param  [in]  pos_result  本轮用于解闭锁的距离值，参考 @ref
 *                                  errcode_slem_locate_key。
 * @par Description:
 * @retval 执行结果错误码。
 * @par 依赖:
 *            @li slem_alg_common_para.h
 * @endif
 */
errcode_slem slem_alg_get_lockunlock(slem_lock_unlock_res *lock_unlock_res);

/**
 * @if Eng
 * @brief  Interface for triggering the inside and outside identification algorithm.
 * @par Description:
 * @attention  1.Algorithms may not have results in each round.
 * @param  [in]  pos_result    inside and outside identification results.
 * @retval Execution Result Error Code.
 * @par Dependency:
 *            @li posalg.h
 *            @li pipeline.h
 *            @li slem_alg_common_para.h
 * @else
 * @brief  触发车内外识别算法接口。
 * @par 说明:
 * @attention  1.不一定每一轮算法都有结果。
 * @param  [in]  pos_result  本轮车内外识别结果。
 * @par Description:
 * @retval 执行结果错误码。
 * @par 依赖:
 *            @li slem_alg_common_para.h
 * @endif
 */
errcode_slem slem_alg_get_carinout(int *car_in_out_res);

/**
 * @if Eng
 * @brief  Used by AT commands to configure parameters for locking and unlocking cars.
 * @par Description:
 * @param  [in]  alpha_lock_unlock           parameter for locking and unlocking the car
 * @else
 * @brief  用于at命令配置解闭锁参数。
 * @par 说明:
 * @param  [in]  alpha_lock_unlock           解闭锁参数。
 * @par Description:
 * @endif
 */
void slem_set_alpha_lock_unlock(float alpha_lock_unlock);

/**
 * @if Eng
 * @brief  Used by AT commands to configure the distance of locking and unlocking the car.
 * @par Description:
 * @param  [in]  unlock_dis           lock distance
 * @param  [in]  lock_dis             unlock distance.
 * @else
 * @brief  用于at命令配置解闭锁距离的参数。
 * @par 说明:
 * @param  [in]  unlock_dis           闭锁距离。
 * @param  [in]  lock_dis             解锁距离。
 * @par Description:
 * @endif
 */
void slem_set_lock_unlock_dis(float unlock_dis, float lock_dis);

#endif
