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

#include "audio_encode_transport.h"

#include "audio_data_channel.h"
#include "audio_encoder_processor.h"
#include "audio_param.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AudioEncodeTransport::SetUp(const AudioParam &localParam, const AudioParam &remoteParam,
    const std::shared_ptr<IAudioDataTransCallback> &callback)
{
    if (callback == nullptr) {
        DHLOGE("%s: The parameter is empty.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    dataTransCallback_ = callback;
    auto ret = InitAudioEncodeTrans(localParam, remoteParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Init audio encode transport, ret: %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    DHLOGI("%s: SetUp success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioEncodeTransport::Start()
{
    DHLOGI("%s: Start.", LOG_TAG);
    if (processor_ == nullptr || audioChannel_ == nullptr) {
        DHLOGE("%s: Processor or channel is null, setup first.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }
    auto ret = audioChannel_->OpenSession();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Open channel session failed ret: %d.", LOG_TAG, ret);
        audioChannel_ =   nullptr;
        return ret;
    }
    ret = processor_->StartAudioProcessor();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Open audio processor failed ret: %d.", LOG_TAG, ret);
        processor_ = nullptr;
        return ERR_DH_AUDIO_TRANS_PROCESSOR_FAILED;
    }
    DHLOGI("%s: Start success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioEncodeTransport::Stop()
{
    DHLOGI("%s: Stop.", LOG_TAG);
    bool stopStatus = true;
    int32_t ret;
    if (processor_ != nullptr) {
        ret = processor_->StopAudioProcessor();
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Stop audio processor failed, ret: %d.", LOG_TAG, ret);
            stopStatus = false;
        }
    }
    if (audioChannel_ != nullptr) {
        ret = audioChannel_->CloseSession();
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Close session failed, ret: %d.", LOG_TAG, ret);
            stopStatus = false;
        }
    }
    if (!stopStatus) {
        DHLOGE("%s: The stopStatus is false: %d.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    DHLOGI("%s: Stop success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioEncodeTransport::Release()
{
    DHLOGI("%s: Stop.", LOG_TAG);
    bool releaseStatus = true;
    int32_t ret;
    if (processor_ != nullptr) {
        ret = processor_->ReleaseAudioProcessor();
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Release audio processor failed, ret: %d.", LOG_TAG, ret);
            releaseStatus = false;
        }
    }
    if (audioChannel_ != nullptr) {
        ret = audioChannel_->ReleaseSession();
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Release session failed, ret: %d.", LOG_TAG, ret);
            releaseStatus = false;
        }
    }
    if (!releaseStatus) {
        DHLOGE("%s: The releaseStatus is false: %d.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    DHLOGI("%s: Release success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioEncodeTransport::FeedAudioData(std::shared_ptr<AudioData> &audioData)
{
    DHLOGI("%s: Feed audio data.", LOG_TAG);
    if (!processor_) {
        DHLOGE("%s: Processor null, setup first.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = processor_->FeedAudioProcessor(audioData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Feed audio processor failed, ret: %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    return DH_SUCCESS;
}
int32_t AudioEncodeTransport::RequestAudioData(std::shared_ptr<AudioData> &audioData)
{
    return DH_SUCCESS;
}
int32_t AudioEncodeTransport::InitAudioEncodeTrans(const AudioParam &localParam, const AudioParam &remoteParam)
{
    int32_t ret = RegisterChannelListener();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register channel listener failed, ret: %d.", LOG_TAG, ret);
        audioChannel_ = nullptr;
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    ret = RegisterProcessorListener(localParam, remoteParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register processor listener failed, ret: %d.", LOG_TAG, ret);
        processor_ = nullptr;
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    return DH_SUCCESS;
}

int32_t AudioEncodeTransport::RegisterChannelListener()
{
    DHLOGI("%s: Register channel listener.", LOG_TAG);
    audioChannel_ = std::make_shared<AudioDataChannel>(peerDevId_);
    if (audioChannel_ == nullptr) {
        DHLOGE("%s: Create audio channel failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    auto result = audioChannel_->CreateSession(shared_from_this());
    if (result != DH_SUCCESS) {
        DHLOGE("%s: CreateSession failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    return DH_SUCCESS;
}

int32_t AudioEncodeTransport::RegisterProcessorListener(const AudioParam &localParam, const AudioParam &remoteParam)
{
    DHLOGI("%s: Register processor listener.", LOG_TAG);
    processor_ = std::make_shared<AudioEncoderProcessor>();
    if (audioChannel_ == nullptr) {
        DHLOGE("%s: Create audio processor failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    auto ret = processor_->ConfigureAudioProcessor(localParam.comParam, remoteParam.comParam, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Configure audio processor failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    return DH_SUCCESS;
}

void AudioEncodeTransport::OnSessionOpened()
{
    DHLOGI("%s: On channel session opened.", LOG_TAG);
    if (dataTransCallback_ == nullptr) {
        DHLOGI("%s: On channel session opened. callback is nullptr.", LOG_TAG);
        return;
    }
    dataTransCallback_->OnStateChange(AudioEventType::DATA_OPENED);
}

void AudioEncodeTransport::OnSessionClosed()
{
    DHLOGI("%s: On channel session close.", LOG_TAG);
    if (dataTransCallback_ == nullptr) {
        DHLOGI("%s: On channel session closed. callback is nullptr.", LOG_TAG);
        return;
    }
    dataTransCallback_->OnStateChange(AudioEventType::DATA_CLOSED);
}

void AudioEncodeTransport::OnDataReceived(const std::shared_ptr<AudioData> &data)
{
    (void)data;
}

void AudioEncodeTransport::OnEventReceived(const std::shared_ptr<AudioEvent> &event)
{
    (void)event;
}

void AudioEncodeTransport::OnAudioDataDone(const std::shared_ptr<AudioData> &outputData)
{
    DHLOGI("%s: On audio data done.", LOG_TAG);
    if (!audioChannel_) {
        DHLOGE("%s: Channel is null, setup first.", LOG_TAG);
        return;
    }
    int32_t ret = audioChannel_->SendData(outputData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Send data failed ret: %d.", LOG_TAG, ret);
        return;
    }
}

void AudioEncodeTransport::OnStateNotify(const AudioEvent &event)
{
    (void)event;
}
} // namespace DistributedHardware
} // namespace OHOS
