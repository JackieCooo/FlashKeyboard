/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 *
 * Description: SLE Hadm Manager module.
 */

/**
 * @defgroup sle_hadm_manager hadm manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_HADM_MANAGER_H
#define SLE_HADM_MANAGER_H

#include <stdint.h>
#include "errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SLE_CS_CAPS_LEN 0x20        /*!< @if Eng size of channel sounding capability set
                                        @else channel sounding 能力集大小 @endif */
#define SLE_CS_IQ_REPORT_COUNT 80   /*!< @if Eng Channel sounding IQ report
                                       @else channel sounding iq 上报 @endif */
/**
 * @if Eng
 * @brief Enum of sle channel sounding enable state.
 * @else
 * @brief channel sounding 使能状态
 * @endif
 */
typedef enum {
    SLE_CHANNEL_SOUNDING_DISABLE,
    SLE_CHANNEL_SOUNDING_ENABLE,
    SLE_CHANNEL_SOUNDING_MAX,
} sle_channel_sounding_state_t;

/**
 * @if Eng
 * @brief Enum of set sle channel sounding param.
 * @else
 * @brief 设置channel sounding 参数
 * @endif
 */
typedef struct {
    uint8_t  is_cs_param_chg; /*!< @if Eng Whether ranging connection parameters need to be modified.
                                   @else 是否需要更改测距连接参数 @endif */
    uint8_t  freq_space; /*!< @if Eng Frequency spacing.
                              @else 频率间隔 @endif */
    uint8_t  con_anchor_num; /*!< @if Eng Number of anchors to be connected.
                                  @else 需要连接的锚点数量 @endif */
    uint8_t  refresh_rate; /*!< @if Eng Refresh rate.
                                @else 刷新率 @endif */
    uint16_t acb_interval; /*!< @if Eng ACB link period calculated based on parameters.
                                @else 根据参数计算得到的acb链路周期 @endif */
    uint16_t cs_interval;  /*!< @if Eng Ranging period calculated based on parameters.
                                @else 根据参数计算得到的测距周期 @endif */
    uint16_t posalg_freq;  /*!< @if Eng Posalg Frequency.
                                @else 算法频率 @endif */
    uint8_t  glp_mode;     /*!< @if Eng glp mode.
                                @else 2.4GHz 模式 @endif */
} sle_set_channel_sounding_param_ex_t;

/**
 * @if Eng
 * @brief Enum of set GTTT mode sle channel sounding param.
 * @else
 * @brief 设置GTTT 模式下 channel sounding 参数
 * @endif
 */
typedef struct {
    uint8_t  is_cs_param_chg; /*!< @if Eng Whether ranging connection parameters need to be modified.
                                @else 是否需要更改测距连接参数 @endif */
    uint8_t  freq_space; /*!< @if Eng Frequency spacing.
                                @else 频率间隔 @endif */
    uint8_t  con_anchor_num; /*!< @if Eng Number of anchors to be connected.
                                @else 需要连接的锚点数量 @endif */
    uint8_t  refresh_rate; /*!< @if Eng Refresh rate.
                                @else 刷新率 @endif */
    uint16_t acb_interval; /*!< @if Eng ACB link period calculated based on parameters.
                                @else 根据参数计算得到的acb链路周期 @endif */
    uint16_t cs_interval;  /*!< @if Eng Ranging period calculated based on parameters.
                                @else 根据参数计算得到的测距周期 @endif */
    uint16_t posalg_freq;  /*!< @if Eng Posalg Frequency.
                                @else 算法频率 @endif */
    uint8_t  glp_mode;     /*!< @if Eng glp mode.
                                @else 2.4GHz 模式 @endif */
    uint8_t g_handle;   /*!< @if Eng GTTT serial number.
                             @else 标识GTTT在网序号 @endif */
    uint8_t is_gtt;     /*!< @if Eng Indicates whether the network is a GTT network. 0: no; 1: yes.
                             @else 是否是GTTT组网, 0：不是GTT组网业务，1：是GTT组网  @endif */
    uint8_t is_gtt_tt_slem; /*!< @if Eng Whether the slem service is TT type. 0: no; 1: yes.
                                 @else 是否是GTT组网下的TT测距业务, 0: 不是TT测距, 即G-T测距,  1: 是TT测距 @endif */
    uint8_t set_local_or_remote_param; /*!< @if Eng set local or remote parameters. 0:set local and remote; 1:set remote
                                            @else 设置本端或对端参数，0：给本端及对端设置，1：给对端设置 @endif */
    uint8_t role; /*!< @if Eng Remote device sending indication. Serving the G-T slem
                       @else 对端设备先发后发指示，主要针对GTT组网时，G-T测距的场景 @endif */
    uint16_t tt_tx_conhdl; /*!< @if Eng Conn ID of the Tx party in a TT.
                                @else TT中tx方的conn id @endif */
    uint16_t tt_rx_conhdl; /*!< @if Eng Conn ID of the Rx party in a TT.
                                @else TT中rx方的conn id @endif */
    uint16_t tx_tt_id;     /*!< @if Eng ID of the TX party in the GTTT network.
                                @else TT中tx方在GTTT组网中的标志号 @endif */
    uint16_t rx_tt_id;     /*!< @if Eng ID of the RX party in the GTTT network.
                                @else TT中rx方在GTTT组网中的标志号 @endif */
} sle_set_channel_sounding_param_ex_gttt_t;

/**
 * @if Eng
 * @brief Enum of set GTTT mode slem enable.
 * @else
 * @brief 设置GTTT 模式下的 slem 使能
 * @endif
 */
typedef struct {
    uint8_t cs_en;
    uint8_t g_handle;   /*!< @if Eng GTTT serial number.
                             @else 标识GTTT在网序号 @endif */
    uint8_t is_gtt;     /*!< @if Eng Indicates whether the network is a GTTT network. 0: no; 1: yes.
                             @else 是否是GTTT组网, 0：不是GTT组网业务，1：是GTT组网  @endif */
    uint8_t is_gtt_tt_slem; /*!< @if Eng Whether the slem service is TT type. 0: no; 1: yes.
                                 @else 是否是GTTT组网下的TT测距业务, 0: 不是TT测距, 即G-T测距,  1: 是TT测距 @endif */
    uint8_t role; /*!< @if Eng Remote device sending indication. Serving the G-T slem
                       @else 对端设备先发后发指示，主要针对GTTT组网时，G-T测距的场景 @endif */
    uint8_t enlarge_ratio; /*!< @if Eng Enlarge ratio of period in GTTT network
                                @else GTTT组网中大周期倍数,与同时接入的T_Tx数量有关，影响大周期调度 @endif */
    uint8_t sched_order; /*!< @if Eng schedule order
                              @else T-Tx与T-Rx的测距调度组,在大周期内的相对位置组序号 @endif */
    uint8_t sched_times; /*!< @if Eng slem schedule times
                              @else 在一组T-Tx与T-Rx的测距调度中，slem总共执行的次数,与T_Rx的数量有关 @endif */
    uint16_t tt_tx_conhdl; /*!< @if Eng Conn ID of the Tx party in a TT.
                                @else TT中tx方的conn id @endif */
    uint16_t tt_rx_conhdl; /*!< @if Eng Conn ID of the Rx party in a TT.
                                @else TT中rx方的conn id @endif */
    uint16_t tx_tt_id;     /*!< @if Eng ID of the TX party in the GTTT network.
                                @else TT中tx方在GTTT组网中的标志号 @endif */
    uint16_t rx_tt_id;     /*!< @if Eng ID of the RX party in the GTTT network.
                                @else TT中rx方在GTTT组网中的标志号 @endif */
} sle_set_slem_enable_gttt_t;

/**
 * @if Eng
 * @brief Enum of sle channel sounding iq report
 * @else
 * @brief channel sounding IQ 上报
 * @endif
 */
typedef struct {
    uint8_t samp_cnt;
    uint8_t report_idx;
    uint16_t es_sn;
    uint32_t timestamp_sn; // 对齐master的时间
    uint8_t rssi[SLE_CS_IQ_REPORT_COUNT];
    uint8_t freq[SLE_CS_IQ_REPORT_COUNT];
    uint16_t i_data[SLE_CS_IQ_REPORT_COUNT];
    uint16_t q_data[SLE_CS_IQ_REPORT_COUNT];
    uint32_t tof_result;
    uint8_t is_gtt_tt_slem; /*!< @if Eng Whether the slem service is TT type. 0: no; 1: yes.
                                 @else 是否是GTTT组网下的TT测距业务, 0: 不是TT测距, 即G-T测距,  1: 是TT测距 @endif */
    uint16_t tx_tt_id;      /*!< @if Eng ID of the TX party in the GTTT network.
                                 @else TT中tx方在GTTT组网中的标志号 @endif */
    uint16_t rx_tt_id;      /*!< @if Eng ID of the RX party in the GTTT network.
                                 @else TT中rx方在GTTT组网中的标志号 @endif */
} sle_channel_sounding_iq_report_t;

/**
 * @if Eng
 * @brief sle channel sounding capability set.
 * @else
 * @brief channel sounding 能力集
 * @endif
 */
typedef struct {
    uint8_t caps[SLE_CS_CAPS_LEN]; /*!< @if Eng sle channel sounding capability set.
                                        @else   channel sounding 能力集 @endif */
} sle_channel_sounding_caps_t;

/**
 * @if Eng
 * @brief Callback invoked when read local channel sounding.
 * @par Callback invoked when read local channel sounding.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  caps    channel sounding capability set.
 * @param  [in]  status  error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  读取本端channel sounding的回调函数。
 * @par    读取本端channel sounding的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  caps    channel sounding能力集。
 * @param  [in]  status  执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_read_local_channel_sounding_callback)(sle_channel_sounding_caps_t *caps,
    errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when read remote channel sounding.
 * @par Callback invoked when read remote channel sounding.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  conn_id discovery ID.
 * @param  [in]  caps    channel sounding capability set.
 * @param  [in]  status  error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  读取对端channel sounding的回调函数。
 * @par    读取对端channel sounding的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  conn_id 公开ID。
 * @param  [in]  caps    channel sounding能力集。
 * @param  [in]  status  执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_read_remote_channel_sounding_callback)(uint16_t conn_id, sle_channel_sounding_caps_t *caps,
    errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when channel sounding state changed.
 * @par Callback invoked when channel sounding state changed.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  channel sounding状态变化的回调函数。
 * @par    channel sounding状态变化的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_channel_sounding_state_changed_callback)(uint8_t slem_status, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when set channel sounding parameter.
 * @par Callback invoked when channel sounding parameter.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  conn_id discovery ID.
 * @param  [in]  report.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  channel sounding参数设置的回调函数。
 * @par    channel sounding参数设置的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  conn_id 公开ID。
 * @param  [in]  report
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_channel_sounding_iq_report_callback)(uint16_t conn_id, sle_channel_sounding_iq_report_t *report);

/**
 * @if Eng
 * @brief Callback invoked when set channel sounding config.
 * @par Callback invoked when set channel sounding config.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  set channel sounding config的回调函数。
 * @par    set channel sounding config的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_channel_sounding_set_config_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Struct of SLE hadm manager callback function.
 * @else
 * @brief SLE hadm 管理回调函数接口定义。
 * @endif
 */
typedef struct {
    sle_read_local_channel_sounding_callback read_local_cs_caps_cb;   /*!< @if Eng read local channel sounding
                                                                                   callback.
                                                                           @else   读取本端channel sounding回调函数。
                                                                           @endif */
    sle_read_remote_channel_sounding_callback read_remote_cs_caps_cb; /*!< @if Eng read remote channel sounding
                                                                                   callback.
                                                                           @else   读取对端channel sounding回调函数。
                                                                           @endif */
    sle_channel_sounding_state_changed_callback cs_state_changed_cb;  /*!< @if Eng channel sounding state changed
                                                                                   callback.
                                                                           @else   channel sounding状态改变回调函数。
                                                                           @endif */
    sle_channel_sounding_iq_report_callback cs_iq_report_cb;          /*!< @if Eng channel sounding iq report
                                                                                   callback.
                                                                           @else   channel sounding iq上报回调函数。
                                                                           @endif */
    sle_channel_sounding_set_config_callback cs_set_config_cb;        /*!< @if Eng channel sounding iq report
                                                                                   callback.
                                                                           @else   channel sounding iq上报回调函数。
                                                                           @endif */
} sle_hadm_callbacks_t;

/**
 * @if Eng
 * @brief  Read local channel sounding capability.
 * @par Description: Read local channel sounding capability.
 * @retval error code, read result will be returned at { @ref sle_read_local_channel_sounding_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  读取本端channel sounding能力。
 * @par Description: 读取本端channel sounding能力。
 * @retval 执行结果错误码，读取结果将在{ @ref sle_read_local_channel_sounding_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_read_local_channel_sounding_caps(void);

/**
 * @if Eng
 * @brief  Read remote channel sounding capability.
 * @par Description: Read remote channel sounding capability.
 * @param  [in]  conn_id connection ID.
 * @retval error code, read result will be returned at { @ref sle_read_remote_channel_sounding_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  读取对端channel sounding能力。
 * @par Description: 读取对端channel sounding能力。
 * @param  [in]  conn_id 连接 ID
 * @retval 执行结果错误码，读取结果将在{ @ref sle_read_remote_channel_sounding_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_read_remote_channel_sounding_caps(uint16_t conn_id);

/**
 * @if Eng
 * @brief  Set channel sounding parameter.
 * @par Description: Set channel sounding parameter.
 * @param  [in]  conn_id connection ID.
 * @param  [in]  param   parameter.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置channel sounding参数。
 * @par Description: 设置channel sounding参数。
 * @param  [in]  conn_id 连接 ID。
 * @param  [in]  param   参数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败，参考 @ref errcode_t 。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_channel_sounding_param_ex(uint16_t conn_id, sle_set_channel_sounding_param_ex_t *param);

/**
 * @if Eng
 * @brief  Set GTTT mode channel sounding parameter.
 * @par Description: Set GTTT mode channel sounding parameter.
 * @param  [in]  conn_id connection ID.
 * @param  [in]  param   parameter.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置GTTT模式下channel sounding参数。
 * @par Description: 设置GTTT模式下channel sounding参数。
 * @param  [in]  conn_id 连接 ID。
 * @param  [in]  param   参数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败，参考 @ref errcode_t 。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_channel_sounding_param_ex_gttt(uint16_t conn_id, sle_set_channel_sounding_param_ex_gttt_t *param);

/**
 * @if Eng
 * @brief  Set channel sounding enable.
 * @par Description: Set channel sounding enable.
 * @retval error code, read result will be returned at { @ref sle_channel_sounding_state_changed_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置channel sounding使能。
 * @par Description: 设置channel sounding使能。
 * @retval 执行结果错误码，读取结果将在{ @ref sle_channel_sounding_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_channel_sounding_enable(uint16_t conn_id);

/**
 * @if Eng
 * @brief  Set GTTT mode channel sounding enable.
 * @par Description: Set GTTT mode channel sounding enable.
 * @retval error code, read result will be returned at { @ref sle_channel_sounding_state_changed_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置GTTT 模式下 channel sounding使能。
 * @par Description: 设置GTTT 模式下 channel sounding使能。
 * @retval 执行结果错误码，读取结果将在{ @ref sle_channel_sounding_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_channel_sounding_enable_gttt(uint16_t conn_id, sle_set_slem_enable_gttt_t *param);

/**
 * @if Eng
 * @brief  Set channel sounding disable.
 * @par Description: Set channel sounding disable.
 * @retval error code, read result will be returned at { @ref sle_channel_sounding_state_changed_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置channel sounding关闭。
 * @par Description: 设置channel sounding关闭。
 * @retval 执行结果错误码，读取结果将在{ @ref sle_channel_sounding_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_channel_sounding_disable(uint16_t conn_id);

/**
 * @if Eng
 * @brief  Set GTTT mode channel sounding disable.
 * @par Description: Set GTTT mode channel sounding disable.
 * @retval error code, read result will be returned at { @ref sle_channel_sounding_state_changed_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置GTTT 模式下 channel sounding关闭。
 * @par Description: 设置channel sounding关闭。
 * @retval 执行结果错误码，读取结果将在{ @ref sle_channel_sounding_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_channel_sounding_disable_gttt(uint16_t conn_id, sle_set_slem_enable_gttt_t *param);

/**
 * @if Eng
 * @brief  Register SLE hadm manager callbacks.
 * @par Description: Register SLE hadm manager callbacks.
 * @param  [in]  func Callback function.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @else
 * @brief  注册 SLE hadm 管理回调函数。
 * @par Description: 注册 SLE hadm 管理回调函数。
 * @param  [in]  func 回调函数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @endif
 */
errcode_t sle_hadm_register_callbacks(sle_hadm_callbacks_t *func);


#ifdef __cplusplus
}
#endif
#endif /* SLE_HADM_MANAGER */
/**
 * @}
 */
