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

#include "audio_encoder_processor.h"

#include "daudio_errorcode.h"
#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "audio_encoder.h"

namespace OHOS {
namespace DistributedHardware {
AudioEncoderProcessor::~AudioEncoderProcessor()
{
    if (audioEncoder_ != nullptr) {
        DHLOGI("%s: ~AudioEncoderProcessor. Release audio processor.", LOG_TAG);
        StopAudioProcessor();
        ReleaseAudioProcessor();
    }
}

int32_t AudioEncoderProcessor::ConfigureAudioProcessor(const AudioCommonParam &localDevParam,
    const AudioCommonParam &remoteDevParam, const std::shared_ptr<IAudioProcessorCallback> &procCallback)
{
    DHLOGI("%s: Configure audio processor.", LOG_TAG);
    if (procCallback == nullptr) {
        DHLOGE("%s: Processor callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    localDevParam_ = localDevParam;
    remoteDevParam_ = remoteDevParam;
    procCallback_ = procCallback;

    audioEncoder_ = std::make_shared<AudioEncoder>();
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioEncoder_->ConfigureAudioCodec(localDevParam, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Configure encoder fail. Error code: %d.", LOG_TAG, ret);
        audioEncoder_ = nullptr;
        return ret;
    }
    return DH_SUCCESS;
}

int32_t AudioEncoderProcessor::ReleaseAudioProcessor()
{
    DHLOGI("%s: Release audio processor.", LOG_TAG);
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        return DH_SUCCESS;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_RELEASE_ENCODER_PROCESSOR);
    int32_t ret = audioEncoder_->ReleaseAudioCodec();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release encoder fail. Error code: %d.", LOG_TAG, ret);
        return ret;
    }

    audioEncoder_ = nullptr;
    return DH_SUCCESS;
}

int32_t AudioEncoderProcessor::StartAudioProcessor()
{
    DHLOGI("%s: Start audio processor.", LOG_TAG);
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ERR_DH_AUDIO_BAD_VALUE,
            "daudio encoder is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_START_ENCODER_PROCESSOR);
    int32_t ret = audioEncoder_->StartAudioCodec();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Start encoder fail. Error code: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daudio start encoder fail.");
        return ret;
    }

    return DH_SUCCESS;
}

int32_t AudioEncoderProcessor::StopAudioProcessor()
{
    DHLOGI("%s: Stop audio processor.", LOG_TAG);
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ERR_DH_AUDIO_BAD_VALUE,
            "daudio encoder is null.");
        return DH_SUCCESS;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_STOP_ENCODER_PROCESSOR);
    int32_t ret = audioEncoder_->StopAudioCodec();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop encoder fail. Error code: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daudio stop decoder fail.");
        return ret;
    }

    return DH_SUCCESS;
}

int32_t AudioEncoderProcessor::FeedAudioProcessor(const std::shared_ptr<AudioData> &inputData)
{
    DHLOGD("%s: Feed audio processor.", LOG_TAG);
    if (inputData == nullptr) {
        DHLOGE("%s: Input data is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioEncoder_->FeedAudioData(inputData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Feed data fail. Error code: %d.", LOG_TAG, ret);
        return ret;
    }

    return DH_SUCCESS;
}

void AudioEncoderProcessor::OnCodecDataDone(const std::shared_ptr<AudioData> &outputData)
{
    if (outputData == nullptr) {
        DHLOGE("%s: Output data is null.", LOG_TAG);
        return;
    }
    DHLOGD("%s: Codec done. Output data size %zu.", LOG_TAG, outputData->Size());

    std::shared_ptr<IAudioProcessorCallback> targetProcCallback_ = procCallback_.lock();
    if (targetProcCallback_ == nullptr) {
        DHLOGE("%s: Processor callback is null.", LOG_TAG);
        return;
    }
    targetProcCallback_->OnAudioDataDone(outputData);
}

void AudioEncoderProcessor::OnCodecStateNotify(const AudioEvent &event)
{
    DHLOGI("%s: Codec state notify.", LOG_TAG);
    std::shared_ptr<IAudioProcessorCallback> targetProcCallback_ = procCallback_.lock();
    if (targetProcCallback_ == nullptr) {
        DHLOGE("%s: Processor callback is null.", LOG_TAG);
        return;
    }
    targetProcCallback_->OnStateNotify(event);
}
} // namespace DistributedHardware
} // namespace OHOS