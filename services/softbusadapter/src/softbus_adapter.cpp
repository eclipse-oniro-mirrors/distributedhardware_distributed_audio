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

#undef DH_LOG_TAG
#define DH_LOG_TAG "SoftbusAdapter"

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
    DHLOGI("CreateSessionServer sess: %s peerDevId: %s.", sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    if (pkgName != PKG_NAME) {
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    std::lock_guard<std::mutex> setLock(sessSetMtx_);
    if (mapSessionSet_.find(sessionName) == mapSessionSet_.end()) {
        int32_t ret = CreateSessionServer(pkgName.c_str(), sessionName.c_str(), &sessListener_);
        if (ret != DH_SUCCESS) {
            DHLOGE("CreateSessionServer failed.");
            return ret;
        }
    } else {
        DHLOGI("Session already create, sessionName: %s.", sessionName.c_str());
    }

    mapSessionSet_[sessionName].insert(peerDevId);
    DHLOGI("CreateSessionServer success.");
    return DH_SUCCESS;
}

int32_t SoftbusAdapter::RemoveSoftbusSessionServer(const std::string &pkgName, const std::string &sessionName,
    const std::string &peerDevId)
{
    DHLOGI("RemoveSessionServer sess: %s peerDevId: %s.", sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    std::lock_guard<std::mutex> setLock(sessSetMtx_);
    if (mapSessionSet_.find(sessionName) == mapSessionSet_.end()) {
        DHLOGE("SessionName not find.");
        return ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION;
    }
    if (mapSessionSet_[sessionName].find(peerDevId) == mapSessionSet_[sessionName].end()) {
        DHLOGE("PerrDevId not find.");
        return ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION;
    }

    if (mapSessionSet_[sessionName].size() == SINGLE_ITEM) {
        mapSessionSet_[sessionName].erase(peerDevId);
        mapSessionSet_.erase(sessionName);
    }
    DHLOGI("RemoveSessionServer success.");
    return DH_SUCCESS;
}

int32_t SoftbusAdapter::OpenSoftbusSession(const std::string &localSessionName, const std::string &peerSessionName,
    const std::string &peerDevId)
{
    DHLOGI("OpenSoftbusSession localsess: %s peersess: %s peerDevId: %s.", localSessionName.c_str(),
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
        DHLOGE("Data copy failed.");
        return ERR_DH_AUDIO_ADAPTER_PARA_ERROR;
    }
    attr.attr.streamAttr.streamType = streamType;
    int32_t sessionId = OpenSession(localSessionName.c_str(), peerSessionName.c_str(), peerDevId.c_str(), "0", &attr);
    if (sessionId < 0) {
        DHLOGE("OpenSession failed sessionId: %d.", sessionId);
        return ERR_DH_AUDIO_ADAPTER_OPEN_SESSION_FAIL;
    }

    DHLOGI("OpenSoftbusSession success sessionId: %d.", sessionId);
    return sessionId;
}

int32_t SoftbusAdapter::CloseSoftbusSession(int32_t sessionId)
{
    DHLOGI("CloseSoftbusSession, sessionId: %d.", sessionId);
    CloseSession(sessionId);

    std::lock_guard<std::mutex> LisLock(listenerMtx_);
    mapSessListeners_.erase(sessionId);
    StopSendDataThread();
    DHLOGI("CloseSoftbusSession success.");
    return DH_SUCCESS;
}

int32_t SoftbusAdapter::SendSoftbusBytes(int32_t sessionId, const void *data, int32_t dataLen)
{
    DHLOGI("SendAudioEvent, sessionId: %d.", sessionId);
    int32_t ret = SendBytes(sessionId, data, dataLen);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendBytes failed ret:%d.", ret);
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    return DH_SUCCESS;
}

int32_t SoftbusAdapter::SendSoftbusStream(int32_t sessionId, const std::shared_ptr<AudioData> &audioData)
{
    DHLOGI("SendAudioData, sessionId: %d.", sessionId);
    if (!audioData) {
        DHLOGE("Audio data is null.");
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    constexpr size_t DATA_QUEUE_MAX_SIZE = 10;
    std::lock_guard<std::mutex> lck(dataQueueMtx_);
    while (audioDataQueue_.size() >= DATA_QUEUE_MAX_SIZE) {
        DHLOGE("Data queue overflow.");
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
    DHLOGI("RegisterListener sess: %s peerDevId: %s.", sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    if (listener == nullptr) {
        return ERR_DH_AUDIO_TRANS_ERROR;
    }
    std::string strListenerKey = sessionName + "_" + peerDevId;

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapListeners_.find(strListenerKey) != mapListeners_.end()) {
        DHLOGE("Session listener already register.");
    }
    mapListeners_.insert(std::make_pair(strListenerKey, listener));

    return DH_SUCCESS;
}

int32_t SoftbusAdapter::UnRegisterSoftbusListener(const std::string &sessionName, const std::string &peerDevId)
{
    DHLOGI("UnRegisterListener sess: %s peerDevId: %s.", sessionName.c_str(),
        GetAnonyString(peerDevId).c_str());
    std::string strListenerKey = sessionName + "_" + peerDevId;

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    mapListeners_.erase(strListenerKey);

    return DH_SUCCESS;
}

int32_t SoftbusAdapter::OnSoftbusSessionOpened(int32_t sessionId, int32_t result)
{
    DHLOGI("OnSessionOpened sessionId: %d, result: %d.", sessionId, result);
    if (result != DH_SUCCESS) {
        DHLOGE("OnSessionOpened failed.");
        return ERR_DH_AUDIO_ADAPTER_OPEN_SESSION_FAIL;
    }

    std::shared_ptr<ISoftbusListener> &listener = GetSoftbusListenerByName(sessionId);
    if (!listener) {
        DHLOGE("Get softbus listener failed.");
        return ERR_DH_AUDIO_TRANS_ERROR;
    }

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapSessListeners_.empty()) {
        DHLOGI("Start softbus send thread.");
        isAudioDataReady_ = true;
        sendDataThread_ = std::thread(&SoftbusAdapter::SendAudioData, this);
    }
    mapSessListeners_.insert(std::make_pair(sessionId, listener));
    listener->OnSessionOpened(sessionId, result);
    return DH_SUCCESS;
}

void SoftbusAdapter::OnSoftbusSessionClosed(int32_t sessionId)
{
    DHLOGI("OnSessionClosed sessionId:%d.", sessionId);
    auto &listener = GetSoftbusListenerById(sessionId);
    if (!listener) {
        DHLOGE("Get softbus listener failed.");
        return;
    }
    listener->OnSessionClosed(sessionId);

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    mapSessListeners_.erase(sessionId);
    StopSendDataThread();
}

void SoftbusAdapter::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DHLOGI("OnBytesReceicved sessionId: %d.", sessionId);
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
    DHLOGI("OnStreamReceived, sessionId: %d.", sessionId);
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
    DHLOGI("OnMessageReceived, sessionId: %d.", sessionId);
}

void SoftbusAdapter::OnQosEvent(int sessionId, int eventId, int tvCount, const QosTv *tvList)
{
    DHLOGI("OnQosEvent, sessionId: %d.", sessionId);
}

std::shared_ptr<ISoftbusListener> &SoftbusAdapter::GetSoftbusListenerByName(int32_t sessionId)
{
    char sessionName[DAUDIO_MAX_SESSION_NAME_LEN] = "";
    char peerDevId[DAUDIO_MAX_DEVICE_ID_LEN] = "";
    int32_t ret = GetPeerSessionName(sessionId, sessionName, sizeof(sessionName));
    if (ret != DH_SUCCESS) {
        DHLOGE("GetPeerSessionName failed ret: %d.", ret);
        return nullListener_;
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != DH_SUCCESS) {
        DHLOGE("GetPeerDeviceId failed ret: %d.", ret);
        return nullListener_;
    }

    std::string sessionNameStr(sessionName);
    std::string peerDevIdStr(peerDevId);

    DHLOGI("GetSoftbusListenerByName sessionName: %s, peerDevId: %s.", sessionNameStr.c_str(),
        GetAnonyString(peerDevIdStr).c_str());
    std::string strListenerKey = sessionNameStr + "_" + peerDevIdStr;

    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapListeners_.find(strListenerKey) == mapListeners_.end()) {
        DHLOGE("Find listener failed.");
        return nullListener_;
    }
    return mapListeners_[strListenerKey];
}

std::shared_ptr<ISoftbusListener> &SoftbusAdapter::GetSoftbusListenerById(int32_t sessionId)
{
    std::lock_guard<std::mutex> lisLock(listenerMtx_);
    if (mapSessListeners_.find(sessionId) == mapSessListeners_.end()) {
        DHLOGE("Find listener failed.");
        return nullListener_;
    }
    return mapSessListeners_[sessionId];
}

void SoftbusAdapter::SendAudioData()
{
    constexpr uint8_t DATA_WAIT_TIME = 20;
    while (isAudioDataReady_) {
        std::shared_ptr<SoftbusStreamData> streamData;
        {
            std::unique_lock<std::mutex> lock(dataQueueMtx_);
            sendDataCond_.wait_for(lock, std::chrono::milliseconds(DATA_WAIT_TIME),
                [this]() { return !audioDataQueue_.empty(); });
            if (audioDataQueue_.empty()) {
                continue;
            }
            streamData = audioDataQueue_.front();
            DHLOGI("streamData pop.");
            audioDataQueue_.pop();
        }

        if (streamData->data == nullptr) {
            DHLOGE("Audio data is null.");
            continue;
        }

        StreamData data = { (char *)(streamData->data->Data()), streamData->data->Capacity() };
        StreamData ext = { 0 };
        StreamFrameInfo frameInfo = { 0 };

        DHLOGI("SendAudioData. sessionId: %d.", streamData->sessionId);
        int32_t ret = SendStream(streamData->sessionId, &data, &ext, &frameInfo);
        if (ret != DH_SUCCESS) {
            DHLOGE("Send data failed. ret: %d.", ret);
        } else {
            DHLOGI("SendAudioData successs.");
        }
    }
}

void SoftbusAdapter::StopSendDataThread()
{
    if (mapSessListeners_.empty()) {
        DHLOGI("Stop softbus send thread.");
        isAudioDataReady_ = false;
        if (sendDataThread_.joinable()) {
            sendDataThread_.join();
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS