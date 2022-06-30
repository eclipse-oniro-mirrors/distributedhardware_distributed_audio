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

namespace OHOS {
namespace DistributedHardware {
int32_t AudioCtrlChannel::CreateSession(const std::shared_ptr<IAudioChannelListener> &listener,
    const std::string &sessionName)
{
    DHLOGI("%s: CreateSession, peerDevId: %s.", LOG_TAG, GetAnonyString(peerDevId_).c_str());
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret =
        SoftbusAdapter::GetInstance().CreateSoftbusSessionServer(PKG_NAME, sessionName, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Create softbus session failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    std::shared_ptr<ISoftbusListener> softbusListener = shared_from_this();
    ret = SoftbusAdapter::GetInstance().RegisterSoftbusListener(softbusListener, sessionName, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register softbus adapter listener failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    channelListener_ = listener;
    sessionName_ = sessionName;
    DHLOGI("%s: Create softbus session success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlChannel::ReleaseSession()
{
    DHLOGI("%s: ReleaseSession, peerDevId: %s", LOG_TAG, GetAnonyString(peerDevId_).c_str());
    int32_t ret = SoftbusAdapter::GetInstance().RemoveSoftbusSessionServer(PKG_NAME, sessionName_, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release softbus session failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    ret = SoftbusAdapter::GetInstance().UnRegisterSoftbusListener(sessionName_, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: UnRegister softbus adapter listener failed ret: %d.", LOG_TAG, ret);
        return ret;
    }
    channelListener_.reset();

    DHLOGI("%s: Release softbus session success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlChannel::OpenSession()
{
    DHLOGI("%s: OpenSession, peerDevId: %s.", LOG_TAG, GetAnonyString(peerDevId_).c_str());
    int32_t sessionId =
        SoftbusAdapter::GetInstance().OpenSoftbusSession(sessionName_, sessionName_, peerDevId_);
    if (sessionId < 0) {
        DHLOGE("%s: Open ctrl session failed, ret: %d.", LOG_TAG, sessionId);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    sessionId_ = sessionId;

    DHLOGI("%s: Open ctrl session success, sessionId: %d.", LOG_TAG, sessionId_);
    return DH_SUCCESS;
}

int32_t AudioCtrlChannel::CloseSession()
{
    DHLOGI("%s: CloseSession, sessionId: %d.", LOG_TAG, sessionId_);
    if (sessionId_ == 0) {
        DHLOGE("%s: Session is not opened.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_SESSION_NOT_OPEN;
    }

    int32_t ret = SoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Close ctrl session failed ret: %d.", LOG_TAG, ret);
        return ret;
    }
    sessionId_ = 0;

    DHLOGI("%s: Close ctrl session success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlChannel::SendData(const std::shared_ptr<AudioData> &data)
{
    (void) data;

    return DH_SUCCESS;
}
int32_t AudioCtrlChannel::SendEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: SendEvent, sessionId: %d.", LOG_TAG, sessionId_);
    if (!audioEvent) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    void *data = audioEvent.get();
    uint32_t dataLen = (audioEvent->content).length() + sizeof(audioEvent->type);

    int32_t ret = SoftbusAdapter::GetInstance().SendSoftbusBytes(sessionId_, data, dataLen);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Send audio event failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    return DH_SUCCESS;
}

void AudioCtrlChannel::OnSessionOpened(int32_t sessionId, int32_t result)
{
    DHLOGI("%s: OnCtrlSessionOpened, sessionId: %d, result: %d.", LOG_TAG, sessionId, result);
    if (result != 0) {
        DHLOGE("%s: Session open failed.", LOG_TAG);
        return;
    }

    std::shared_ptr<IAudioChannelListener> listener = channelListener_.lock();
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return;
    }
    listener->OnSessionOpened();
    sessionId_ = sessionId;
}

void AudioCtrlChannel::OnSessionClosed(int32_t sessionId)
{
    DHLOGI("%s: OnCtrlSessionClosed, sessionId: %d.", LOG_TAG, sessionId);
    std::shared_ptr<IAudioChannelListener> listener = channelListener_.lock();
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return;
    }
    listener->OnSessionClosed();
    sessionId_ = 0;
}

void AudioCtrlChannel::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    if (data == nullptr) {
        DHLOGE("%s: Bytes data is null.", LOG_TAG);
        return;
    }

    std::shared_ptr<IAudioChannelListener> listener = channelListener_.lock();
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return;
    }

    AudioEvent *event = (AudioEvent*)data;
    auto audioEvent = std::make_shared<AudioEvent>();
    audioEvent->type = event->type;
    audioEvent->content = event->content;

    listener->OnEventReceived(audioEvent);
}

void AudioCtrlChannel::OnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *streamFrameInfo)
{
    (void) sessionId;
    (void) data;
    (void) ext;
    (void) streamFrameInfo;

    DHLOGI("%s: OnAudioStreamReceived ctrl channel not support yet.", LOG_TAG);
}
} // namespace DistributedHardware
} // namespace OHOS
