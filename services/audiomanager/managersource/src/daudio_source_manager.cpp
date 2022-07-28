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

#include "daudio_source_manager.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DAudioSourceManager);
DAudioSourceManager::DAudioSourceManager()
{
    DHLOGI("%s: Distributed audio source manager constructed.", LOG_TAG);
}

DAudioSourceManager::~DAudioSourceManager()
{
    DHLOGI("%s: Distributed audio source manager destructed.", LOG_TAG);
    if (devClearThread_.joinable()) {
        devClearThread_.join();
    }
}

int32_t DAudioSourceManager::Init(const sptr<IDAudioIpcCallback> &callback)
{
    DHLOGI("%s: Init.", LOG_TAG);
    if (callback == nullptr) {
        DHLOGE("%s: Callback is nullptr.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    int32_t ret = DAudioHdiHandler::GetInstance().InitHdiHandler();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Init Hdi handler failed.", LOG_TAG);
        return ret;
    }
    remoteSvrRecipient_ = new (std::nothrow) RemoteSinkSvrRecipient();
    if (remoteSvrRecipient_ == nullptr) {
        DHLOGE("%s: RemoteSvrRecipient is nullptr.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    daudioIpcCallback_ = callback;
    daudioMgrCallback_ = std::make_shared<DAudioSourceMgrCallback>();
    return DH_SUCCESS;
}

int32_t DAudioSourceManager::UnInit()
{
    DHLOGI("%s: UnInit.", LOG_TAG);
    std::lock_guard<std::mutex> lock(remoteSvrMutex_);
    remoteSvrProxyMap_.clear();
    remoteSvrRecipient_ = nullptr;
    daudioIpcCallback_ = nullptr;
    daudioMgrCallback_ = nullptr;
    {
        std::lock_guard<std::mutex> lock(devMapMtx_);
        for (auto iter = audioDevMap_.begin(); iter != audioDevMap_.end(); iter++) {
            iter->second.dev->SleepAudioDev();
        }
        audioDevMap_.clear();
    }
    if (devClearThread_.joinable()) {
        devClearThread_.join();
    }

    int32_t ret = DAudioHdiHandler::GetInstance().UninitHdiHandler();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Uninit Hdi handler failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceManager::EnableDAudio(const std::string &devId, const std::string &dhId,
    const std::string &version, const std::string &attrs, const std::string &reqId)
{
    DHLOGI("%s: EnableDAudio, devId: %s, dhId: %s, version: %s, reqId: %s.", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str(), version.c_str(), reqId.c_str());
    std::lock_guard<std::mutex> lock(devMapMtx_);
    auto dev = audioDevMap_.find(devId);
    if (dev == audioDevMap_.end()) {
        DHLOGI("%s: Create new audio device.", LOG_TAG);
        std::shared_ptr<DAudioSourceDev> sourceDev = std::make_shared<DAudioSourceDev>(devId, daudioMgrCallback_);
        if (sourceDev == nullptr || sourceDev->AwakeAudioDev() != DH_SUCCESS) {
            DHLOGE("%s: Create new audio device failed.", LOG_TAG);
            return ERR_DH_AUDIO_NULLPTR;
        }
        AudioDevice device = {devId, sourceDev};
        audioDevMap_[devId] = device;
    }
    audioDevMap_[devId].ports[dhId] = reqId;
    return audioDevMap_[devId].dev->EnableDAudio(dhId, attrs);
}

int32_t DAudioSourceManager::DisableDAudio(const std::string &devId, const std::string &dhId, const std::string &reqId)
{
    DHLOGI("%s: DisableDAudio, devId: %s, dhId: %s, reqId: %s.", LOG_TAG, GetAnonyString(devId).c_str(), dhId.c_str(),
        reqId.c_str());
    std::lock_guard<std::mutex> lock(devMapMtx_);
    auto dev = audioDevMap_.find(devId);
    if (dev == audioDevMap_.end()) {
        DHLOGE("%s: Audio device not exist.", LOG_TAG);
        return ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST;
    }
    if (audioDevMap_[devId].dev == nullptr) {
        DHLOGE("%s, Audio device is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    audioDevMap_[devId].ports[dhId] = reqId;
    return audioDevMap_[devId].dev->DisableDAudio(dhId);
}

int32_t DAudioSourceManager::HandleDAudioNotify(const std::string &devId, const std::string &dhId,
    const int32_t eventType, const std::string &eventContent)
{
    DHLOGI("%s: HandleDAudioNotify, devId: %s, dhId: %s, eventType: %d.", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str(), eventType);
    std::lock_guard<std::mutex> lock(devMapMtx_);
    auto dev = audioDevMap_.find(devId);
    if (dev == audioDevMap_.end()) {
        DHLOGE("%s: Audio device not exist.", LOG_TAG);
        return ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST;
    }
    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    audioEvent->type = (AudioEventType)eventType;
    audioEvent->content = eventContent;
    DHLOGI("%s: HandleDAudioNotify call sourceDev eventType: %d.", LOG_TAG, eventType);
    audioDevMap_[devId].dev->NotifyEvent(audioEvent);
    return DH_SUCCESS;
}

int32_t DAudioSourceManager::DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
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
    auto remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, devId);
    if (remoteObject == nullptr) {
        DHLOGE("%s: remoteObject is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_GET_REMOTE_SINK_FAILED;
    }
    sptr<IDAudioSink> remoteSvrProxy = iface_cast<IDAudioSink>(remoteObject);
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

void DAudioSourceManager::RemoteSinkSvrRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("DAudioSourceManager::OnRemoteDied.");
}

int32_t DAudioSourceManager::OnEnableDAudio(const std::string &devId, const std::string &dhId, const int32_t result)
{
    DHLOGI("%s: OnEnableDAudio device: %s, dhId: %s, ret: %d.", LOG_TAG, GetAnonyString(devId).c_str(), dhId.c_str(),
        result);
    std::lock_guard<std::mutex> lock(devMapMtx_);
    auto dev = audioDevMap_.find(devId);
    if (dev == audioDevMap_.end()) {
        DHLOGE("%s: Audio device not exist.", LOG_TAG);
        return ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST;
    }
    auto port = audioDevMap_[devId].ports.find(dhId);
    if (port == audioDevMap_[devId].ports.end()) {
        DHLOGE("%s: Audio port not exist.", LOG_TAG);
        return ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST;
    }
    if (daudioIpcCallback_ == nullptr) {
        DHLOGE("%s: Audio Ipc callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    daudioIpcCallback_->OnNotifyRegResult(devId, dhId, port->second, result, "");
    if (result != DH_SUCCESS) {
        audioDevMap_[devId].ports.erase(dhId);
        if (!audioDevMap_[devId].ports.empty()) {
            return DH_SUCCESS;
        }
        if (devClearThread_.joinable()) {
            devClearThread_.join();
        }
        devClearThread_ = std::thread(&DAudioSourceManager::ClearAudioDev, this, devId);
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceManager::OnDisableDAudio(const std::string &devId, const std::string &dhId, const int32_t result)
{
    DHLOGI("%s: Disable device: %s, dhId: %s, ret: %d.", LOG_TAG, GetAnonyString(devId).c_str(), dhId.c_str(), result);
    std::lock_guard<std::mutex> lock(devMapMtx_);
    auto dev = audioDevMap_.find(devId);
    if (dev == audioDevMap_.end()) {
        DHLOGE("%s: Audio device not exist.", LOG_TAG);
        return ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST;
    }
    auto port = audioDevMap_[devId].ports.find(dhId);
    if (port == audioDevMap_[devId].ports.end()) {
        DHLOGE("%s: Audio port not exist.", LOG_TAG);
        return ERR_DH_AUDIO_SA_DEVICE_NOT_EXIST;
    }
    if (daudioIpcCallback_ == nullptr) {
        DHLOGE("%s: Audio Ipc callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    daudioIpcCallback_->OnNotifyUnregResult(devId, dhId, port->second, result, "");

    if (result == DH_SUCCESS) {
        audioDevMap_[devId].ports.erase(dhId);
        if (!audioDevMap_[devId].ports.empty()) {
            return DH_SUCCESS;
        }
        if (devClearThread_.joinable()) {
            devClearThread_.join();
        }
        devClearThread_ = std::thread(&DAudioSourceManager::ClearAudioDev, this, devId);
    }
    return DH_SUCCESS;
}

void DAudioSourceManager::ClearAudioDev(const std::string &devId)
{
    std::lock_guard<std::mutex> lock(devMapMtx_);
    if (audioDevMap_[devId].ports.empty()) {
        audioDevMap_[devId].dev->SleepAudioDev();
        audioDevMap_.erase(devId);
    }
}
} // DistributedHardware
} // OHOS