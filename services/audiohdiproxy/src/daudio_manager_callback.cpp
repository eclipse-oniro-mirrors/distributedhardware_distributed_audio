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

#include "daudio_manager_callback.h"

#include <cstdint>
#include <hdf_base.h>
#include <securec.h>

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"

using OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioParameter;

namespace OHOS {
namespace DistributedHardware {
int32_t DAudioManagerCallback::OpenDevice(const std::string& adpName, int32_t devId)
{
    DHLOGI("%s: OpenDevice enter", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }
    int32_t ret = callback_->OpenDevice(adpName, devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: call hdi callback failed", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::CloseDevice(const std::string& adpName, int32_t devId)
{
    DHLOGI("%s: CloseDevice enter", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }
    int32_t ret = callback_->CloseDevice(adpName, devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: call hdi callback failed", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::SetParameters(const std::string& adpName, int32_t devId, const AudioParameter& param)
{
    DHLOGI("%s: SetParameters enter", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }

    AudioParamHDF newParam = {
        .sampleRate = AudioSampleRate(param.sampleRate),
        .channelMask = AudioChannel(param.channelCount),
        .bitFormat = AudioSampleFormat(param.format),
        .streamUsage = StreamUsage(param.streamUsage),
        .frameSize = param.frameSize,
        .period = param.period,
        .ext = param.ext,
    };
    int32_t ret = callback_->SetParameters(adpName, devId, newParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: call hdi callback failed", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::NotifyEvent(const std::string& adpName, int32_t devId,
    const OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioEvent& event)
{
    DHLOGI("%s: NotifyEvent", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }
    AudioEvent newEvent = {
        .type = AudioEventType(std::stoi(event.type)),
        .content = event.content
    };
    int32_t ret = callback_->NotifyEvent(adpName, devId, newEvent);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: call hdi callback failed", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::WriteStreamData(const std::string &adpName, int32_t devId,
    const OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioData &data)
{
    DHLOGI("%s: Write Stream Data.", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: Register hdi callback is nullptr.", LOG_TAG);
        return HDF_FAILURE;
    }
    if (data.data.size() != DEFAULT_AUDIO_DATA_SIZE) {
        DHLOGE("%s: Audio data size is not support.", LOG_TAG);
        return HDF_FAILURE;
    }

    std::shared_ptr<AudioData> audioData = std::make_shared<AudioData>(DEFAULT_AUDIO_DATA_SIZE);
    int32_t ret = memcpy_s(audioData->Data(), audioData->Capacity(), data.data.data(), data.data.size());
    if (ret != EOK) {
        DHLOGE("%s: Copy audio data failed.", LOG_TAG);
        return HDF_FAILURE;
    }
    ret = callback_->WriteStreamData(adpName, devId, audioData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: WriteStreamData failed.", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::ReadStreamData(const std::string &adpName, int32_t devId,
    OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioData &data)
{
    DHLOGI("%s: Read Stream Data", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: Register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }
    data.data.resize(DEFAULT_AUDIO_DATA_SIZE);
    if (data.data.size() != DEFAULT_AUDIO_DATA_SIZE) {
        DHLOGE("%s: Audio data size is not support.", LOG_TAG);
        return HDF_FAILURE;
    }

    std::shared_ptr<AudioData> audioData = std::make_shared<AudioData>(DEFAULT_AUDIO_DATA_SIZE);
    int32_t ret = callback_->ReadStreamData(adpName, devId, audioData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: ReadStreamData failed.", LOG_TAG);
        return HDF_FAILURE;
    }
    ret = memcpy_s(data.data.data(), data.data.size(), audioData->Data(), audioData->Capacity());
    if (ret != EOK) {
        DHLOGE("%s: Copy audio data failed.", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}
} // DistributedHardware
} // OHOS
