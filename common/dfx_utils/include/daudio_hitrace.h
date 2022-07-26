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

#ifndef OHOS_DAUDIO_HITRACE_H
#define OHOS_DAUDIO_HITRACE_H

#include <string>

#include "hitrace_meter.h"

#define DAUDIO_SYNC_TRACE(value) HITRACE_METER_NAME(DAUDIO_HITRACE_LABEL, value)

namespace OHOS {
namespace DistributedHardware {
constexpr uint64_t DAUDIO_HITRACE_LABEL = HITRACE_TAG_DISTRIBUTED_AUDIO;

const std::string DAUDIO_LOAD_HDF_DRIVER = "DAUDIO_LOAD_HDF_DRIVER";
const std::string DAUDIO_SOURCE_LOAD_SYSTEM_ABILITY = "DAUDIO_SOURCE_LOAD_SYSTEM_ABILITY";
const std::string DAUDIO_SINK_LOAD_SYSTEM_ABILITY = "DAUDIO_SINK_LOAD_SYSTEM_ABILITY";
const std::string DAUDIO_REGISTER_AUDIO = "DAUDIO_REGISTER_AUDIO";
const std::string DAUDIO_UNREGISTER_AUDIO = "DAUDIO_UNREGISTER_AUDIO";
const std::string DAUDIO_ENABLE_SPK = "DAUDIO_ENABLE_SPK";
const std::string DAUDIO_DISABLE_SPK = "DAUDIO_DISABLE_SPK";
const std::string DAUDIO_ENABLE_MIC = "DAUDIO_ENABLE_MIC";
const std::string DAUDIO_DISABLE_MIC = "DAUDIO_DISABLE_MIC";

const std::string DAUDIO_CREATE_DATA_SESSION = "DAUDIO_CREATE_DATA_SESSION";
const std::string DAUDIO_RELEASE_DATA_SESSION = "DAUDIO_RELEASE_DATA_SESSION";
const std::string DAUDIO_OPEN_DATA_SESSION = "DAUDIO_OPEN_DATA_SESSION";
const std::string DAUDIO_CLOSE_DATA_SESSION = "DAUDIO_CLOSE_DATA_SESSION";

const std::string DAUDIO_CREATE_CTRL_SESSION = "DAUDIO_CREATE_CTRL_SESSION";
const std::string DAUDIO_RELEASE_CTRL_SESSION = "DAUDIO_RELEASE_CTRL_SESSION";
const std::string DAUDIO_OPEN_CTRL_SESSION = "DAUDIO_OPEN_CTRL_SESSION";
const std::string DAUDIO_CLOSE_CTRL_SESSION = "DAUDIO_CLOSE_CTRL_SESSION";

const std::string DAUDIO_START_ENCODER_PROCESSOR = "DAUDIO_START_ENCODER_PROCESSOR";
const std::string DAUDIO_STOP_ENCODER_PROCESSOR = "DAUDIO_STOP_ENCODER_PROCESSOR";
const std::string DAUDIO_RELEASE_ENCODER_PROCESSOR = "DAUDIO_RELEASE_ENCODER_PROCESSOR";
const std::string DAUDIO_START_DECODER_PROCESSOR = "DAUDIO_START_DECODER_PROCESSOR";
const std::string DAUDIO_STOP_DECODER_PROCESSOR = "DAUDIO_STOP_DECODER_PROCESSOR";
const std::string DAUDIO_RELEASE_DECODER_PROCESSOR = "DAUDIO_RELEASE_DECODER_PROCESSOR";

enum DaudioTaskId : int32_t {
    DAUDIO_REGISTER_AUDIO_TASKID = 0,
    DAUDIO_UNREGISTER_AUDIO_TASKID = 1,
    DAUDIO_OPEN_DATA_SESSION_TASKID = 2,
    DAUDIO_OPEN_CTRL_SESSION_TASKID = 3,
};

void DaudioStartAsyncTrace(const std::string& str, int32_t taskId);
void DaudioFinishAsyncTrace(const std::string& str, int32_t taskId);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_HITRACE_H
