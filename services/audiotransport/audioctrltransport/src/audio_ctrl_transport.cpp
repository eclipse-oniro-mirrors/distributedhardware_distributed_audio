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

namespace OHOS {
namespace DistributedHardware {
int32_t AudioCtrlTransport::SetUp(const std::shared_ptr<IAudioCtrlTransCallback> &callback)
{
    DHLOGI("%s: SetUp.", LOG_TAG);
    if (!callback) {
        DHLOGE("%s: callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    ctrlTransCallback_ = callback;
    int32_t ret = InitAudioCtrlTrans(devId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: SetUp failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    DHLOGI("%s: SetUp success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::Release()
{
    DHLOGI("%s: Release.", LOG_TAG);
    if (!audioChannel_) {
        DHLOGE("%s: Channel is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = audioChannel_->ReleaseSession();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release channel session failed ret: %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    audioChannel_ = nullptr;

    DHLOGI("%s: Release success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::Start()
{
    DHLOGI("%s: Start.", LOG_TAG);
    if (!audioChannel_) {
        DHLOGE("%s: Channel is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int ret = audioChannel_->OpenSession();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Open channel session failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    DHLOGI("%s: Start success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::Stop()
{
    DHLOGI("%s: Stop.", LOG_TAG);
    if (!audioChannel_) {
        DHLOGE("%s: Channel is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = audioChannel_->CloseSession();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Close Session failed ret: %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    DHLOGI("%s: Stop success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::SendAudioEvent(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Send audio event.", LOG_TAG);
    if (!event) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    if (!audioChannel_) {
        DHLOGE("%s: Channel is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = audioChannel_->SendEvent(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:Send data failed.", LOG_TAG);
    }

    DHLOGE("%s: Send Audio Event success.", LOG_TAG);
    return DH_SUCCESS;
}

void AudioCtrlTransport::OnSessionOpened()
{
    DHLOGI("%s: On Channel Session Opened.", LOG_TAG);
    std::shared_ptr<IAudioCtrlTransCallback> callback = ctrlTransCallback_.lock();
    if (callback == nullptr) {
        DHLOGE("%s: On Channel Session Opened. callback is nullptr.", LOG_TAG);
        return;
    }
    callback->OnStateChange(AudioEventType::CTRL_OPENED);
}

void AudioCtrlTransport::OnSessionClosed()
{
    DHLOGI("%s: On Channel Session Closed.", LOG_TAG);
    std::shared_ptr<IAudioCtrlTransCallback> callback = ctrlTransCallback_.lock();
    if (callback == nullptr) {
        DHLOGE("%s: On Channel Session Closed, callback is nullptr.", LOG_TAG);
        return;
    }
    callback->OnStateChange(AudioEventType::CTRL_CLOSED);
}

void AudioCtrlTransport::OnDataReceived(const std::shared_ptr<AudioData> &data) {}

void AudioCtrlTransport::OnEventReceived(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: audio event received.", LOG_TAG);
    std::shared_ptr<IAudioCtrlTransCallback> callback = ctrlTransCallback_.lock();
    if (!callback) {
        DHLOGE("%s: CtrlTrans callback is null.", LOG_TAG);
        return;
    }
    callback->OnEventReceived(event);
}

int32_t AudioCtrlTransport::InitAudioCtrlTrans(const std::string &netWordId)
{
    audioChannel_ = std::make_shared<AudioCtrlChannel>(netWordId);
    if (!audioChannel_) {
        DHLOGE("%s: Create audio ctrl channel failed.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = RegisterChannelListener();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register channel listener failed ret: %d.", LOG_TAG, ret);
        audioChannel_ = nullptr;
        return ret;
    }

    return DH_SUCCESS;
}

int32_t AudioCtrlTransport::RegisterChannelListener()
{
    DHLOGI("%s: RegisterChannelListener.", LOG_TAG);
    std::shared_ptr<IAudioChannelListener> listener = shared_from_this();
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    if (audioChannel_ == nullptr) {
        DHLOGE("%s: Channel is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = audioChannel_->CreateSession(listener);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Create session failed ret: %d.", LOG_TAG);
        return ret;
    }

    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
