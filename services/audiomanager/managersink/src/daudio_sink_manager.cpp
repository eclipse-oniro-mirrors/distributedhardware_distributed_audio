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

#include "daudio_sink_manager.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DAudioSinkManager);
DAudioSinkManager::DAudioSinkManager()
{
    DHLOGI("%s: Distributed audio sink manager constructed.", LOG_TAG);
}

DAudioSinkManager::~DAudioSinkManager()
{
    if (devClearThread_.joinable()) {
        devClearThread_.join();
    }
    DHLOGI("%s: Distributed audio sink manager deconstructed.", LOG_TAG);
}

int32_t DAudioSinkManager::Init()
{
    DHLOGI("%s: DAudioSinkManager Init.", LOG_TAG);
    remoteSourceSvrRecipient_ = new RemoteSourceSvrRecipient();
    if (remoteSourceSvrRecipient_ == nullptr) {
        DHLOGE("%s: RemoteiSourceSvrRecipient is nullptr.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    return DH_SUCCESS;
}

int32_t DAudioSinkManager::UnInit()
{
    DHLOGI("%s: DAudioSinkManager UnInit.", LOG_TAG);
    std::lock_guard<std::mutex> lock(remoteSvrMutex_);
    remoteSourceSvrRecipient_ = nullptr;
    remoteSvrProxyMap_.clear();
    {
        std::lock_guard<std::mutex> lock(devMapMutex_);
        for (auto iter = dAudioSinkDevMap_.begin(); iter != dAudioSinkDevMap_.end(); iter++) {
            if (iter->second != nullptr) {
                iter->second->SleepAudioDev();
            }
        }
        dAudioSinkDevMap_.clear();
    }
    if (devClearThread_.joinable()) {
        devClearThread_.join();
    }
    return DH_SUCCESS;
}

void DAudioSinkManager::OnSinkDevReleased(const std::string &devId)
{
    DHLOGI("%s: Release audio device devId: %s.", LOG_TAG, GetAnonyString(devId).c_str());
    if (devClearThread_.joinable()) {
        devClearThread_.join();
    }
    devClearThread_ = std::thread(&DAudioSinkManager::ClearAudioDev, this, devId);
}

int32_t DAudioSinkManager::HandleDAudioNotify(const std::string &devId, const std::string &dhId,
    const int32_t eventType, const std::string &eventContent)
{
    DHLOGI("%s: Recive audio event from devId: %s, event type: %d.", LOG_TAG, GetAnonyString(devId).c_str(), eventType);
    std::lock_guard<std::mutex> lock(devMapMutex_);
    auto iter = dAudioSinkDevMap_.find(devId);
    if (iter == dAudioSinkDevMap_.end()) {
        DHLOGI("%s: Create audio sink dev.", LOG_TAG);
        std::shared_ptr<DAudioSinkDev> sinkDev = std::make_shared<DAudioSinkDev>(devId);
        if (sinkDev->AwakeAudioDev() != DH_SUCCESS) {
            DHLOGE("%s: Awake audio dev failed.", LOG_TAG);
            return ERR_DH_AUDIO_FAILED;
        }
        dAudioSinkDevMap_.emplace(devId, sinkDev);
    }
    NotifyEvent(devId, eventType, eventContent);
    return DH_SUCCESS;
}

int32_t DAudioSinkManager::DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
    const std::string &eventContent)
{
    DHLOGI("%s: DAudioNotify, devId: %s, dhId: %s, eventType: %d.", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str(), eventType);
    std::string localNetworkId;
    int32_t ret = GetLocalDeviceNetworkId(localNetworkId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get local network id failed.", LOG_TAG);
        return ret;
    }
    {
        std::lock_guard<std::mutex> autoLock(remoteSvrMutex_);
        auto sinkProxy = remoteSvrProxyMap_.find(devId);
        if (sinkProxy != remoteSvrProxyMap_.end()) {
            if (sinkProxy->second != nullptr) {
                sinkProxy->second->DAudioNotify(localNetworkId, dhId, eventType, eventContent);
                return DH_SUCCESS;
            }
        }
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DHLOGE("%s: Failed to get system ability mgr.", LOG_TAG);
        return ERR_DH_AUDIO_SA_GET_SAMGR_FAILED;
    }
    auto remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID, devId);
    if (remoteObject == nullptr) {
        DHLOGE("%s: remoteObject is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_GET_REMOTE_SINK_FAILED;
    }
    sptr<IDAudioSource> remoteSvrProxy = iface_cast<IDAudioSource>(remoteObject);
    if (remoteSvrProxy == nullptr) {
        DHLOGE("%s: Failed to get remote daudio sink SA.", LOG_TAG);
        return ERR_DH_AUDIO_SA_GET_REMOTE_SINK_FAILED;
    }
    {
        std::lock_guard<std::mutex> autoLock(remoteSvrMutex_);
        remoteSvrProxyMap_[devId] = remoteSvrProxy;
        remoteSvrProxy->DAudioNotify(localNetworkId, dhId, eventType, eventContent);
    }
    return DH_SUCCESS;
}

void DAudioSinkManager::NotifyEvent(const std::string &devId, const int32_t eventType, const std::string &eventContent)
{
    auto audioEvent = std::make_shared<AudioEvent>();
    audioEvent->type = (AudioEventType)eventType;
    audioEvent->content = eventContent;
    dAudioSinkDevMap_[devId]->NotifyEvent(audioEvent);
}

void DAudioSinkManager::RemoteSourceSvrRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("%s: DAudioSinkManager::OnRemoteDied.", LOG_TAG);
}

void DAudioSinkManager::ClearAudioDev(const std::string &devId)
{
    std::lock_guard<std::mutex> lock(devMapMutex_);
    auto dev = dAudioSinkDevMap_.find(devId);
    if (dev == dAudioSinkDevMap_.end()) {
        DHLOGD("%s: Device not register.", LOG_TAG);
        return;
    }
    if (dev->second == nullptr) {
        DHLOGD("%s: Device already released.", LOG_TAG);
        return;
    }
    dev->second->SleepAudioDev();
    dAudioSinkDevMap_.erase(devId);
}
} // namespace DistributedHardware
} // namespace OHOS
