/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#ifndef OHOS_DAUDIO_CONSTANTS_H
#define OHOS_DAUDIO_CONSTANTS_H

#include <cstdint>
#include <string>
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
/* Audio package name */
const std::string PKG_NAME = "ohos.dhardware.daudio";
/* Audio data session name */
const std::string DATA_SPEAKER_SESSION_NAME = "ohos.dhardware.daudio.speakerdata";
const std::string DATA_MIC_SESSION_NAME = "ohos.dhardware.daudio.micdata";
/* Audio ctrl session name */
const std::string CTRL_SESSION_NAME = "ohos.dhardware.daudio.ctrl";

constexpr int32_t DEFAULT_AUDIO_DATA_SIZE = 4096;

constexpr int32_t DELETE_POINT_POS = 4;
constexpr int32_t DELETE_CPP_LEN = 4;
constexpr int32_t CHANNEL_WAIT_SECONDS = 5;
constexpr int32_t LOG_MAX_LEN = 4096;
constexpr int32_t DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID = 4805;
constexpr int32_t DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID = 4806;
constexpr int32_t AUDIO_LOADSA_TIMEOUT_MS = 10000;

constexpr int32_t AUDIO_DEVICE_TYPE_UNKNOWN = 0;
constexpr int32_t AUDIO_DEVICE_TYPE_SPEAKER = 1;
constexpr int32_t AUDIO_DEVICE_TYPE_MIC = 2;

constexpr int32_t PIN_OUT_SPEAKER = 1;
constexpr int32_t PIN_OUT_DAUDIO_DEFAULT = 1 << 7;
constexpr int32_t PIN_IN_DAUDIO_DEFAULT = 1 << 27 | 1 << 5;
constexpr int32_t PIN_IN_MIC = 1 << 27 | 1 << 0;

constexpr int32_t NONE_ITEM = 0;
constexpr int32_t SINGLE_ITEM = 1;

constexpr uint32_t SAMPLE_RATE_DEFAULT = 48000;
constexpr uint32_t CHANNEL_COUNT_DEFAULT = 2;
constexpr uint32_t SAMPLE_FORMAT_DEFAULT = 1;

constexpr uint32_t DAUDIO_MAX_SESSION_NAME_LEN = 50;
constexpr uint32_t DAUDIO_MAX_DEVICE_ID_LEN = 100;
constexpr uint32_t DAUDIO_MAX_TASKQUEUE_LEN = 100;
constexpr uint32_t DAUDIO_MAX_RECV_DATA_LEN = 104857600;
constexpr uint32_t DAUDIO_MAX_JSON_LEN = 1024;

const std::string DAUDIO_LOG_TITLE_TAG = "DAUDIO";
const std::string DAUDIO_PREFIX = "DISTRIBUTED_AUDIO";
const std::string AUDIO_PREFIX = "AUDIO";
const std::string SEPERATOR = "#";
const std::string SUPPORTED_SAMPLE_RATE = "supportedSampleRate";
const std::string SUPPORTED_FORMATS = "supportedFormats";
const std::string SUPPORTED_CHANNEL_MAX = "supportedChannelMax";
const std::string SUPPORTED_CHANNEL_MIN = "supportedChannelMin";
const std::string SUPPORTED_BITRATE_MAX = "supportedBirteMax";
const std::string SUPPORTED_BITRATE_MIN = "supportedBirteMin";
const std::string MINE_TYPE = "mineType";
const std::string AVENC_AAC = "avenc_aac";
const std::string NAME = "name";
const std::string KEY_CODECTYPE = "codecType";
const std::string KEY_DEVICE_TYPE = "deviceType";

const std::string HDF_EVENT_RESULT_SUCCESS = "DH_SUCCESS";
const std::string HDF_EVENT_RESULT_FAILED = "DH_FAILED";

const std::string STREAM_MUTE_STATUS = "STREAM_MUTE_STATUS";
const std::string AUDIO_VOLUME_TYPE = "AUDIO_VOLUME_TYPE";
const std::string VOLUME_LEVEL = "VOLUME_LEVEL";

constexpr const char *KEY_TYPE = "type";
constexpr const char *KEY_CONTENT = "content";
constexpr const char *KEY_DH_ID = "dhId";
constexpr const char *KEY_DEV_ID = "devId";
constexpr const char *KEY_RESULT = "result";
constexpr const char *KEY_EVENT_TYPE = "eventType";
constexpr const char *KEY_AUDIO_PARAM = "audioParam";
constexpr const char *KEY_ATTRS = "attrs";

constexpr const char *KEY_SAMPLING_RATE = "samplingRate";
constexpr const char *KEY_CHANNELS = "channels";
constexpr const char *KEY_FORMAT = "format";
constexpr const char *KEY_SOURCE_TYPE = "sourceType";
constexpr const char *KEY_CONTENT_TYPE = "contentType";
constexpr const char *KEY_STREAM_USAGE = "streamUsage";
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_CONSTANTS_H
