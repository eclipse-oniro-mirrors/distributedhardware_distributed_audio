/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dmic_client.h"

#include "daudio_constants.h"

#include <chrono>

namespace OHOS {
namespace DistributedHardware {
DMicClient::~DMicClient()
{
    if (micTrans_ != nullptr) {
        DHLOGI("%s: ~DMicClient. Release mic client.", LOG_TAG);
        StopCapture();
    }
}

int32_t DMicClient::OnStateChange(int32_t type)
{
    DHLOGI("%s: On state change type: %d.", LOG_TAG, type);
    switch (type) {
        case AudioEventType::DATA_OPENED: {
            isChannelReady_ = true;
            std::unique_lock<std::mutex> lck(channelWaitMutex_);
            channelWaitCond_.notify_all();
            DHLOGI("%s: Data opened.", LOG_TAG);
            isBlocking_ = true;
            isCaptureReady_.store(true);
            captureDataThread_ = std::thread(&DMicClient::CaptureThreadRunning, this);
            return DH_SUCCESS;
        }
        case AudioEventType::DATA_CLOSED: {
            std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
            event->type = AudioEventType::MIC_CLOSED;
            event->content = "";
            eventCallback_->NotifyEvent(event);
            DHLOGI("%s: Data closed.", LOG_TAG);
            return DH_SUCCESS;
        }
        default:
            DHLOGE("%s: Invalid parameter type: %d.", LOG_TAG, type);
            return ERR_DH_AUDIO_CLIENT_STATE_IS_INVALID;
    }
}

int32_t DMicClient::SetUp(const AudioParam &param)
{
    DHLOGI("%s: Set mic client parameters {sampleRate: %d, bitFormat: %d, channelMask: %d, sourceType: %d}.", LOG_TAG,
        param.comParam.sampleRate, param.comParam.bitFormat, param.comParam.channelMask, param.CaptureOpts.sourceType);
    audioParam_ = param;
    audioParam_.comParam.bitFormat = SAMPLE_S16LE;
    AudioStandard::AudioCapturerOptions capturerOptions = {
        {
            static_cast<AudioStandard::AudioSamplingRate>(audioParam_.comParam.sampleRate),
            AudioStandard::AudioEncodingType::ENCODING_PCM,
            static_cast<AudioStandard::AudioSampleFormat>(SAMPLE_S16LE),
            static_cast<AudioStandard::AudioChannel>(audioParam_.comParam.channelMask),
        },
        {
            static_cast<AudioStandard::SourceType>(audioParam_.CaptureOpts.sourceType),
            NUMBER_ZERO,
        }
    };
    audioCapturer_ = AudioStandard::AudioCapturer::Create(capturerOptions);
    if (audioCapturer_ == nullptr) {
        DHLOGE("%s: Audio capturer create failed.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_CREATE_CAPTURER_FAILED;
    }

    if (micTrans_ == nullptr) {
        micTrans_ = std::make_shared<AudioEncodeTransport>(devId_);
    }
    int32_t ret = micTrans_->SetUp(audioParam_, audioParam_, shared_from_this(), "mic");
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic trans setup failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DMicClient::StartCapture()
{
    DHLOGI("%s: Start capturer.", LOG_TAG);
    if (audioCapturer_ == nullptr || micTrans_ == nullptr) {
        DHLOGE("%s: The capturer or mictrans is not instantiated.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_CAPTURER_OR_MICTRANS_INSTANCE;
    }
    if (!audioCapturer_->Start()) {
        DHLOGE("%s: Audio capturer start failed.", LOG_TAG);
        if (!audioCapturer_->Release()) {
            DHLOGE("%s: Audio capturer release failed.", LOG_TAG);
            return ERR_DH_AUDIO_CLIENT_CAPTURER_RELEASE_FAILED;
        }
        return ERR_DH_AUDIO_CLIENT_CAPTURER_START_FAILED;
    }

    int32_t ret = micTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic trans start failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s: Wait for channel session opened.", LOG_TAG);
    std::unique_lock<std::mutex> lck(channelWaitMutex_);
    auto status = channelWaitCond_.wait_for(lck, std::chrono::seconds(CHANNEL_WAIT_SECONDS),
        [this]() { return isChannelReady_; });
    if (!status) {
        DHLOGI("%s: Open channel session timeout(%ds).", LOG_TAG, CHANNEL_WAIT_SECONDS);
        return ERR_DH_AUDIO_CLIENT_TRANS_TIMEOUT;
    }
    return DH_SUCCESS;
}

void DMicClient::CaptureThreadRunning()
{
    DHLOGI("%s: Start the capturer thread.", LOG_TAG);
    size_t bufferLen = NUMBER_ZERO;
    int32_t ret = audioCapturer_->GetBufferSize(bufferLen);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Failed to get minimum buffer.", LOG_TAG);
        return;
    }
    DHLOGI("%s: Obtains the minimum buffer length, bufferlen: %d.", LOG_TAG, bufferLen);
    while (isCaptureReady_.load()) {
        std::shared_ptr<AudioData> audioData = std::make_shared<AudioData>(bufferLen);
        size_t bytesRead = NUMBER_ZERO;
        while (bytesRead < bufferLen) {
            auto start = std::chrono::high_resolution_clock::now();
            int32_t len = audioCapturer_->Read(*(audioData->Data() + bytesRead), bufferLen - bytesRead, isBlocking_);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            DHLOGI("%s: Audio Capturer Read in microseconds TimeTaken =(%ds).", LOG_TAG, (long long)duration.count());
            if (len >= NUMBER_ZERO) {
                bytesRead += len;
            } else {
                bytesRead = len;
                break;
            }
        }
        if (bytesRead < NUMBER_ZERO) {
            DHLOGI("%s: Bytes read failed, bytesRead: %d.", LOG_TAG, bytesRead);
            break;
        } else if (bytesRead == NUMBER_ZERO) {
            continue;
        }

        int32_t ret = micTrans_->FeedAudioData(audioData);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Failed to send data.", LOG_TAG);
        }
    }
}

int32_t DMicClient::StopCapture()
{
    DHLOGI("%s: Stop capturer.", LOG_TAG);
    if (audioCapturer_ == nullptr || micTrans_ == nullptr) {
        DHLOGE("%s: The capturer or mictrans is not instantiated.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_CAPTURER_OR_MICTRANS_INSTANCE;
    }
    isBlocking_ = false;
    isCaptureReady_.store(false);
    if (captureDataThread_.joinable()) {
        captureDataThread_.join();
    }

    int32_t ret = micTrans_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic trans stop failed.", LOG_TAG);
        return ret;
    }
    ret = micTrans_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic trans release failed.", LOG_TAG);
        return ret;
    }
    micTrans_ = nullptr;

    if (!audioCapturer_->Stop()) {
        DHLOGE("%s: Audio capturer stop failed.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_CAPTURER_STOP_FAILED;
    }
    if (!audioCapturer_->Release()) {
        DHLOGE("%s: Audio capturer release failed.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_CAPTURER_FREE_FAILED;
    }
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS
