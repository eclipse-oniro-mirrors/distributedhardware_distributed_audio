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

#include "daudio_source_handler.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "daudio_util.h"
#include "daudio_source_load_callback.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSourceHandler"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DAudioSourceHandler);
DAudioSourceHandler::DAudioSourceHandler()
{
    DHLOGI("Audio source handler constructed.");
    if (!sourceSvrRecipient_) {
        sourceSvrRecipient_ = new DAudioSourceSvrRecipient();
    }

    if (!dAudioIpcCallback_) {
        dAudioIpcCallback_ = new DAudioIpcCallback();
    }
}

DAudioSourceHandler::~DAudioSourceHandler()
{
    DHLOGI("Audio source handler destructed.");
}

int32_t DAudioSourceHandler::InitSource(const std::string &params)
{
    DHLOGI("Init source handler.");
    DAUDIO_SYNC_TRACE(DAUDIO_SOURCE_LOAD_SYSTEM_ABILITY);
    if (dAudioSourceProxy_ == nullptr) {
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            DHLOGE("Failed to get system ability mgr.");
            return ERR_DH_AUDIO_SA_GET_SAMGR_FAILED;
        }
        sptr<DAudioSourceLoadCallback> loadCallback = new DAudioSourceLoadCallback(params);
        int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID, loadCallback);
        if (ret != ERR_OK) {
            DHLOGE("Failed to Load systemAbility, ret code: %d", ret);
            DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL,
                DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID, ERR_DH_AUDIO_SA_LOAD_FAILED,
                "daudio source LoadSystemAbility call failed.");
            return ERR_DH_AUDIO_SA_LOAD_FAILED;
        }
    }

    std::unique_lock<std::mutex> lock(sourceProxyMutex_);
    auto waitStatus = sourceProxyConVar_.wait_for(lock, std::chrono::milliseconds(AUDIO_LOADSA_TIMEOUT_MS),
        [this]() { return dAudioSourceProxy_ != nullptr; });
    if (!waitStatus) {
        DHLOGE("Load audio SA timeout.");
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID,
            ERR_DH_AUDIO_SA_LOAD_TIMEOUT, "daudio source sa load timeout.");
        return ERR_DH_AUDIO_SA_LOAD_TIMEOUT;
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceHandler::ReleaseSource()
{
    DHLOGI("Release source handler.");
    std::lock_guard<std::mutex> lock(sourceProxyMutex_);
    if (dAudioSourceProxy_ == nullptr) {
        DHLOGE("Daudio source proxy not init.");
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID,
            ERR_DH_AUDIO_SA_PROXY_NOT_INIT, "daudio source proxy not init.");
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }

    int32_t ret = dAudioSourceProxy_->ReleaseSource();
    dAudioSourceProxy_ = nullptr;
    return ret;
}

int32_t DAudioSourceHandler::RegisterDistributedHardware(const std::string &devId, const std::string &dhId,
    const EnableParam &param, std::shared_ptr<RegisterCallback> callback)
{
    DHLOGI("Register distributed hardware, devId: %s, dhId: %s.", GetAnonyString(devId).c_str(), dhId.c_str());
    std::lock_guard<std::mutex> lock(sourceProxyMutex_);
    if (dAudioSourceProxy_ == nullptr) {
        DHLOGE("Daudio source proxy not init.");
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }
    if (dAudioIpcCallback_ == nullptr) {
        DHLOGE("Daudio ipc callback is null.");
        return ERR_DH_AUDIO_SA_IPCCALLBACK_NOT_INIT;
    }
    if (devId.length() > DAUDIO_MAX_DEVICE_ID_LEN || dhId.length() > DAUDIO_MAX_DEVICE_ID_LEN) {
        return ERR_DH_AUDIO_SA_DEVID_ILLEGAL;
    }

    DaudioStartAsyncTrace(DAUDIO_REGISTER_AUDIO, DAUDIO_REGISTER_AUDIO_TASKID);
    std::string reqId = GetRandomID();
    dAudioIpcCallback_->PushRegisterCallback(reqId, callback);
    return dAudioSourceProxy_->RegisterDistributedHardware(devId, dhId, param, reqId);
}

int32_t DAudioSourceHandler::UnregisterDistributedHardware(const std::string &devId, const std::string &dhId,
    std::shared_ptr<UnregisterCallback> callback)
{
    DHLOGI("Unregister distributed hardware, devId: %s, dhId: %s.", GetAnonyString(devId).c_str(), dhId.c_str());
    std::lock_guard<std::mutex> lock(sourceProxyMutex_);
    if (dAudioSourceProxy_ == nullptr) {
        DHLOGE("Daudio source proxy not init.");
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }
    if (dAudioIpcCallback_ == nullptr) {
        DHLOGE("Daudio ipc callback is null.");
        return ERR_DH_AUDIO_SA_IPCCALLBACK_NOT_INIT;
    }
    if (devId.length() > DAUDIO_MAX_DEVICE_ID_LEN || dhId.length() > DAUDIO_MAX_DEVICE_ID_LEN) {
        return ERR_DH_AUDIO_SA_DEVID_ILLEGAL;
    }

    DaudioStartAsyncTrace(DAUDIO_UNREGISTER_AUDIO, DAUDIO_UNREGISTER_AUDIO_TASKID);
    std::string reqId = GetRandomID();
    dAudioIpcCallback_->PushUnregisterCallback(reqId, callback);
    return dAudioSourceProxy_->UnregisterDistributedHardware(devId, dhId, reqId);
}

int32_t DAudioSourceHandler::ConfigDistributedHardware(const std::string &devId, const std::string &dhId,
    const std::string &key, const std::string &value)
{
    DHLOGI("Config distributed hardware, devId: %s, dhId: %s.", GetAnonyString(devId).c_str(), dhId.c_str());
    std::lock_guard<std::mutex> lock(sourceProxyMutex_);
    if (dAudioSourceProxy_ == nullptr) {
        DHLOGE("Daudio source proxy not init.");
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }
    if (devId.length() > DAUDIO_MAX_DEVICE_ID_LEN || dhId.length() > DAUDIO_MAX_DEVICE_ID_LEN) {
        return ERR_DH_AUDIO_SA_DEVID_ILLEGAL;
    }
    return dAudioSourceProxy_->ConfigDistributedHardware(devId, dhId, key, value);
}

void DAudioSourceHandler::OnRemoteSourceSvrDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("The daudio source service died.");
    sptr<IRemoteObject> remoteObject = remote.promote();
    if (!remoteObject) {
        DHLOGE("OnRemoteDied remote promoted failed");
        return;
    }
    std::lock_guard<std::mutex> lock(sourceProxyMutex_);
    if (dAudioSourceProxy_ != nullptr) {
        dAudioSourceProxy_->AsObject()->RemoveDeathRecipient(sourceSvrRecipient_);
        dAudioSourceProxy_ = nullptr;
    }
}

void DAudioSourceHandler::FinishStartSA(const std::string &param, const sptr<IRemoteObject> &remoteObject)
{
    DHLOGI("Finish start SA.");
    std::lock_guard<std::mutex> lock(sourceProxyMutex_);
    remoteObject->AddDeathRecipient(sourceSvrRecipient_);
    dAudioSourceProxy_ = iface_cast<IDAudioSource>(remoteObject);
    if ((dAudioSourceProxy_ == nullptr) || (!dAudioSourceProxy_->AsObject())) {
        DHLOGE("Failed to get daudio source proxy.");
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID,
            ERR_DH_AUDIO_SA_PROXY_NOT_INIT, "daudio source get proxy failed.");
        return;
    }
    dAudioSourceProxy_->InitSource(param, dAudioIpcCallback_);
    sourceProxyConVar_.notify_one();
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_INIT, "daudio source sa load success.");
}

void DAudioSourceHandler::DAudioSourceSvrRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DAudioSourceHandler::GetInstance().OnRemoteSourceSvrDied(remote);
}

IDistributedHardwareSource *GetSourceHardwareHandler()
{
    DHLOGI("Get source hardware handler.");
    return &DAudioSourceHandler::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
