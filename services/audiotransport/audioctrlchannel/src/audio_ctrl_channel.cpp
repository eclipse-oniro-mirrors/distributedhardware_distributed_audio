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

#include <securec.h>

#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"

using json = nlohmann::json;
namespace OHOS {
namespace DistributedHardware {
int32_t AudioCtrlChannel::CreateSession(const std::shared_ptr<IAudioChannelListener> &listener,
    const std::string &sessionName)
{
    DHLOGI("%s: CreateSession, peerDevId: %s.", LOG_TAG, GetAnonyString(peerDevId_).c_str());
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ERR_DH_AUDIO_TRANS_NULL_VALUE,
            "daudio channel listener is null.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_CREATE_CTRL_SESSION);
    int32_t ret =
        SoftbusAdapter::GetInstance().CreateSoftbusSessionServer(PKG_NAME, sessionName, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Create softbus session failed ret: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daudio create softbus session failed.");
        return ret;
    }

    std::shared_ptr<ISoftbusListener> softbusListener = shared_from_this();
    ret = SoftbusAdapter::GetInstance().RegisterSoftbusListener(softbusListener, sessionName, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register softbus adapter listener failed ret: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daudio register softbus adapter listener failed.");
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
    DAUDIO_SYNC_TRACE(DAUDIO_RELEASE_CTRL_SESSION);
    int32_t ret = SoftbusAdapter::GetInstance().RemoveSoftbusSessionServer(PKG_NAME, sessionName_, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release softbus session failed ret: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daudio release softbus session failed.");
        return ret;
    }

    ret = SoftbusAdapter::GetInstance().UnRegisterSoftbusListener(sessionName_, peerDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: UnRegister softbus adapter listener failed ret: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daudio unRegister softbus adapter listener failed.");
        return ret;
    }
    channelListener_.reset();

    DHLOGI("%s: Release softbus session success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioCtrlChannel::OpenSession()
{
    DHLOGI("%s: OpenSession, peerDevId: %s.", LOG_TAG, GetAnonyString(peerDevId_).c_str());
    DaudioStartAsyncTrace(DAUDIO_OPEN_CTRL_SESSION, DAUDIO_OPEN_CTRL_SESSION_TASKID);
    int32_t sessionId =
        SoftbusAdapter::GetInstance().OpenSoftbusSession(sessionName_, sessionName_, peerDevId_);
    if (sessionId < 0) {
        DHLOGE("%s: Open ctrl session failed, ret: %d.", LOG_TAG, sessionId);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ERR_DH_AUDIO_TRANS_ERROR,
            "daudio open ctrl session failed.");
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
        DHLOGI("%s: Session is already closed.", LOG_TAG);
        return DH_SUCCESS;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_CLOSE_CTRL_SESSION);
    int32_t ret = SoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Close ctrl session failed ret: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_OPT_FAIL, ret,
            "daudio close ctrl session failed.");
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
    DHLOGI("%s: Send event, sessionId: %d.", LOG_TAG, sessionId_);
    if (!audioEvent) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    json jAudioEvent;
    jAudioEvent[KEY_TYPE] = audioEvent->type;
    jAudioEvent[KEY_CONTENT] = audioEvent->content;
    std::string message = jAudioEvent.dump();
    int ret = SendMsg(message);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Send audio event failed ret: %d.", LOG_TAG, ret);
        return ret;
    }

    return DH_SUCCESS;
}

int32_t AudioCtrlChannel::SendMsg(string &message)
{
    DHLOGI("%s: Start SendMsg.", LOG_TAG);
    uint8_t *buf = (uint8_t *)calloc((MSG_MAX_SIZE), sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("%s: SendMsg: malloc memory failed", LOG_TAG);
        return ERR_DH_AUDIO_CTRL_CHANNEL_SEND_MSG_FAIL;
    }
    int32_t outLen = 0;
    if (memcpy_s(buf, MSG_MAX_SIZE, (const uint8_t *)message.c_str(), message.size()) != DH_SUCCESS) {
        DHLOGE("%s: SendMsg: memcpy memory failed", LOG_TAG);
        free(buf);
        return ERR_DH_AUDIO_CTRL_CHANNEL_SEND_MSG_FAIL;
    }
    outLen = (int32_t)message.size();
    int32_t ret = SoftbusAdapter::GetInstance().SendSoftbusBytes(sessionId_, buf, outLen);
    free(buf);
    return ret;
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
    DaudioFinishAsyncTrace(DAUDIO_OPEN_CTRL_SESSION, DAUDIO_OPEN_CTRL_SESSION_TASKID);
    sessionId_ = sessionId;
}

void AudioCtrlChannel::OnSessionClosed(int32_t sessionId)
{
    DHLOGI("%s: OnCtrlSessionClosed, sessionId: %d.", LOG_TAG, sessionId);
    if (sessionId_ == 0) {
        DHLOGI("%s: Session already closed.", LOG_TAG);
        return;
    }
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
    DHLOGI("%s: OnBytesReceived, sessionId: %d, dataLen: %d.", LOG_TAG, sessionId, dataLen);
    if (sessionId < 0 || data == nullptr || dataLen <= 0) {
        DHLOGE("%s: OnBytesReceived param check failed", LOG_TAG);
        return;
    }
    std::shared_ptr<IAudioChannelListener> listener = channelListener_.lock();
    if (!listener) {
        DHLOGE("%s: Channel listener is null.", LOG_TAG);
        return;
    }

    uint8_t *buf = (uint8_t *)calloc(dataLen + 1, sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("%s: OnBytesReceived: malloc memory failed.", LOG_TAG);
        return;
    }

    if (memcpy_s(buf, dataLen + 1, (const uint8_t *)data, dataLen) != DH_SUCCESS) {
        DHLOGE("%s: OnBytesReceived: memcpy memory failed.", LOG_TAG);
        free(buf);
        return;
    }

    std::string message(buf, buf + dataLen);
    DHLOGI("%s: OnBytesReceived message: %s.", LOG_TAG, message.c_str());
    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    json jParam = json::parse(message, nullptr, false);
    if (from_audioEventJson(jParam, audioEvent) != DH_SUCCESS) {
        DHLOGE("%s: OnBytesReceived, Get audioEvent from json failed.", LOG_TAG);
        return;
    }
    free(buf);
    DHLOGI("%s: OnBytesReceived end", LOG_TAG);

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

int from_audioEventJson(const json &j, std::shared_ptr<AudioEvent> &audioEvent)
{
    if (j.is_discarded()) {
        DHLOGE("AudioCtrlChannel: Json data is discarded.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }

    if (!j.contains(KEY_TYPE) || !j.contains(KEY_CONTENT)) {
        DHLOGE("AudioCtrlChannel: Some key values do not exist in json data.");
        return ERR_DH_AUDIO_TRANS_NULL_VALUE;
    }
    j.at(KEY_TYPE).get_to(audioEvent->type);
    j.at(KEY_CONTENT).get_to(audioEvent->content);
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
