/*
 * @file posalg.h
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2024. All rights reserved.
 * Description: interface of position algorithom.
 * Create: 2023-7-20
 */
#ifndef __POS_ALG_H__
#define __POS_ALG_H__
#include "slem_alg_common_para.h"
#include "slem_errcode.h"

/* get position algorithm
   If you want to change the algorithm version, see the header file posver.h
   param:
         anchorPara:  Position of anchors(5 * 3 array)
         dMeas:       Measured distance for each anchor(1 * 5 array)
         dMeasNum:    element num of dMeas(5/4)
         probMeas:    Probability of distances for each anchor(1 * 5 or 1 * 4 array)
         probMeasNum: element num of probMeas(5/4)
         cfg:         config of pos alg
   return:
         value of dimension x and dimension y and dimension z
*/

typedef struct {
    float *dMeas;                       /*!< @if Eng the ranging information of all anchor.
                                             @else   所有锚点测距值信息。 @endif */
    float *probMeas;                    /*!< @if Eng confidence value information of all anchors.
                                             @else   所有锚点置信度值信息。 @endif */
    uint8_t dMeasNum;                   /*!< @if Eng length of dMeas.
                                             @else   测距向量长度。 @endif */
    uint8_t probMeasNum;                /*!< @if Eng length of probMeas.
                                             @else   置信度向量长度。 @endif */
    uint8_t *smooth_num;
} slem_pos_dis_prob;

errcode_slem slem_alg_calc_pos(float (*anchorPara)[3], slem_pos_dis_prob *p_dis_prob, slem_pos_cfg *cfg, uint8_t key_id,
                               slem_tag_pos *result);
#endif