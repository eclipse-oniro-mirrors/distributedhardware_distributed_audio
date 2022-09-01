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

#include "daudio_sink_handler.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "daudio_sink_load_callback.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSinkHandler"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DAudioSinkHandler);

DAudioSinkHandler::DAudioSinkHandler()
{
    DHLOGI("DAudio sink handler constructed.");
}

DAudioSinkHandler::~DAudioSinkHandler()
{
    DHLOGI("DAudio sink handler destructed.");
}

int32_t DAudioSinkHandler::InitSink(const std::string &params)
{
    DHLOGI("Init sink handler.");
    DAUDIO_SYNC_TRACE(DAUDIO_SOURCE_LOAD_SYSTEM_ABILITY);
    if (dAudioSinkProxy_ == nullptr) {
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            DHLOGE("Failed to get system ability mgr.");
            DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID,
                ERR_DH_AUDIO_SA_GET_SAMGR_FAILED, "daudio sink get samgr failed.");
            return ERR_DH_AUDIO_SA_GET_SAMGR_FAILED;
        }
        sptr<DAudioSinkLoadCallback> loadCallback = new DAudioSinkLoadCallback(params);
        int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, loadCallback);
        if (ret != ERR_OK) {
            DHLOGE("Failed to Load systemAbility, systemAbilityId:%d, ret code:%d.",
                DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, ret);
            DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID,
                ERR_DH_AUDIO_SA_LOAD_FAILED, "daudio sink LoadSystemAbility call failed.");
            return ERR_DH_AUDIO_SA_LOAD_FAILED;
        }
    }

    std::unique_lock<std::mutex> lock(sinkProxyMutex_);
    auto waitStatus = sinkProxyConVar_.wait_for(lock, std::chrono::milliseconds(AUDIO_LOADSA_TIMEOUT_MS),
        [this]() { return dAudioSinkProxy_ != nullptr; });
    if (!waitStatus) {
        DHLOGE("Audio load sa timeout.");
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID,
            ERR_DH_AUDIO_SA_LOAD_TIMEOUT, "daudio sink sa load timeout.");
        return ERR_DH_AUDIO_SA_LOAD_TIMEOUT;
    }
    return DH_SUCCESS;
}

int32_t DAudioSinkHandler::ReleaseSink()
{
    DHLOGI("Release sink handler");
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    if (dAudioSinkProxy_ == nullptr) {
        DHLOGE("Daudio sink proxy not init.");
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID,
            ERR_DH_AUDIO_SA_PROXY_NOT_INIT, "daudio sink proxy not init.");
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }

    int32_t ret = dAudioSinkProxy_->ReleaseSink();
    dAudioSinkProxy_ = nullptr;
    return ret;
}

int32_t DAudioSinkHandler::SubscribeLocalHardware(const std::string &dhId, const std::string &param)
{
    DHLOGI("Subscribe to local hardware.");
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    if (dAudioSinkProxy_ == nullptr) {
        DHLOGE("daudio sink proxy not init.");
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }
    int32_t ret = dAudioSinkProxy_->SubscribeLocalHardware(dhId, param);
    return ret;
}

int32_t DAudioSinkHandler::UnsubscribeLocalHardware(const std::string &dhId)
{
    DHLOGI("Unsubscribe from local hardware.");
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    if (dAudioSinkProxy_ == nullptr) {
        DHLOGE("daudio sink proxy not init.");
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }
    int32_t ret = dAudioSinkProxy_->UnsubscribeLocalHardware(dhId);
    return ret;
}

void DAudioSinkHandler::OnRemoteSinkSvrDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("The remote sink server died.");
    sptr<IRemoteObject> remoteObject = remote.promote();
    if (remoteObject == nullptr) {
        DHLOGE("OnRemoteDied remote promoted failed.");
        return;
    }

    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    if (dAudioSinkProxy_ != nullptr) {
        dAudioSinkProxy_->AsObject()->RemoveDeathRecipient(sinkSvrRecipient_);
        dAudioSinkProxy_ = nullptr;
    }
}

void DAudioSinkHandler::FinishStartSA(const std::string &param, const sptr<IRemoteObject> &remoteObject)
{
    DHLOGI("Finish start SA.");
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    remoteObject->AddDeathRecipient(sinkSvrRecipient_);
    dAudioSinkProxy_ = iface_cast<IDAudioSink>(remoteObject);
    if ((dAudioSinkProxy_ == nullptr) || (!dAudioSinkProxy_->AsObject())) {
        DHLOGE("Failed to get daudio sink proxy.");
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_INIT_FAIL, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID,
            ERR_DH_AUDIO_SA_PROXY_NOT_INIT, "daudio sink get proxy failed.");
        return;
    }
    dAudioSinkProxy_->InitSink(param);
    sinkProxyConVar_.notify_one();
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_INIT, "daudio sink sa load success.");
}

void DAudioSinkHandler::DAudioSinkSvrRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("On remote died.");
    DAudioSinkHandler::GetInstance().OnRemoteSinkSvrDied(remote);
}

IDistributedHardwareSink *GetSinkHardwareHandler()
{
    DHLOGD("Get sink hardware handler.");
    return &DAudioSinkHandler::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS