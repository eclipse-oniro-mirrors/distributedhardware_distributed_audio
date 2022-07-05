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

#include "audio_decode_transport.h"

#include "audio_data_channel.h"
#include "audio_decoder_processor.h"
#include "audio_param.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AudioDecodeTransport::SetUp(const AudioParam &localParam, const AudioParam &remoteParam,
    const std::shared_ptr<IAudioDataTransCallback> &callback, const std::string &role)
{
    if (callback == nullptr) {
        DHLOGE("%s: The parameter is empty.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    dataTransCallback_ = callback;
    auto ret = InitAudioDecodeTransport(localParam, remoteParam, role);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Init audio encode transport, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    DHLOGI("%s: SetUp success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioDecodeTransport::Start()
{
    DHLOGI("%s: Start.", LOG_TAG);
    if (processor_ == nullptr) {
        DHLOGE("%s: Processor  is null, Setup first.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = processor_->StartAudioProcessor();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Open audio processor failed, ret: %d.", LOG_TAG, ret);
        processor_ = nullptr;
        return ERR_DH_AUDIO_TRANS_PROCESSOR_FAILED;
    }
    DHLOGI("%s: Start success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioDecodeTransport::Stop()
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
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    DHLOGI("%s: Stop success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioDecodeTransport::Release()
{
    DHLOGI("%s: Release.", LOG_TAG);
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
        DHLOGE("%s: The releaseStatus is false, ret: %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    DHLOGI("%s: Release success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioDecodeTransport::FeedAudioData(std::shared_ptr<AudioData> &audioData)
{
    (void)audioData;
    return DH_SUCCESS;
}

int32_t AudioDecodeTransport::RequestAudioData(std::shared_ptr<AudioData> &audioData)
{
    DHLOGI("%s: Request audio data.", LOG_TAG);
    std::unique_lock<std::mutex> lock(dataQueueMtx_);
    if (dataQueue_.empty()) {
        usleep(SLEEP_TIME);
        audioData = std::make_shared<AudioData>(FRAME_SIZE);
    } else {
        audioData = dataQueue_.front();
        dataQueue_.pop();
    }
    return DH_SUCCESS;
}

void AudioDecodeTransport::OnSessionOpened()
{
    DHLOGI("%s: On channel session opened.", LOG_TAG);
    if (dataTransCallback_ == nullptr) {
        DHLOGI("%s: On channel session opened. callback is nullptr", LOG_TAG);
        return;
    }
    dataTransCallback_->OnStateChange(AudioEventType::DATA_OPENED);
}

void AudioDecodeTransport::OnSessionClosed()
{
    DHLOGI("%s: On channel session close.", LOG_TAG);
    if (dataTransCallback_ == nullptr) {
        DHLOGI("%s: On channel session close, callback is nullptr.", LOG_TAG);
        return;
    }
    dataTransCallback_->OnStateChange(AudioEventType::DATA_CLOSED);
}

void AudioDecodeTransport::OnDataReceived(const std::shared_ptr<AudioData> &data)
{
    DHLOGI("%s: On audio data received.", LOG_TAG);
    if (processor_ == nullptr) {
        DHLOGE("%s: Processor null, setup first.", LOG_TAG);
        return;
    }

    int32_t ret = processor_->FeedAudioProcessor(data);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Feed audio processor failed ret: %d.", LOG_TAG, ret);
    }
}

void AudioDecodeTransport::OnEventReceived(const std::shared_ptr<AudioEvent> &event)
{
    (void)event;
}

void AudioDecodeTransport::OnAudioDataDone(const std::shared_ptr<AudioData> &outputData)
{
    DHLOGI("%s: On audio data done.", LOG_TAG);
    std::lock_guard<std::mutex> lock(dataQueueMtx_);
    while (dataQueue_.size() > DATA_QUEUE_MAX_SIZE) {
        DHLOGE("%s: Data queue overflow.", LOG_TAG);
        dataQueue_.pop();
    }
    dataQueue_.push(outputData);
}

void AudioDecodeTransport::OnStateNotify(const AudioEvent &event)
{
    (void)event;
}

int32_t AudioDecodeTransport::InitAudioDecodeTransport(const AudioParam &localParam,
    const AudioParam &remoteParam, const std::string &role)
{
    int32_t ret = RegisterChannelListener(role);
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
    audioParam_ = remoteParam;
    return DH_SUCCESS;
}

int32_t AudioDecodeTransport::RegisterChannelListener(const std::string &role)
{
    DHLOGI("%s: Register Channel Listener.", LOG_TAG);
    audioChannel_ = std::make_shared<AudioDataChannel>(peerDevId_);
    if (audioChannel_ == nullptr) {
        DHLOGE("%s: Create audio channel failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    int32_t result;
    if (role == "speaker") {
        result = audioChannel_->CreateSession(shared_from_this(), DATA_SPEAKER_SESSION_NAME);
    } else {
        result = audioChannel_->CreateSession(shared_from_this(), DATA_MIC_SESSION_NAME);
    }
    if (result != DH_SUCCESS) {
        DHLOGE("%s: CreateSession failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    return DH_SUCCESS;
}

int32_t AudioDecodeTransport::RegisterProcessorListener(const AudioParam &localParam, const AudioParam &remoteParam)
{
    DHLOGI("%s: Register processor listener.", LOG_TAG);
    processor_ = std::make_shared<AudioDecoderProcessor>();
    if (audioChannel_ == nullptr) {
        DHLOGE("%s: Create audio processor failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    auto ret = processor_->ConfigureAudioProcessor(localParam.comParam, remoteParam.comParam, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Configure audio processor failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
