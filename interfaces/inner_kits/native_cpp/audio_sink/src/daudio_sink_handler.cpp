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
#include "daudio_log.h"
#include "daudio_sink_load_callback.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DAudioSinkHandler);

DAudioSinkHandler::DAudioSinkHandler()
{
    DHLOGI("%s: DAudio sink handler constructed.", LOG_TAG);
}

DAudioSinkHandler::~DAudioSinkHandler()
{
    DHLOGI("%s: DAudio sink handler destructed.", LOG_TAG);
}

int32_t DAudioSinkHandler::InitSink(const std::string &params)
{
    DHLOGI("%s: InitSink.", LOG_TAG);
    if (dAudioSinkProxy_ == nullptr) {
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            DHLOGE("%s: Failed to get system ability mgr.", LOG_TAG);
            return ERR_DH_AUDIO_SA_GET_SAMGR_FAILED;
        }
        sptr<DAudioSinkLoadCallback> loadCallback = new DAudioSinkLoadCallback(params);
        int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, loadCallback);
        if (ret != ERR_OK) {
            DHLOGE("%s: Failed to Load systemAbility, systemAbilityId:%d, ret code:%d.",
                LOG_TAG, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, ret);
            return ERR_DH_AUDIO_SA_LOAD_FAILED;
        }
    }

    std::unique_lock<std::mutex> lock(sinkProxyMutex_);
    auto waitStatus = sinkProxyConVar_.wait_for(lock, std::chrono::milliseconds(AUDIO_LOADSA_TIMEOUT_MS),
        [this]() { return dAudioSinkProxy_ != nullptr; });
    if (!waitStatus) {
        DHLOGE("%s: Audio load sa timeout.", LOG_TAG);
        return ERR_DH_AUDIO_SA_LOAD_TIMEOUT;
    }
    return DH_SUCCESS;
}

int32_t DAudioSinkHandler::ReleaseSink()
{
    DHLOGI("%s: ReleaseSink", LOG_TAG);
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    if (dAudioSinkProxy_ == nullptr) {
        DHLOGE("%s: Daudio sink proxy not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }

    int32_t ret = dAudioSinkProxy_->ReleaseSink();
    dAudioSinkProxy_ = nullptr;
    return ret;
}

int32_t DAudioSinkHandler::SubscribeLocalHardware(const std::string &dhId, const std::string &param)
{
    DHLOGI("%s: SubscribeLocalHardware.", LOG_TAG);
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    if (dAudioSinkProxy_ == nullptr) {
        DHLOGE("%s: daudio sink proxy not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }
    int32_t ret = dAudioSinkProxy_->SubscribeLocalHardware(dhId, param);
    return ret;
}

int32_t DAudioSinkHandler::UnsubscribeLocalHardware(const std::string &dhId)
{
    DHLOGI("%s: UnsubscribeLocalHardware.", LOG_TAG);
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    if (dAudioSinkProxy_ == nullptr) {
        DHLOGE("%s: daudio sink proxy not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_PROXY_NOT_INIT;
    }
    int32_t ret = dAudioSinkProxy_->UnsubscribeLocalHardware(dhId);
    return ret;
}

void DAudioSinkHandler::OnRemoteSinkSvrDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("%s: OnRemoteSinkSvrDied.", LOG_TAG);
    sptr<IRemoteObject> remoteObject = remote.promote();
    if (remoteObject == nullptr) {
        DHLOGE("%s: OnRemoteDied remote promoted failed.", LOG_TAG);
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
    DHLOGI("%s: FinishStartSA.", LOG_TAG);
    std::lock_guard<std::mutex> lock(sinkProxyMutex_);
    remoteObject->AddDeathRecipient(sinkSvrRecipient_);
    dAudioSinkProxy_ = iface_cast<IDAudioSink>(remoteObject);
    if ((dAudioSinkProxy_ == nullptr) || (!dAudioSinkProxy_->AsObject())) {
        DHLOGE("%s: Failed to get daudio sink proxy.", LOG_TAG);
        return;
    }
    dAudioSinkProxy_->InitSink(param);
    sinkProxyConVar_.notify_one();
}

void DAudioSinkHandler::DAudioSinkSvrRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("%s: DAudioSinkSvrRecipient::OnRemoteDied.", LOG_TAG);
    DAudioSinkHandler::GetInstance().OnRemoteSinkSvrDied(remote);
}

IDistributedHardwareSink *GetSinkHardwareHandler()
{
    DHLOGD("GetSinkHardwareHandler.");
    return &DAudioSinkHandler::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS