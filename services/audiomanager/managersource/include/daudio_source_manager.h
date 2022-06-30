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

#ifndef OHOS_DAUDIO_SOURCE_MANAGER_H
#define OHOS_DAUDIO_SOURCE_MANAGER_H

#include <map>
#include <mutex>
#include <thread>

#include "daudio_hdi_handler.h"
#include "daudio_source_dev.h"
#include "daudio_source_mgr_callback.h"
#include "idaudio_sink.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct {
    std::string devId;
    std::shared_ptr<DAudioSourceDev> dev;
    std::map<std::string, std::string> ports;
} AudioDevice;

class DAudioSourceManager {
    DECLARE_SINGLE_INSTANCE_BASE(DAudioSourceManager);

public:
    int32_t Init(const sptr<IDAudioIpcCallback> &callback);
    int32_t UnInit();
    int32_t EnableDAudio(const std::string &devId, const std::string &dhId, const std::string &version,
        const std::string &attrs, const std::string &reqId);
    int32_t DisableDAudio(const std::string &devId, const std::string &dhId, const std::string &reqId);
    int32_t HandleDAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
        const std::string &eventContent);
    int32_t DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
        const std::string &eventContent);

    int32_t OnEnableDAudio(const std::string &devId, const std::string &dhId, const int32_t result);
    int32_t OnDisableDAudio(const std::string &devId, const std::string &dhId, const int32_t result);

private:
    class RemoteSinkSvrRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };
    DAudioSourceManager();
    ~DAudioSourceManager();
    void ClearAudioDev(const std::string &devId);

private:
    static const constexpr char *LOG_TAG = "DAudioSourceManager";
    std::mutex devMapMtx_;
    std::map<std::string, AudioDevice> audioDevMap_;
    std::mutex remoteSvrMutex_;
    sptr<RemoteSinkSvrRecipient> remoteSvrRecipient_ = nullptr;
    std::map<std::string, sptr<IDAudioSink>> remoteSvrProxyMap_;
    sptr<IDAudioIpcCallback> daudioIpcCallback_ = nullptr;
    std::shared_ptr<DAudioSourceMgrCallback> daudioMgrCallback_ = nullptr;
    std::thread devClearThread_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DAUDIO_SINK_MANAGER_H