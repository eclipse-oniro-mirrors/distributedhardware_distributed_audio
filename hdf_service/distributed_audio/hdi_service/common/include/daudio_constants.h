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

#include <string>

namespace OHOS {
namespace DistributedHardware {
// Distributed Auido Parameters
const std::string KEY_AUDIOPORT_DIR = "portdir";
const std::string KEY_AUDIOFORMAT = "format";
const std::string KEY_AUDIOCHANNELMASK = "channel";
const std::string KEY_AUDIOSAMPLERATE = "samplerate";

const std::string VALUE_AUDIOPORT_DIR_IN = "portdirin";
const std::string VALUE_AUDIOPORT_DIR_OUT = "portdirout";
const std::string VALUE_AUDIOPORT_DIR_INOUT = "portdirinout";

const std::string DEVICE_TYPE_OUTPUT_DEFAULT = "0";
const std::string DEVICE_TYPE_INPUT_DEFAULT = "1";

const std::string VOLUME_GROUP_ID = "VOLUME_GROUP_ID";
const std::string INTERRUPT_GROUP_ID = "INTERRUPT_GROUP_ID";

// Distributed Auido Parameters
const std::string VOLUME_LEVEL = "VOLUME_LEVEL";
const std::string VOLUME_EVENT_TYPE = "EVENT_TYPE";
const std::string MAX_VOLUME_LEVEL = "MAX_VOLUME_LEVEL";
const std::string MIN_VOLUME_LEVEL = "MIN_VOLUME_LEVEL";
const std::string STREAM_MUTE_STATUS = "STREAM_MUTE_STATUS";

const std::string HDF_EVENT_RESULT_SUCCESS = "DH_SUCCESS";
const std::string HDF_EVENT_RESULT_FAILED = "DH_FAILED";

constexpr int32_t AUDIO_DEVICE_TYPE_UNKNOWN = 0;
constexpr int32_t AUDIO_DEVICE_TYPE_SPEAKER = 1;
constexpr int32_t AUDIO_DEVICE_TYPE_MIC = 2;

constexpr uint32_t VOLUME_GROUP_ID_DEFAULT = 0;
constexpr uint32_t INTERRUPT_GROUP_ID_DEFAULT = 0;

constexpr uint32_t AUDIO_SAMPLE_RATE_DEFAULT = 4800;
constexpr uint32_t AUDIO_CHANNEL_COUNT_DEFAULT = 2;
constexpr uint32_t AUDIO_FORMAT_DEFAULT = 16;
constexpr uint32_t AUDIO_DATA_SIZE_DEFAULT = 4096;

constexpr int32_t MILLISECOND_PER_SECOND = 1000;
constexpr int32_t AUDIO_FRAME_TIME_INTERFAL_DEFAULT = 20000;

constexpr uint32_t AUDIO_DEFAULT_MAX_VOLUME_LEVEL = 15;
constexpr uint32_t AUDIO_DEFAULT_MIN_VOLUME_LEVEL = 0;
} // DistributeHardware
} // OHOS
#endif // OHOS_DAUDIO_CONSTANTS_H
