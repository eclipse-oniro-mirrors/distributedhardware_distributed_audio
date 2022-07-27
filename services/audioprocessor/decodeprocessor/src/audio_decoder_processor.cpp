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

#include "audio_decoder_processor.h"

#include "daudio_errorcode.h"
#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "audio_decoder.h"

namespace OHOS {
namespace DistributedHardware {
AudioDecoderProcessor::~AudioDecoderProcessor()
{
    if (audioDecoder_ != nullptr) {
        DHLOGI("%s: ~AudioDecoderProcessor. Release audio processor.", LOG_TAG);
        StopAudioProcessor();
        ReleaseAudioProcessor();
    }
}

int32_t AudioDecoderProcessor::ConfigureAudioProcessor(const AudioCommonParam &localDevParam,
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

    audioDecoder_ = std::make_shared<AudioDecoder>();
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioDecoder_->ConfigureAudioCodec(localDevParam, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Configure decoder fail. Error code: %d.", LOG_TAG, ret);
        audioDecoder_ = nullptr;
        return ret;
    }
    return DH_SUCCESS;
}

int32_t AudioDecoderProcessor::ReleaseAudioProcessor()
{
    DHLOGI("%s: Release audio processor.", LOG_TAG);
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_RELEASE_DECODER_PROCESSOR);
    int32_t ret = audioDecoder_->ReleaseAudioCodec();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release decoder fail. Error code: %d.", LOG_TAG, ret);
        return ret;
    }

    audioDecoder_ = nullptr;
    return DH_SUCCESS;
}

int32_t AudioDecoderProcessor::StartAudioProcessor()
{
    DHLOGI("%s: Start audio processor.", LOG_TAG);
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ERR_DH_AUDIO_BAD_VALUE,
            "daduio decoder is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_START_DECODER_PROCESSOR);
    int32_t ret = audioDecoder_->StartAudioCodec();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Start decoder fail. Error code: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daduio start decoder fail.");
        return ret;
    }

    DHLOGI("%s: Start audio processor success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioDecoderProcessor::StopAudioProcessor()
{
    DHLOGI("%s: Stop audio processor.", LOG_TAG);
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ERR_DH_AUDIO_BAD_VALUE,
            "daduio decoder is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_STOP_DECODER_PROCESSOR);
    int32_t ret = audioDecoder_->StopAudioCodec();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop decoder fail. Error code: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daduio stop decoder fail.");
        return ret;
    }

    return DH_SUCCESS;
}

int32_t AudioDecoderProcessor::FeedAudioProcessor(const std::shared_ptr<AudioData> &inputData)
{
    DHLOGI("%s: Feed audio processor.", LOG_TAG);
    if (inputData == nullptr) {
        DHLOGE("%s: Input data is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioDecoder_->FeedAudioData(inputData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Feed data fail. Error code: %d.", LOG_TAG, ret);
        return ret;
    }

    return DH_SUCCESS;
}

void AudioDecoderProcessor::OnCodecDataDone(const std::shared_ptr<AudioData> &outputData)
{
    if (outputData == nullptr) {
        DHLOGE("%s: Output data is null.", LOG_TAG);
        return;
    }
    DHLOGI("%s: Codec done. Output data size %zu.", LOG_TAG, outputData->Size());

    std::shared_ptr<IAudioProcessorCallback> targetProcCallback_ = procCallback_.lock();
    if (targetProcCallback_ == nullptr) {
        DHLOGE("%s: Processor callback is null.", LOG_TAG);
        return;
    }
    targetProcCallback_->OnAudioDataDone(outputData);
}

void AudioDecoderProcessor::OnCodecStateNotify(const AudioEvent &event)
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