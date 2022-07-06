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
    DHLOGI("%s: Distributed audio sink manager destructed.", LOG_TAG);
}

int32_t DAudioSinkManager::Init()
{
    DHLOGI("%s: DAudioSinkManager Init.", LOG_TAG);
    if (remoteSourceSvrRecipient_ == nullptr) {
        remoteSourceSvrRecipient_ = new RemoteSourceSvrRecipient();
    }
    return DH_SUCCESS;
}

int32_t DAudioSinkManager::UnInit()
{
    DHLOGI("%s: DAudioSinkManager UnInit.", LOG_TAG);
    std::lock_guard<std::mutex> lock(remoteSourceSvrMutex_);
    if (remoteSourceSvrRecipient_ != nullptr) {
        remoteSourceSvrRecipient_ = nullptr;
    }

    for (auto iter = remoteSourceSvrProxyMap_.begin(); iter != remoteSourceSvrProxyMap_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->AsObject()->RemoveDeathRecipient(remoteSourceSvrRecipient_);
        }
    }
    remoteSourceSvrProxyMap_.clear();
    dAudioSinkDevMap_.clear();
    return DH_SUCCESS;
}

void DAudioSinkManager::OnSinkDevReleased(const std::string &devId)
{
    DHLOGI("%s: DAudioSinkManager OnSinkDevReleased. devId: %s.", LOG_TAG, GetAnonyString(devId).c_str());
    std::lock_guard<std::mutex> lock(devMapMutex_);
    dAudioSinkDevMap_.erase(devId);
}

int32_t DAudioSinkManager::HandleDAudioNotify(const std::string &devId, const std::string &dhId,
    const int32_t eventType, const std::string &eventContent)
{
    DHLOGI("%s: HandleDAudioNotify, devId: %s", LOG_TAG, GetAnonyString(devId).c_str());
    std::lock_guard<std::mutex> lock(devMapMutex_);
    auto iter = dAudioSinkDevMap_.find(devId);
    if (iter == dAudioSinkDevMap_.end()) {
        if (eventType == AudioEventType::OPEN_CTRL) {
            DHLOGI("%s: create DAudioSinkDev, devId: %s", LOG_TAG, GetAnonyString(devId).c_str());
            std::shared_ptr<DAudioSinkDev> sinkDev = std::make_shared<DAudioSinkDev>(devId);
            dAudioSinkDevMap_.emplace(devId, sinkDev);
        } else {
            DHLOGE("%s: device not exist, devId: %s", LOG_TAG, GetAnonyString(devId).c_str());
            return ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST;
        }
    }
    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    audioEvent->type = (AudioEventType)eventType;
    audioEvent->content = eventContent;
    DHLOGI("%s: call sinkDev Notify Event, eventType: %d, content: %s", LOG_TAG, eventType, eventContent.c_str());
    dAudioSinkDevMap_[devId]->NotifyEvent(audioEvent);
    return DH_SUCCESS;
}

int32_t DAudioSinkManager::DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
    const std::string &eventContent)
{
    DHLOGI("%s: DAudioNotify, devId: %s", LOG_TAG, GetAnonyString(devId).c_str());
    if (devId.empty()) {
        DHLOGE("%s: DAudioNotify remote devId is empty", LOG_TAG);
        return ERR_DH_AUDIO_SA_INVALID_NETWORKID;
    }
    sptr<IDAudioSource> remoteSourceSvrProxy = nullptr;

    {
        std::lock_guard<std::mutex> autoLock(remoteSourceSvrMutex_);
        auto iter = remoteSourceSvrProxyMap_.find(devId);
        if (iter != remoteSourceSvrProxyMap_.end()) {
            if (iter->second != nullptr) {
                remoteSourceSvrProxy = iter->second;
            }
        }
    }

    if (remoteSourceSvrProxy == nullptr) {
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            DHLOGE("%s: Failed to get system ability mgr.", LOG_TAG);
            return ERR_DH_AUDIO_SA_GET_SAMGR_FAILED;
        }
        auto remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID, devId);
        if (remoteObject == nullptr) {
            DHLOGE("%s: remoteObject is null", LOG_TAG);
            return ERR_DH_AUDIO_SA_GET_REMOTE_SOURCE_FAILED;
        }

        remoteSourceSvrProxy = iface_cast<IDAudioSource>(remoteObject);
        if (remoteSourceSvrProxy == nullptr) {
            DHLOGE("%s: Failed to get remote daudio source sa.", LOG_TAG);
            return ERR_DH_AUDIO_SA_GET_REMOTE_SOURCE_FAILED;
        }
        {
            std::lock_guard<std::mutex> autoLock(remoteSourceSvrMutex_);
            remoteSourceSvrProxyMap_[devId] = remoteSourceSvrProxy;
        }
    }
    std::string localNetworkId;
    int ret = GetLocalDeviceNetworkId(localNetworkId);
    if (ret != DH_SUCCESS) {
        return ret;
    }
    DHLOGI("%s: DAudioNotify finish, eventType: %d, eventContent: %s.", LOG_TAG, eventType, eventContent.c_str());
    remoteSourceSvrProxyMap_[devId]->DAudioNotify(localNetworkId, dhId, eventType, eventContent);
    return DH_SUCCESS;
}

void DAudioSinkManager::RemoteSourceSvrRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("%s: DAudioSinkManager::OnRemoteDied.", LOG_TAG);
}
} // namespace DistributedHardware
} // namespace OHOS
