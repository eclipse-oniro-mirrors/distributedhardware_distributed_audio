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

#include "audio_types.h"

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
        DHLOGE("%s: Register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }
    int32_t ret = callback_->OpenDevice(adpName, devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Call hdi callback failed", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::CloseDevice(const std::string& adpName, int32_t devId)
{
    DHLOGI("%s: CloseDevice enter", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: Register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }
    int32_t ret = callback_->CloseDevice(adpName, devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Rall hdi callback failed", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::GetAudioParamHDF(const AudioParameter& param, AudioParamHDF& paramHDF)
{
    paramHDF.sampleRate = static_cast<AudioSampleRate>(param.sampleRate);
    paramHDF.channelMask = static_cast<AudioChannel>(param.channelCount);
    switch (static_cast<AudioFormat>(param.format)) {
        case AUDIO_FORMAT_PCM_8_BIT:
            paramHDF.bitFormat = AudioSampleFormat::SAMPLE_U8;
            break;
        case AUDIO_FORMAT_PCM_16_BIT:
            paramHDF.bitFormat = AudioSampleFormat::SAMPLE_S16LE;
            break;
        case AUDIO_FORMAT_PCM_24_BIT:
            paramHDF.bitFormat = AudioSampleFormat::SAMPLE_S24LE;
            break;
        default:
            DHLOGE("%s: Format [%zu] does not support conversion.", LOG_TAG, param.format);
            return HDF_FAILURE;
    }
    switch (static_cast<AudioCategory>(param.streamUsage)) {
        case AUDIO_IN_MEDIA:
            paramHDF.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
            break;
        case AUDIO_IN_COMMUNICATION:
            paramHDF.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
            break;
        case AUDIO_IN_RINGTONE:
            paramHDF.streamUsage = StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE;
            break;
        default:
            DHLOGE("%s: Stream usage [%zu] does not support conversion.", LOG_TAG, param.streamUsage);
            return HDF_FAILURE;
    }
    paramHDF.frameSize = param.frameSize;
    paramHDF.period = param.period;
    paramHDF.ext = param.ext;
    DHLOGI("%s: HDF Param: sample rate %d, channel %d, bit format %d, stream usage %d, frame size %zu, " +
        "period %zu, ext {%s}.", LOG_TAG, paramHDF.sampleRate, paramHDF.channelMask, paramHDF.bitFormat,
        paramHDF.streamUsage, paramHDF.frameSize, paramHDF.period, paramHDF.ext.c_str());
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::SetParameters(const std::string& adpName, int32_t devId, const AudioParameter& param)
{
    DHLOGI("%s: SetParameters enter", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: Register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }

    AudioParamHDF paramHDF;
    int32_t ret = GetAudioParamHDF(param, paramHDF);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get audio HDF param failed.", LOG_TAG);
        return HDF_FAILURE;
    }
    ret = callback_->SetParameters(adpName, devId, paramHDF);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Call hdi callback failed", LOG_TAG);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DAudioManagerCallback::NotifyEvent(const std::string& adpName, int32_t devId,
    const OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioEvent& event)
{
    DHLOGI("%s: NotifyEvent", LOG_TAG);
    if (callback_ == nullptr) {
        DHLOGE("%s: Register hdi callback is nullptr", LOG_TAG);
        return HDF_FAILURE;
    }
    AudioEvent newEvent(AudioEventType::EVENT_UNKNOWN, event.content);
    switch (event.type) {
        case AudioEventHDF::AUDIO_EVENT_VOLUME_SET:
            newEvent.type = AudioEventType::VOLUME_SET;
            break;
        case AudioEventHDF::AUDIO_EVENT_MUTE_SET:
            newEvent.type = AudioEventType::VOLUME_MUTE_SET;
            break;
        default:
            DHLOGE("%s: Unsupport event tpye.", LOG_TAG);
            break;
    }

    int32_t ret = callback_->NotifyEvent(adpName, devId, newEvent);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Call hdi callback failed", LOG_TAG);
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

    std::shared_ptr<AudioData> audioData = std::make_shared<AudioData>(DEFAULT_AUDIO_DATA_SIZE);
    int32_t ret = callback_->ReadStreamData(adpName, devId, audioData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: ReadStreamData failed.", LOG_TAG);
        return HDF_FAILURE;
    }
    data.data.assign(audioData->Data(), audioData->Data()+audioData->Capacity());
    DHLOGI("%s: Read Stream Data success.", LOG_TAG);
    return HDF_SUCCESS;
}
} // DistributedHardware
} // OHOS
