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

#include "audio_ctrl_channel.h"
#include "audio_ctrl_transport.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioCtrlTransport"

namespace OHOS {
namespace DistributedHardware {
int32_t AudioCtrlTransport::SetUp(const std::shared_ptr<IAudioCtrlTransCallback> &callback)
{
    DHLOGI("SetUp.");
    if (!callback) {
        DHLOGE("callback is null.");
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    ctrlTransCallback_ = callback;
    int32_t ret = InitAudioCtrlTrans(devId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("SetUp failed ret: %d.", ret);
        return ret;
    }

    DHLOGI("SetUp success.");
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::Release()
{
    DHLOGI("Release.");
    if (!audioChannel_) {
        DHLOGE("Channel is already release.");
        return DH_SUCCESS;
    }

    int32_t ret = audioChannel_->ReleaseSession();
    if (ret != DH_SUCCESS) {
        DHLOGE("Release channel session failed ret: %d.", ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    audioChannel_ = nullptr;

    DHLOGI("Release success.");
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::Start()
{
    DHLOGI("Start.");
    if (!audioChannel_) {
        DHLOGE("Channel is null.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int ret = audioChannel_->OpenSession();
    if (ret != DH_SUCCESS) {
        DHLOGE("Open channel session failed ret: %d.", ret);
        return ret;
    }

    DHLOGI("Start success.");
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::Stop()
{
    DHLOGI("Stop.");
    if (!audioChannel_) {
        DHLOGE("Channel is already release.");
        return DH_SUCCESS;
    }

    int32_t ret = audioChannel_->CloseSession();
    if (ret != DH_SUCCESS) {
        DHLOGE("Close Session failed ret: %d.", ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    DHLOGI("Stop success.");
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::SendAudioEvent(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("Send audio event.");
    if (!event) {
        DHLOGE("Audio event is null.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    if (!audioChannel_) {
        DHLOGE("Channel is null.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = audioChannel_->SendEvent(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("Send data failed.");
    }

    DHLOGI("Send Audio Event success.");
    return DH_SUCCESS;
}

void AudioCtrlTransport::OnSessionOpened()
{
    DHLOGI("On Channel Session Opened.");
    std::shared_ptr<IAudioCtrlTransCallback> callback = ctrlTransCallback_.lock();
    if (callback == nullptr) {
        DHLOGE("On Channel Session Opened. callback is nullptr.");
        return;
    }
    callback->OnStateChange(AudioEventType::CTRL_OPENED);
}

void AudioCtrlTransport::OnSessionClosed()
{
    DHLOGI("On Channel Session Closed.");
    std::shared_ptr<IAudioCtrlTransCallback> callback = ctrlTransCallback_.lock();
    if (callback == nullptr) {
        DHLOGE("On Channel Session Closed, callback is nullptr.");
        return;
    }
    callback->OnStateChange(AudioEventType::CTRL_CLOSED);
}

void AudioCtrlTransport::OnDataReceived(const std::shared_ptr<AudioData> &data)
{
    (void)data;
}

void AudioCtrlTransport::OnEventReceived(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("audio event received.");
    std::shared_ptr<IAudioCtrlTransCallback> callback = ctrlTransCallback_.lock();
    if (!callback) {
        DHLOGE("CtrlTrans callback is null.");
        return;
    }
    callback->OnEventReceived(event);
}

int32_t AudioCtrlTransport::InitAudioCtrlTrans(const std::string &netWordId)
{
    if (audioChannel_ == nullptr) {
        audioChannel_ = std::make_shared<AudioCtrlChannel>(netWordId);
    }

    int32_t ret = RegisterChannelListener();
    if (ret != DH_SUCCESS) {
        DHLOGE("Register channel listener failed ret: %d.", ret);
        audioChannel_ = nullptr;
        return ret;
    }

    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::RegisterChannelListener()
{
    DHLOGI("RegisterChannelListener.");
    std::shared_ptr<IAudioChannelListener> listener = shared_from_this();
    if (!listener) {
        DHLOGE("Channel listener is null.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    if (audioChannel_ == nullptr) {
        DHLOGE("Channel is null.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = audioChannel_->CreateSession(listener, CTRL_SESSION_NAME);
    if (ret != DH_SUCCESS) {
        DHLOGE("Create session failed ret: %d.", ret);
        return ret;
    }
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
