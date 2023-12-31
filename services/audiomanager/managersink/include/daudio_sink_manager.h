/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DAUDIO_SINK_MANAGER_H
#define OHOS_DAUDIO_SINK_MANAGER_H

#include <map>
#include <mutex>

#include "single_instance.h"

#include "daudio_sink_dev.h"
#include "idaudio_source.h"

namespace OHOS {
namespace DistributedHardware {
class DAudioSinkManager {
DECLARE_SINGLE_INSTANCE_BASE(DAudioSinkManager);
public:
    int32_t Init();
    int32_t UnInit();
    int32_t HandleDAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
        const std::string &eventContent);
    int32_t DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
        const std::string &eventContent);
    void OnSinkDevReleased(const std::string &devId);

    void NotifyEvent(const std::string &devId, const int32_t eventType, const std::string &eventContent);

private:
    static constexpr const char* DEVCLEAR_THREAD = "sinkClearTh";

    DAudioSinkManager();
    ~DAudioSinkManager();
    void ClearAudioDev(const std::string &devId);
    int32_t CreateAudioDevice(const std::string &devId);
    std::mutex devMapMutex_;
    std::unordered_map<std::string, std::shared_ptr<DAudioSinkDev>> audioDevMap_;
    std::mutex remoteSvrMutex_;
    std::map<std::string, sptr<IDAudioSource>> sourceServiceMap_;
    std::thread devClearThread_;
    std::string localNetworkId_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DAUDIO_SINK_MANAGER_H
