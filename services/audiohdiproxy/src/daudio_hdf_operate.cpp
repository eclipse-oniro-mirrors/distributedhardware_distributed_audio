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

#include "daudio_hdf_operate.h"

#include <hdf_io_service_if.h>

#include "daudio_errorcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DaudioHdfOperate);
static const constexpr char *LOG_TAG = "DaudioHdfOperate";
void DAudioHdfServStatListener::OnReceive(const ServiceStatus& status)
{
    DHLOGI("%s: Service status on receive.", LOG_TAG);
    if (status.serviceName == AUDIO_SERVICE_NAME || status.serviceName == AUDIOEXT_SERVICE_NAME) {
        callback_(status);
    }
}

int32_t DaudioHdfOperate::LoadDaudioHDFImpl()
{
    if (audioServStatus_ == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START &&
        audioextServStatus_ == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START) {
        DHLOGI("%s: Service has already start.", LOG_TAG);
        return DH_SUCCESS;
    }
    servMgr_ = IServiceManager::Get();
    devmgr_ = IDeviceManager::Get();
    if (servMgr_ == nullptr || devmgr_ == nullptr) {
        DHLOGE("%s: Get hdi service manager or device manager failed!", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }

    ::OHOS::sptr<IServStatListener> listener =
        new DAudioHdfServStatListener(DAudioHdfServStatListener::StatusCallback([&](const ServiceStatus& status) {
            DHLOGI("%s: LoadAudioService service status callback, serviceName: %s, status: %d", LOG_TAG,
                status.serviceName.c_str(), status.status);
            std::unique_lock<std::mutex> lock(hdfOperateMutex_);
            if (status.serviceName == AUDIO_SERVICE_NAME) {
                audioServStatus_ = status.status;
                hdfOperateCon_.notify_one();
            } else if (status.serviceName == AUDIOEXT_SERVICE_NAME) {
                audioextServStatus_ = status.status;
                hdfOperateCon_.notify_one();
            }
        }));
    if (servMgr_->RegisterServiceStatusListener(listener, DEVICE_CLASS_AUDIO) != 0) {
        DHLOGE("%s: RegisterServiceStatusListener failed!", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }

    if (devmgr_->LoadDevice(AUDIO_SERVICE_NAME) != 0) {
        DHLOGE("%s: Load audio service failed!", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }
    if (WaitLoadService(audioServStatus_, AUDIO_SERVICE_NAME) != DH_SUCCESS) {
        DHLOGE("%s: Wait load audio service failed!", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }

    if (devmgr_->LoadDevice(AUDIOEXT_SERVICE_NAME) != 0) {
        DHLOGE("%s: Load provider service failed!", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }
    if (WaitLoadService(audioextServStatus_, AUDIOEXT_SERVICE_NAME) != DH_SUCCESS) {
        DHLOGE("%s: Wait load provider service failed!", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }

    if (servMgr_->UnregisterServiceStatusListener(listener) != 0) {
        DHLOGE("%s: UnregisterServiceStatusListener failed!", LOG_TAG);
    }
    return DH_SUCCESS;
}

int32_t DaudioHdfOperate::WaitLoadService(const uint16_t& servStatus, const std::string& servName)
{
    std::unique_lock<std::mutex> lock(hdfOperateMutex_);
    hdfOperateCon_.wait_for(lock, std::chrono::milliseconds(WAIT_TIME), [servStatus] {
        return (servStatus == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    });

    if (servStatus != OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START) {
        DHLOGE("%s: Wait load service %s failed, status %d", LOG_TAG, servName.c_str(), servStatus);
        return ERR_DH_AUDIO_FAILED;
    }

    return DH_SUCCESS;
}

int32_t DaudioHdfOperate::UnLoadDaudioHDFImpl()
{
    DHLOGI("%s: UnLoadDaudioHDFImpl begin!", LOG_TAG);
    devmgr_ = IDeviceManager::Get();
    if (devmgr_ == nullptr) {
        DHLOGE("%s: Get hdi device manager failed!", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }

    int32_t ret = devmgr_->UnloadDevice(AUDIO_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("%s: Unload audio service failed, ret: %d", LOG_TAG, ret);
    }
    ret = devmgr_->UnloadDevice(AUDIOEXT_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("%s: Unload provider service failed, ret: %d", LOG_TAG, ret);
    }
    audioServStatus_ = INVALID_VALUE;
    audioextServStatus_ = INVALID_VALUE;
    DHLOGI("%s: UnLoadDaudioHDFImpl end!", LOG_TAG);
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS