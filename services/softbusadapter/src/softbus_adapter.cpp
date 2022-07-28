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

#include "softbus_adapter.h"

#include <securec.h>

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(SoftbusAdapter);
static int32_t AudioOnSoftbusSessionOpened(int32_t sessionId, int32_t result)
{
    return SoftbusAdapter::GetInstance().OnSoftbusSessionOpened(sessionId, result);
}

static void AudioOnSoftbusSessionClosed(int32_t sessionId)
{
    SoftbusAdapter::GetInstance().OnSoftbusSessionClosed(sessionId);
}

static void AudioOnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    SoftbusAdapter::GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

static void AudioOnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *frameInfo)
{
    SoftbusAdapter::GetInstance().OnStreamReceived(sessionId, data, ext, frameInfo);
}

static void AudioOnMessageReceived(int sessionId, const void *data, unsigned int dataLen)
{
    SoftbusAdapter::GetInstance().OnMessageReceived(sessionId, data, dataLen);
}

static void AudioOnQosEvent(int sessionId, int eventId, int tvCount, const QosTv *tvList)
{
    SoftbusAdapter::GetInstance().OnQosEvent(sessionId, eventId, tvCount, tvList);
}

SoftbusAdapter::SoftbusAdapter()
{
    DHLOGI("SoftbusAdapter");
    sessListener_.OnSessionOpened = AudioOnSoftbusSessionOpened;
    sessListener_.OnSessionClosed = AudioOnSoftbusSessionClosed;
    sessListener_.OnBytesReceived = AudioOnBytesReceived;
    sessListener_.OnStreamReceived = AudioOnStreamReceived;
    sessListener_.OnMessageReceived = AudioOnMessageReceived;
    sessListener_.OnQosEvent = AudioOnQosEvent;
}

SoftbusAdapter::~SoftbusAdapter()
{
    DHLOGI("~SoftbusAdapter");
}

int32_t SoftbusAdapter::CreateSoftbusSessionServer(const std::string &pkgName, const std::string &sessionName,
    const std::string &peerDevId)
{
    DHLOGI("%s: CreateSessionServer sess: %s peerDevId: %s.", LOG_TAG, sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    if (pkgName != PKG_NAME) {
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    std::lock_guard<std::mutex> setLock(sessSetMtx_);
    if (mapSessionSet_.find(sessionName) == mapSessionSet_.end()) {
        int32_t ret = CreateSessionServer(pkgName.c_str(), sessionName.c_str(), &sessListener_);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: CreateSessionServer failed.", LOG_TAG);
            return ret;
        }
    } else {
        DHLOGI("%s: Session already create, sessionName: %s.", LOG_TAG, sessionName.c_str());
    }

    mapSessionSet_[sessionName].insert(peerDevId);
    DHLOGI("%s: CreateSessionServer success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t SoftbusAdapter::RemoveSoftbusSessionServer(const std::string &pkgName, const std::string &sessionName,
    const std::string &peerDevId)
{
    DHLOGI("%s: RemoveSessionServer sess: %s peerDevId: %s.", LOG_TAG, sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    std::lock_guard<std::mutex> setLock(sessSetMtx_);
    if (mapSessionSet_.find(sessionName) == mapSessionSet_.end()) {
        DHLOGE("%s: SessionName not find.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION;
    }
    if (mapSessionSet_[sessionName].find(peerDevId) == mapSessionSet_[sessionName].end()) {
        DHLOGE("%s: PerrDevId not find.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION;
    }

    if (mapSessionSet_[sessionName].size() == SINGLE_ITEM) {
        mapSessionSet_[sessionName].erase(peerDevId);
        mapSessionSet_.erase(sessionName);
    }
    DHLOGI("%s: RemoveSessionServer success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t SoftbusAdapter::OpenSoftbusSession(const std::string &localSessionName, const std::string &peerSessionName,
    const std::string &peerDevId)
{
    DHLOGI("%s: OpenSoftbusSession localsess: %s peersess: %s peerDevId: %s.", LOG_TAG, localSessionName.c_str(),
        peerSessionName.c_str(), GetAnonyString(peerDevId).c_str());
    int dataType = TYPE_BYTES;
    int streamType = -1;
    if (localSessionName != CTRL_SESSION_NAME) {
        dataType = TYPE_STREAM;
        streamType = RAW_STREAM;
    }

    SessionAttribute attr = { 0 };
    attr.dataType = dataType;
    attr.linkTypeNum = LINK_TYPE_MAX;
    LinkType linkTypeList[LINK_TYPE_MAX] = {
        LINK_TYPE_WIFI_P2P,
        LINK_TYPE_WIFI_WLAN_5G,
        LINK_TYPE_WIFI_WLAN_2G,
        LINK_TYPE_BR,
    };
    int32_t ret = memcpy_s(attr.linkType, sizeof(attr.linkType), linkTypeList, sizeof(linkTypeList));
    if (ret != EOK) {
        DHLOGE("%s: Data copy failed.", LOG_TAG);
        return ERR_DH_AUDIO_ADAPTER_PARA_ERROR;
    }
    attr.attr.streamAttr.streamType = streamType;
    int32_t sessionId = OpenSession(localSessionName.c_str(), peerSessionName.c_str(), peerDevId.c_str(), "0", &attr);
    if (sessionId < 0) {
        DHLOGE("%s: OpenSession failed sessionId: %d.", LOG_TAG, sessionId);
        return ERR_DH_AUDIO_ADAPTER_OPEN_SESSION_FAIL;
    }

    DHLOGI("%s: OpenSoftbusSession success sessionId: %d.", LOG_TAG, sessionId);
    return sessionId;
}

int32_t SoftbusAdapter::CloseSoftbusSession(int32_t sessionId)
{
    DHLOGI("%s: CloseSoftbusSession, sessionId: %d.", LOG_TAG, sessionId);
    CloseSession(sessionId);

    std::lock_guard<std::mutex> LisLock(listenerMtx_);
    mapSessListeners_.erase(sessionId);

    if (mapSessListeners_.empty()) {
        DHLOGI("%s: Stop softbus send thread.", LOG_TAG);
        isAudioDataReady_ = false;
        if (sendDataThread_.joinable()) {
            sendDataThread_.join();
        }
    }
    DHLOGI("%s: CloseSoftbusSession success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t SoftbusAdapter::SendSoftbusBytes(int32_t sessionId, const void *data, int32_t dataLen)
{
    DHLOGI("%s: SendAudioEvent, sessionId: %d.", LOG_TAG, sessionId);
    int32_t ret = SendBytes(sessionId, data, dataLen);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: SendBytes failed ret:%d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    return DH_SUCCESS;
}

int32_t SoftbusAdapter::SendSoftbusStream(int32_t sessionId, const std::shared_ptr<AudioData> &audioData)
{
    DHLOGI("%s: SendAudioData, sessionId: %d.", LOG_TAG, sessionId);
    if (!audioData) {
        DHLOGE("%s: Audio data is null.", LOG_TAG);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    std::lock_guard<std::mutex> lck(dataQueueMtx_);
    while (audioDataQueue_.size() >= DATA_QUEUE_MAX_SIZE) {
        DHLOGE("%s: Data queue overflow.", LOG_TAG);
        audioDataQueue_.pop();
    }
    std::shared_ptr<SoftbusStreamData> streamData = std::make_shared<SoftbusStreamData>();
    streamData->data = audioData;
    streamData->sessionId = sessionId;
    audioDataQueue_.push(streamData);
    sendDataCond_.notify_all();
    return DH_SUCCESS;
}

int32_t SoftbusAdapter::RegisterSoftbusListener(const std::shared_ptr<ISoftbusListener> &listener,
    const std::string &sessionName, const std::string &peerDevId)
{
    DHLOGI("%s: RegisterListener sess: %s peerDevId: %s.", LOG_TAG, sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    if (listener == nullptr) {
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    std::string strListenerKey = sessionName + "_" + peerDevId;

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapListeners_.find(strListenerKey) != mapListeners_.end()) {
        DHLOGE("%s: Session listener already register.", LOG_TAG);
    }
    mapListeners_.insert(std::make_pair(strListenerKey, listener));

    return DH_SUCCESS;
}

int32_t SoftbusAdapter::UnRegisterSoftbusListener(const std::string &sessionName, const std::string &peerDevId)
{
    DHLOGI("%s: UnRegisterListener sess: %s peerDevId: %s.", LOG_TAG, sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    std::string strListenerKey = sessionName + "_" + peerDevId;

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    mapListeners_.erase(strListenerKey);

    return DH_SUCCESS;
}

int32_t SoftbusAdapter::OnSoftbusSessionOpened(int32_t sessionId, int32_t result)
{
    DHLOGI("%s: OnSessionOpened sessionId: %d, result: %d.", LOG_TAG, sessionId, result);
    if (result != DH_SUCCESS) {
        DHLOGE("%s: OnSessionOpened failed.", LOG_TAG);
        return ERR_DH_AUDIO_ADAPTER_OPEN_SESSION_FAIL;
    }

    std::shared_ptr<ISoftbusListener> &listener = GetSoftbusListenerByName(sessionId);
    if (!listener) {
        DHLOGE("Get softbus listener failed.");
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapSessListeners_.empty()) {
        DHLOGI("%s: Start softbus send thread.", LOG_TAG);
        isAudioDataReady_ = true;
        sendDataThread_ = std::thread(&SoftbusAdapter::SendAudioData, this);
    }
    mapSessListeners_.insert(std::make_pair(sessionId, listener));
    listener->OnSessionOpened(sessionId, result);
    return DH_SUCCESS;
}

void SoftbusAdapter::OnSoftbusSessionClosed(int32_t sessionId)
{
    DHLOGI("%s: OnSessionClosed sessionId:%d.", LOG_TAG, sessionId);
    auto &listener = GetSoftbusListenerById(sessionId);
    if (!listener) {
        DHLOGE("Get softbus listener failed.");
        return;
    }
    listener->OnSessionClosed(sessionId);

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    mapSessListeners_.erase(sessionId);

    if (mapSessListeners_.empty()) {
        DHLOGI("%s: Stop softbus send thread.", LOG_TAG);
        isAudioDataReady_ = false;
        if (sendDataThread_.joinable()) {
            sendDataThread_.join();
        }
    }
}

void SoftbusAdapter::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DHLOGI("%s: OnBytesReceicved sessionId: %d.", LOG_TAG, sessionId);
    if (data == nullptr) {
        DHLOGE("BytesData is null.");
        return;
    }
    if (dataLen == 0 || dataLen > DAUDIO_MAX_RECV_DATA_LEN) {
        DHLOGE ("BytesData length is 0 or too large");
        return;
    }

    auto &listener = GetSoftbusListenerById(sessionId);
    if (!listener) {
        DHLOGE("Get softbus listener failed.");
        return;
    }
    listener->OnBytesReceived(sessionId, data, dataLen);
}

void SoftbusAdapter::OnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *streamFrameInfo)
{
    DHLOGI("%s: OnStreamReceived, sessionId: %d.", LOG_TAG, sessionId);
    if (data == nullptr) {
        DHLOGE("StreamData is null.");
        return;
    }
    if (data->bufLen <= 0 || (uint32_t)(data->bufLen) > DAUDIO_MAX_RECV_DATA_LEN) {
        DHLOGE("StreamData length is illegal or too large, dataLen: %d.", data->bufLen);
        return;
    }

    auto &listener = GetSoftbusListenerById(sessionId);
    if (!listener) {
        DHLOGE("Get softbus listener failed.");
        return;
    }
    listener->OnStreamReceived(sessionId, data, ext, streamFrameInfo);
}

void SoftbusAdapter::OnMessageReceived(int sessionId, const void *data, unsigned int dataLen)
{
    DHLOGI("%s: OnMessageReceived, sessionId: %d.", LOG_TAG, sessionId);
}

void SoftbusAdapter::OnQosEvent(int sessionId, int eventId, int tvCount, const QosTv *tvList)
{
    DHLOGI("%s: OnQosEvent, sessionId: %d.", LOG_TAG, sessionId);
}

std::shared_ptr<ISoftbusListener> &SoftbusAdapter::GetSoftbusListenerByName(int32_t sessionId)
{
    char sessionName[DAUDIO_MAX_SESSION_NAME_LEN] = "";
    char peerDevId[DAUDIO_MAX_DEVICE_ID_LEN] = "";
    int32_t ret = GetPeerSessionName(sessionId, sessionName, sizeof(sessionName));
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: GetPeerSessionName failed ret: %d.", LOG_TAG, ret);
        return nullListener_;
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: GetPeerDeviceId failed ret: %d.", LOG_TAG, ret);
        return nullListener_;
    }

    std::string sessionNameStr(sessionName);
    std::string peerDevIdStr(peerDevId);

    DHLOGI("%s: GetSoftbusListenerByName sessionName: %s, peerDevId: %s.", LOG_TAG, sessionNameStr.c_str(),
        GetAnonyString(peerDevIdStr).c_str());
    std::string strListenerKey = sessionNameStr + "_" + peerDevIdStr;

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapListeners_.find(strListenerKey) == mapListeners_.end()) {
        DHLOGE("%s: Find listener failed.", LOG_TAG);
        return nullListener_;
    }
    return mapListeners_[strListenerKey];
}

std::shared_ptr<ISoftbusListener> &SoftbusAdapter::GetSoftbusListenerById(int32_t sessionId)
{
    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapSessListeners_.find(sessionId) == mapSessListeners_.end()) {
        DHLOGE("%s: Find listener failed.", LOG_TAG);
        return nullListener_;
    }
    return mapSessListeners_[sessionId];
}

void SoftbusAdapter::SendAudioData()
{
    while (isAudioDataReady_) {
        DHLOGI("%s: SendAudioData enter.", LOG_TAG);
        std::shared_ptr<SoftbusStreamData> streamData;
        {
            std::unique_lock<std::mutex> lock(dataQueueMtx_);
            sendDataCond_.wait_for(lock, std::chrono::milliseconds(DATA_WAIT_TIME),
                [this]() { return !audioDataQueue_.empty(); });
            if (audioDataQueue_.empty()) {
                continue;
            }
            streamData = audioDataQueue_.front();
            DHLOGI("%s: streamData pop.", LOG_TAG);
            audioDataQueue_.pop();
        }

        if (streamData->data == nullptr) {
            DHLOGE("%s: Audio data is null.", LOG_TAG);
            continue;
        }

        StreamData data = { (char *)(streamData->data->Data()), streamData->data->Capacity() };
        StreamData ext = { 0 };
        StreamFrameInfo frameInfo = { 0 };

        DHLOGI("%s: SendAudioData. sessionId: %d.", LOG_TAG, streamData->sessionId);
        int32_t ret = SendStream(streamData->sessionId, &data, &ext, &frameInfo);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Send data failed. ret: %d.", LOG_TAG, ret);
        } else {
            DHLOGI("%s: SendAudioData successs.", LOG_TAG);
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS