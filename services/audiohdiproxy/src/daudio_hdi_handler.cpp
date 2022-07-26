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

#include "daudio_manager_callback.h"

#include <string>
#include <hdf_base.h>

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hdf_operate.h"
#include "daudio_hdi_handler.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DAudioHdiHandler);

DAudioHdiHandler::DAudioHdiHandler()
{
    DHLOGI("%s: DAudioHdiHandler construct", LOG_TAG);
}

DAudioHdiHandler::~DAudioHdiHandler()
{
    DHLOGI("%s: ~DAudioHdiHandler", LOG_TAG);
}

int32_t DAudioHdiHandler::InitHdiHandler()
{
    DHLOGI("%s: Init hdi handler.", LOG_TAG);
    if (audioSrvHdf_ != nullptr) {
        return DH_SUCCESS;
    }

    DAUDIO_SYNC_TRACE(DAUDIO_LOAD_HDF_DRIVER);
    DHLOGI("%s: Load hdf driver start.", LOG_TAG);
    int32_t ret = DaudioHdfOperate::GetInstance().LoadDaudioHDFImpl();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Load hdf driver failed, ret: %d", LOG_TAG, ret);
        return ret;
    }
    DHLOGI("%s: Load hdf driver end.", LOG_TAG);

    audioSrvHdf_ = IDAudioManager::Get(HDF_AUDIO_SERVICE_NAME.c_str(), false);
    if (audioSrvHdf_ == nullptr) {
        DHLOGE("%s: Can not get hdf audio manager.", LOG_TAG);
        return ERR_DH_AUDIO_HDI_PROXY_NOT_INIT;
    }

    DHLOGI("%s: Init hdi handler success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioHdiHandler::UninitHdiHandler()
{
    DHLOGI("%s: Unload hdf driver start.", LOG_TAG);
    int32_t ret = DaudioHdfOperate::GetInstance().UnLoadDaudioHDFImpl();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Unload hdf driver failed, ret: %d", LOG_TAG, ret);
        return ret;
    }
    DHLOGI("%s: Uninit hdi handler success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioHdiHandler::RegisterAudioDevice(const std::string &devId, const int32_t dhId,
    const std::string &capability, const std::shared_ptr<IDAudioHdiCallback> &callbackObjParam)
{
    DHLOGI("%s: RegisterAudioDevice, adpname: %s, dhId: %d", LOG_TAG, GetAnonyString(devId).c_str(), dhId);
    if (audioSrvHdf_ == nullptr) {
        DHLOGE("%s: Audio hdi proxy not init.", LOG_TAG);
        return ERR_DH_AUDIO_HDI_PROXY_NOT_INIT;
    }
    std::string searchKey;
    switch (GetDevTypeByDHId(dhId)) {
        case AUDIO_DEVICE_TYPE_SPEAKER:
            searchKey = devId + "Speaker";
            break;
        case AUDIO_DEVICE_TYPE_MIC:
            searchKey = devId + "Mic";
            break;
        case AUDIO_DEVICE_TYPE_UNKNOWN:
        default:
            DHLOGE("%s: Unknown audio device.", LOG_TAG);
            return ERR_DH_AUDIO_HDI_UNKOWN_DEVTYPE;
    }
    {
        std::lock_guard<std::mutex> devLck(devMapMtx_);
        auto call = mapAudioMgrCallback_.find(searchKey);
        if (call == mapAudioMgrCallback_.end()) {
            const sptr<DAudioManagerCallback> callbackptr(new DAudioManagerCallback(callbackObjParam));
            mapAudioMgrCallback_.emplace(searchKey, callbackptr);
        }
        auto dhIds = mapAudioMgrDhIds_.find(devId);
        if (dhIds != mapAudioMgrDhIds_.end()) {
            dhIds->second.insert(dhId);
        } else {
            std::set<int32_t> newDhIds;
            newDhIds.insert(dhId);
            mapAudioMgrDhIds_.emplace(devId, newDhIds);
        }
    }

    auto iter = mapAudioMgrCallback_.find(searchKey);
    if (iter == mapAudioMgrCallback_.end()) {
        DHLOGE("%s: Can not find callback. devId: %s", LOG_TAG, GetAnonyString(devId).c_str());
        return ERR_DH_AUDIO_HDI_CALLBACK_NOT_EXIST;
    }
    int32_t res = audioSrvHdf_->RegisterAudioDevice(devId, dhId, capability, iter->second);
    if (res != HDF_SUCCESS) {
        DHLOGE("%s: Call hdf proxy register failed, res: %d", LOG_TAG, res);
        return ERR_DH_AUDIO_HDI_CALL_FAILED;
    }
    return DH_SUCCESS;
}

int32_t DAudioHdiHandler::UnRegisterAudioDevice(const std::string &devId, const int32_t dhId)
{
    DHLOGI("UnRegisterAudioDevice, adpname: %s, dhId: %d", GetAnonyString(devId).c_str(), dhId);
    if (audioSrvHdf_ == nullptr) {
        DHLOGE("%s: Audio hdi proxy not init", LOG_TAG);
        return ERR_DH_AUDIO_HDI_PROXY_NOT_INIT;
    }

    int32_t res = audioSrvHdf_->UnRegisterAudioDevice(devId, dhId);
    if (res != HDF_SUCCESS) {
        DHLOGE("%s: Call hdf proxy unregister failed, res: %d", LOG_TAG, res);
        return ERR_DH_AUDIO_HDI_CALL_FAILED;
    }

    {
        std::lock_guard<std::mutex> devLck(devMapMtx_);
        auto iter = mapAudioMgrDhIds_.find(devId);
        if (iter == mapAudioMgrDhIds_.end()) {
            DHLOGE("%s: Can not find register devId. devId: %s", LOG_TAG, GetAnonyString(devId).c_str());
            return ERR_DH_AUDIO_HDI_CALLBACK_NOT_EXIST;
        }

        iter->second.erase(dhId);
        if (iter->second.empty()) {
            mapAudioMgrDhIds_.erase(devId);
        }
    }
    return DH_SUCCESS;
}

int32_t DAudioHdiHandler::NotifyEvent(const std::string &devId, const int32_t dhId,
    const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyEvent adpname: %s, dhId: %d, event type: %d, event content: %s.", LOG_TAG,
        GetAnonyString(devId).c_str(), dhId, audioEvent->type, audioEvent->content.c_str());
    if (audioSrvHdf_ == nullptr) {
        DHLOGE("Audio hdi proxy not init");
        return ERR_DH_AUDIO_HDI_PROXY_NOT_INIT;
    }
    OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioEvent newEvent = {AUDIO_EVENT_UNKNOWN, audioEvent->content};
    switch (audioEvent->type) {
        case AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT:
            newEvent.type = AUDIO_EVENT_OPEN_SPK_RESULT;
            break;
        case AudioEventType::NOTIFY_CLOSE_SPEAKER_RESULT:
            newEvent.type = AUDIO_EVENT_CLOSE_SPK_RESULT;
            break;
        case AudioEventType::NOTIFY_OPEN_MIC_RESULT:
            newEvent.type = AUDIO_EVENT_OPEN_MIC_RESULT;
            break;
        case AudioEventType::NOTIFY_CLOSE_MIC_RESULT:
            newEvent.type = AUDIO_EVENT_CLOSE_MIC_RESULT;
            break;
        case AudioEventType::VOLUME_CHANGE:
            newEvent.type = AUDIO_EVENT_VOLUME_CHANGE;
            break;
        case AudioEventType::SPEAKER_CLOSED:
            newEvent.type = AUDIO_EVENT_SPK_CLOSED;
            break;
        case AudioEventType::MIC_CLOSED:
            newEvent.type = AUDIO_EVENT_MIC_CLOSED;
            break;
        default:
            DHLOGE("%s: Unsupport audio event.", LOG_TAG);
            break;
    }

    int32_t res = audioSrvHdf_->NotifyEvent(devId, dhId, newEvent);
    if (res != HDF_SUCCESS) {
        DHLOGE("%s: Call hdf proxy NotifyEvent failed, res: %d", LOG_TAG, res);
        return ERR_DH_AUDIO_HDI_CALL_FAILED;
    }
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
