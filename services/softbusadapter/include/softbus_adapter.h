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

#ifndef OHOS_SOFTBUS_ADAPTER_H
#define OHOS_SOFTBUS_ADAPTER_H

#include <condition_variable>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <queue>
#include <set>
#include <thread>

#include "single_instance.h"

#include "audio_data.h"
#include "daudio_constants.h"
#include "daudio_log.h"
#include "daudio_errorcode.h"
#include "daudio_util.h"
#include "isoftbus_listener.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct {
    std::shared_ptr<AudioData> data;
    int32_t sessionId;
} SoftbusStreamData;
class SoftbusAdapter {
public:
    DECLARE_SINGLE_INSTANCE_BASE(SoftbusAdapter);
public:
    int32_t CreateSoftbusSessionServer(const std::string &pkgName, const std::string &sessionName,
        const std::string &peerDevId);
    int32_t RemoveSoftbusSessionServer(const std::string &pkgname, const std::string &sessionName,
        const std::string &peerDevId);
    int32_t OpenSoftbusSession(const std::string &mySessionName, const std::string &peerSessionName,
        const std::string &peerDevId);
    int32_t CloseSoftbusSession(int32_t sessionId);
    int32_t SendSoftbusBytes(int32_t sessionId, const void *data, int32_t dataLen);
    int32_t SendSoftbusStream(int32_t sessionId, const std::shared_ptr<AudioData> &audioData);
    int32_t RegisterSoftbusListener(const std::shared_ptr<ISoftbusListener> &listener, const std::string &sessionName,
        const std::string &peerDevId);
    int32_t UnRegisterSoftbusListener(const std::string &sessionName, const std::string &peerDevId);

    int32_t OnSoftbusSessionOpened(int32_t sessionId, int32_t result);
    void OnSoftbusSessionClosed(int32_t sessionId);
    void OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen);
    void OnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
        const StreamFrameInfo *streamFrameInfo);
    void OnMessageReceived(int sessionId, const void *data, unsigned int dataLen);
    void OnQosEvent(int sessionId, int eventId, int tvCount, const QosTv *tvList);

private:
    SoftbusAdapter();
    ~SoftbusAdapter();
    std::shared_ptr<ISoftbusListener> &GetSoftbusListenerByName(int32_t sessionId);
    std::shared_ptr<ISoftbusListener> &GetSoftbusListenerById(int32_t sessionId);
    void SendAudioData();

private:
    static const constexpr char *LOG_TAG = "AudioSoftbusAdapter";
    static constexpr uint8_t DATA_WAIT_TIME = 20;
    static constexpr size_t DATA_QUEUE_MAX_SIZE = 10;

    std::mutex listenerMtx_;
    std::mutex sessSetMtx_;
    std::mutex dataQueueMtx_;
    std::condition_variable sendDataCond_;
    std::thread sendDataThread_;

    bool isAudioDataReady_ = false;
    ISessionListener sessListener_;
    std::shared_ptr<ISoftbusListener> nullListener_;
    std::unordered_map<std::string, std::set<std::string>> mapSessionSet_;
    std::unordered_map<std::string, std::shared_ptr<ISoftbusListener>> mapListeners_;
    std::unordered_map<int32_t, std::shared_ptr<ISoftbusListener>> mapSessListeners_;
    std::queue<std::shared_ptr<SoftbusStreamData>> audioDataQueue_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_SOFTBUS_ADAPTER_H
