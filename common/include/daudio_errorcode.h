/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DAUDIO_ERRCODE_H
#define OHOS_DAUDIO_ERRCODE_H

namespace OHOS {
namespace DistributedHardware {
enum DAudioErrorCode {
    DH_SUCCESS = 0,
    ERR_DH_AUDIO_NULLPTR = -40000,
    ERR_DH_AUDIO_FAILED = -40001,
    ERR_DH_AUDIO_NOT_SUPPORT = -40002,
    ERR_DH_AUDIO_REPEAT_OPREATOR = -40003,

    ERR_DH_AUDIO_SA_WRITE_INTERFACE_TOKEN_FAILED = -40004,
    ERR_DH_AUDIO_SA_WRITE_PARAM_FAIED = -40005,
    ERR_DH_AUDIO_SA_IPCCALLBACK_NOT_INIT = -40006,
    ERR_DH_AUDIO_SA_REGISTERCALLBACK_NOT_FOUND = -40007,
    ERR_DH_AUDIO_SA_UNREGISTERCALLBACK_NOT_FOUND = -40008,
    ERR_DH_AUDIO_SA_INVALID_INTERFACE_TOKEN = -40009,
    ERR_DH_AUDIO_SA_INVALID_REQUEST_CODE = -40010,
    ERR_DH_AUDIO_SA_READ_PARAM_FAILED = -40011,
    ERR_DH_AUDIO_SA_CTRL_TRANS_NULL = -40012,
    ERR_DH_AUDIO_SA_CTRL_CHANNEL_WAIT_TIMEOUT = -40013,
    ERR_DH_AUDIO_SA_MIC_CHANNEL_WAIT_TIMEOUT = -40014,
    ERR_DH_AUDIO_SA_SINK_CTRL_TRANS_NULL = -40015,
    ERR_DH_AUDIO_SA_MICCALLBACK_NULL = -40016,
    ERR_DH_AUDIO_SA_MICTRANS_NULL = -40017,
    ERR_DH_AUDIO_SA_EVENT_CALLBACK_NULL = -40018,
    ERR_DH_AUDIO_SA_MIC_TRANS_NULL = -40019,
    ERR_DH_AUDIO_SA_FUNCTION_NOT_IMPLEMENT = -40020,
    ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT = -40021,
    ERR_DH_AUDIO_SA_RPC_WAIT_TIMEOUT = -40022,
    ERR_DH_AUDIO_SA_SPEAKER_CLIENT_NOT_INIT = -40023,
    ERR_DH_AUDIO_SA_SPEAKER_TRANS_NULL = -40024,
    ERR_DH_AUDIO_SA_SPEAKER_CHANNEL_WAIT_TIMEOUT = -40025,
    ERR_DH_AUDIO_SA_PARAM_INVALID = -40026,
    ERR_DH_AUDIO_SA_GET_REMOTE_SINK_FAILED = -40027,
    ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST = -40028,
    ERR_DH_AUDIO_SA_INVALID_NETWORKID = -40029,
    ERR_DH_AUDIO_SA_OPEN_CTRL_FAILED = -40030,
    ERR_DH_AUDIO_SA_ENABLE_PARAM_INVALID = -40031,
    ERR_DH_AUDIO_SA_DEVICE_TYPE_INVALID = -40032,
    ERR_DH_AUDIO_SA_DISABLE_PARAM_INVALID = -40033,
    ERR_DH_AUDIO_SA_DISABLE_SPEAKER_NOT_EXIST = -40034,
    ERR_DH_AUDIO_SA_DISABLE_MIC_NOT_EXIST = -40035,
    ERR_DH_AUDIO_SA_SPEAKER_DEVICE_NOT_INIT = -40036,
    ERR_DH_AUDIO_SA_MIC_DEVICE_NOT_INIT = -40037,
    ERR_DH_AUDIO_SA_SOURCECTRLMGR_NOT_INIT = -40038,
    ERR_DH_AUDIO_SA_TASKQUEUE_FULL = -40039,
    ERR_DH_AUDIO_SA_GET_REMOTE_SOURCE_FAILED = -40040,
    ERR_DH_AUDIO_SA_PROXY_NOT_INIT = -40041,
    ERR_DH_AUDIO_SA_MIC_CLIENT_NOT_INIT = -40042,
    ERR_DH_AUDIO_SA_GET_SAMGR_FAILED = -40043,
    ERR_DH_AUDIO_SA_LOAD_FAILED = -40044,
    ERR_DH_AUDIO_SA_LOAD_TIMEOUT = -40045,

    ERR_DH_AUDIO_TRANS_NULL_VALUE = -41000,
    ERR_DH_AUDIO_TRANS_PROCESSOR_FAILED = -41001,
    ERR_DH_AUDIO_TRANS_ERROR = -41002,
    ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION = -41003,
    ERR_DH_AUDIO_TRANS_SESSION_NOT_OPEN = -41004,

    ERR_DH_AUDIO_ADAPTER_PARA_ERROR = -42000,
    ERR_DH_AUDIO_ADAPTER_OPEN_SESSION_FAIL = -42001,
    ERR_DH_AUDIO_ADAPTER_REGISTER_SOFTBUS_LISTENER_FAIL = -42002,

    ERR_DH_AUDIO_BAD_VALUE = -43000,
    ERR_DH_AUDIO_BAD_OPERATE = -43001,
    ERR_DH_AUDIO_CODEC_CONFIG = -43002,
    ERR_DH_AUDIO_CODEC_START = -43003,
    ERR_DH_AUDIO_CODEC_STOP = -43004,
    ERR_DH_AUDIO_CODEC_RELEASE = -43005,
    ERR_DH_AUDIO_CODEC_INPUT = -43006,

    ERR_DH_AUDIO_CLIENT_PARAM_IS_NULL = -44000,
    ERR_DH_AUDIO_CLIENT_CREATE_RENDER_FAILED = -44001,
    ERR_DH_AUDIO_CLIENT_RENDERER_WITHOUT_INSTANCE = -44002,
    ERR_DH_AUDIO_CLIENT_RENDER_FREE_FAILED = -44003,
    ERR_DH_AUDIO_CLIENT_RENDER_STARTUP_FAILURE = -44004,
    ERR_DH_AUDIO_CLIENT_RENDER_OR_TRANS_IS_NULL = -44005,
    ERR_DH_AUDIO_CLIENT_LACE_OF_GFRAMES = -44006,
    ERR_DH_AUDIO_CLIENT_REBDER_PAUSE_FAILURE = -44007,
    ERR_DH_AUDIO_CLIENT_STATE_IS_INVALID = -44008,
    ERR_DH_AUDIO_CLIENT_INVALID_VOLUME_PARAMETER = -44009,
    ERR_DH_AUDIO_CLIENT_SET_VOLUME_FAILED = -44010,
    ERR_DH_AUDIO_CLIENT_SET_RENDER_RATE_FAILED = -44011,
    ERR_DH_AUDIO_CLIENT_SET_CALLBACK_FAILED = -44012,
    ERR_DH_AUDIO_CLIENT_GET_MINBUFFER_FAILED = -44013,
    ERR_DH_AUDIO_CLIENT_CAPTURER_WITHOUT_INSTANCE = -44014,
    ERR_DH_AUDIO_CLIENT_CAPTURER_FREE_FAILED = -44015,
    ERR_DH_AUDIO_CLIENT_CAPTURER_START_FAILED = -44016,
    ERR_DH_AUDIO_CLIENT_CAPTURER_STOP_FAILED = -44017,
    ERR_DH_AUDIO_CLIENT_CAPTURER_RELEASE_FAILED = -44018,
    ERR_DH_AUDIO_CLIENT_CAPTURER_OR_MICTRANS_INSTANCE = -44019,
    ERR_DH_AUDIO_CLIENT_CREATE_CAPTURER_FAILED = -44020,
    ERR_DH_AUDIO_CLIENT_RENDER_STOP_FAILED = -44021,
    ERR_DH_AUDIO_CLIENT_TRANS_TIMEOUT = -44022,
    ERR_DH_AUDIO_CLIENT__VOLUME_SET_FAILED = -44024,
    ERR_DH_AUDIO_CLIENT_GET_MAX_VOLUME_FAILED = -44025,
    ERR_DH_AUDIO_CLIENT_GET_MIX_VOLUME_FAILED = -44026,
    ERR_DH_AUDIO_CLIENT_GET_VOLUME_FAILED = -44027,

    ERR_DH_AUDIO_HDI_PROXY_NOT_INIT = -45000,
    ERR_DH_AUDIO_HDI_CALLBACK_NOT_EXIST = -45001,
    ERR_DH_AUDIO_HDI_CALL_FAILED = -45002,
    ERR_DH_AUDIO_HDI_UNKOWN_DEVTYPE = -45003,
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_ERRCODE_H
