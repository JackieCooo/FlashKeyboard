/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * Description: entry point of car_3in1_pos_inout_fus\n
 * Author:  \n
 * History: \n
 * 2024-04-10, Create file. \n
 */
#ifndef SLEM_ALG_COMMON_PARA_H
#define SLEM_ALG_COMMON_PARA_H
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
/**
 * @if Eng
 * @brief  SLEM total num of keys.
 * @else
 * @brief  SLEM 算法钥匙总数。
 * @endif
 */
#define ALG_KEY_NUM            2

/**
 * @if Eng
 * @brief  SLEM number of features selected by the algorithm.
 * @else
 * @brief  SLEM 算法选择训练特征个数。
 * @endif
 */
#define NUM_OF_FEATURES    8

/**
 * @if Eng
 * @brief  SLEM invalid ranging.
 * @else
 * @brief  SLEM 无效测距。
 * @endif
 */
#define ALG_INVALID_INDEX (-1)

/**
 * @if Eng
 * @brief  SLEM invalid ranging.
 * @else
 * @brief  SLEM 无效测距。
 * @endif
 */
#define ALG_DIS_INVALID                 (-1)

/**
 *
 * @if Eng
 * @brief  SLEM invalid rssi.
 * @else
 * @brief  SLEM 无效信号强度。
 * @endif
 */
#define ALG_RSSI_INVALID                (-128)
/**
 * @if Eng
 * @brief  SLEM invalid height information.
 * @else
 * @brief  SLEM 无效置信度中间信息。
 * @endif
 */
#define ALG_HEIGHT_INVALID              (-10)
/**
 * @if Eng
 * @brief  SLEM invalid position.
 * @else
 * @brief  SLEM 无效定位。
 * @endif
 */
#define ALG_POS_INVALID                 0
/**
 * @if Eng
 * @brief  SLEM number of secondary anchors required for algorithm.
 * @else
 * @brief  SLEM 算法中从锚点个数。
 * @endif
 */
#define ALG_SLAVE_NUM                   4
/**
 * @if Eng
 * @brief  number of anchors required for algorithm.
 * @else
 * @brief  SLEM 算法锚点个数。
 * @endif
 */
#define ALG_ANCHOR_NUM          5

/**
 * @if Eng
 * @brief  SLEM positioning information.
 * @else
 * @brief  SLEM 位置信息。
 * @endif
 */
typedef struct _tagPos {
    float x;        /*!< @if Eng x axis
                         @else   定位x轴坐标。 @endif */
    float y;        /*!< @if Eng y axis
                         @else   定位y轴坐标。 @endif */
    float z;        /*!< @if Eng z axis
                         @else   定位z轴坐标。 @endif */
} slem_tag_pos;

typedef struct _tagPosCfg {
    bool flag_init;                     /*!< @if Eng the flag of whether start the initiation of the positioning.
                                             @else   是否初始化标志。 @endif */
    uint8_t R_method;                   /*!< @if Eng the method to adjust the parameter R of the positioning algorithm.
                                             @else   定位算法调节参数R的方式。 @endif */
    float lr_space;                     /*!< @if Eng upper boundary of the square box that triggers the in-vehicle
                                                     positioning algorithm.
                                             @else   触发车内定位算法方形框的上边界。 @endif */
    float ud_space;                     /*!< @if Eng lpper boundary of the square box that triggers the in-vehicle
                                                     positioning algorithm.
                                             @else   触发车内定位算法方形框的下边界。 @endif */
    float car_in_r[ALG_ANCHOR_NUM];     /*!< @if Eng R value of each anchor point when the positioning result is inside
                                                     the car after the car's inside positioning algorithm is triggered.
                                             @else   车内定位算法触发后，定位结果为车内时，各个锚点的R值。 @endif */
    float car_out_r[ALG_ANCHOR_NUM];    /*!< @if Eng R value of each anchor point when the positioning result is outside
                                                     the car after the car's inside positioning algorithm is triggered.
                                             @else   车内定位算法触发后，定位结果为车外时，各个锚点的R值。 @endif */
} slem_pos_cfg;

typedef struct {
    float intercepts;                   /*!< @if Eng the 1st Parameter used for the detection of the inside and outside
                                                     of the car.
                                             @else   车内外识别参数1。 @endif */
    int16_t index[NUM_OF_FEATURES];   /*!< @if Eng the 6th Parameter used for the detection of the inside and outside
                                                     of the car.
                                             @else   车内外识别参数6。 @endif */
    float mean[NUM_OF_FEATURES];        /*!< @if Eng the 3rd Parameter used for the detection of the inside and outside
                                                     of the car.
                                             @else   车内外识别参数3。 @endif */
    float scale[NUM_OF_FEATURES];       /*!< @if Eng the 4th Parameter used for the detection of the inside and outside
                                                     of the car.
                                             @else   车内外识别参数4。 @endif */
    float coef[NUM_OF_FEATURES];        /*!< @if Eng the 5th Parameter used for the detection of the inside and outside
                                                     of the car.
                                             @else   车内外识别参数5。 @endif */
} slem_alg_config_car_in_out;

typedef struct {
    uint64_t cur_time;
    uint8_t key_id;

    uint8_t used_times;        /*!< @if Eng the times this data have been used in locate_key algorithm.
                                          @else   locate_key算法中数据使用的次数。 @endif */
    uint8_t invalid_times;     /*!< @if Eng the times this data is invalid in locate_key algorithm.
                                          @else   locate_key算法中数据无效的次数。 @endif */
    bool is_new_data;                /*!< @if Eng whether the data is the latest.
                                          @else   锚点数据是否是最新的。 @endif */
    bool is_valid_data;              /*!< @if Eng whether the data is valid in locate_key algorithm.
                                          @else   locate_key算法中数据是否有效。 @endif */
    float dis_org;                 /*!< @if Eng ranging data after smooth.
                                             @else   原始测距值。 @endif */
    float dis_smoothed;                 /*!< @if Eng ranging data after smooth.
                                             @else   平滑后测距值。 @endif */
    float dis_slight_smoothed;          /*!< @if Eng slight smooth of ranging result.
                                             @else   轻度平滑后测距值。 @endif */
    float prob;                         /*!< @if Eng confidence of ranging data after smooth.
                                             @else   平滑后测距的置信度。 @endif */
    float rssi;                         /*!< @if Eng received signal strength indication.
                                             @else   接收信号强度。 @endif */
    float height;                       /*!< @if Eng confidence intermediate information.
                                             @else   置信度中间信息。 @endif */
    uint8_t smooth_num;
} slem_alg_dis_info;

#endif