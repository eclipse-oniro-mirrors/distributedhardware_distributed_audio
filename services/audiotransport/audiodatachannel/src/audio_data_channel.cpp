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

#include <securec.h>
#include "audio_data_channel.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AudioDataChannel::CreateSession(const std::shared_ptr<IAudioChannelListener> &listener,
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
        DHLOGE("%s: Create softbus session failed ret.", LOG_TAG);
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

int32_t AudioDataChannel::ReleaseSession()
{
    DHLOGI("%s: ReleaseSession, peerDevId: %s.", LOG_TAG, GetAnonyString(peerDevId_).c_str());
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

int32_t AudioDataChannel::OpenSession()
{
    DHLOGI("%s: OpenSession, peerDevId: %s.", LOG_TAG, GetAnonyString(peerDevId_).c_str());
    int32_t sessionId =
        SoftbusAdapter::GetInstance().OpenSoftbusSession(sessionName_, sessionName_, peerDevId_);
    if (sessionId < 0) {
        DHLOGE("%s: Open audio session failed, ret: %d.", LOG_TAG, sessionId);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    sessionId_ = sessionId;

    DHLOGI("%s: Open audio session success, sessionId: %d.", LOG_TAG, sessionId_);
    return DH_SUCCESS;
}

int32_t AudioDataChannel::CloseSession()
{
    DHLOGI("%s: CloseSession, sessionId: %d.", LOG_TAG, sessionId_);
    if (sessionId_ == 0) {
        DHLOGE("%s: Session is already close.", LOG_TAG);
        return DH_SUCCESS;
    }

    int32_t ret = SoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Close audio session failed ret: %d.", LOG_TAG, ret);
        return ret;
    }
    sessionId_ = 0;

    DHLOGI("%s: Close audio session success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioDataChannel::SendEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void) audioEvent;
    return DH_SUCCESS;
}

int32_t AudioDataChannel::SendData(const std::shared_ptr<AudioData> &audioData)
{
    DHLOGI("%s: SendData, sessionId: %d.", LOG_TAG, sessionId_);
    if (!audioData) {
        DHLOGE("%s: Audio data is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    int32_t ret = SoftbusAdapter::GetInstance().SendSoftbusStream(sessionId_, audioData);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Send audio data failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    return DH_SUCCESS;
}

void AudioDataChannel::OnSessionOpened(int32_t sessionId, int32_t result)
{
    DHLOGI("%s: OnAudioSessionOpened, sessionId: %d, result: %d.", LOG_TAG, sessionId, result);
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

void AudioDataChannel::OnSessionClosed(int32_t sessionId)
{
    DHLOGI("%s: OnAudioSessionClosed, sessionId: %d.", LOG_TAG, sessionId);
    std::shared_ptr<IAudioChannelListener> listener = channelListener_.lock();
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return;
    }
    listener->OnSessionClosed();
    sessionId_ = 0;
}

void AudioDataChannel::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    (void) sessionId;
    (void) data;
    (void) dataLen;

    DHLOGI("%s: OnAudioBytesReceived data channel not support yet.", LOG_TAG);
}

void AudioDataChannel::OnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *streamFrameInfo)
{
    (void) ext;
    (void) streamFrameInfo;

    if (data == nullptr) {
        DHLOGE("%s: Stream data is null.", LOG_TAG);
        return;
    }

    std::shared_ptr<IAudioChannelListener> listener = channelListener_.lock();
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return;
    }

    DHLOGI("%s: OnAudioStreamReceived, sessionId: %d dataSize: %zu.", LOG_TAG, sessionId, data->bufLen);
    auto audioData = std::make_shared<AudioData>(data->bufLen);
    if (!audioData) {
        DHLOGE("%s: audioData is null.", LOG_TAG);
        return;
    }

    int32_t ret = memcpy_s(audioData->Data(), audioData->Capacity(), (uint8_t *)data->buf, data->bufLen);
    if (ret != EOK) {
        DHLOGE("%s: Data memcpy_s failed.", LOG_TAG);
        return;
    }
    listener->OnDataReceived(audioData);
}
} // namespace DistributedHardware
} // namespace OHOS
