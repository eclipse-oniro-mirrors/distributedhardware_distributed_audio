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

#include "daudio_manager_interface_impl.h"

#include <hdf_base.h>
#include "daudio_errcode.h"
#include "daudio_log.h"
#include "daudio_utils.h"

using namespace OHOS::DistributedHardware;
using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audioext {
namespace V1_0 {
DAudioManagerInterfaceImpl *DAudioManagerInterfaceImpl::dmgr = nullptr;
std::mutex DAudioManagerInterfaceImpl::mutex_dmgr;
extern "C" IDAudioManager *DAudioManagerImplGetInstance(void)
{
    return DAudioManagerInterfaceImpl::GetDAudioManager();
}

DAudioManagerInterfaceImpl::DAudioManagerInterfaceImpl()
{
    DHLOGD("%s: Distributed Audio Ext Manager constructed.", AUDIO_LOG);
    audiomgr_ = AudioManagerInterfaceImpl::GetAudioManager();
}

DAudioManagerInterfaceImpl::~DAudioManagerInterfaceImpl()
{
    DHLOGD("%s: Distributed Audio Ext Manager destructed.", AUDIO_LOG);
}

int32_t DAudioManagerInterfaceImpl::RegisterAudioDevice(const std::string &adpName, int32_t devId,
    const std::string &capability, const sptr<IDAudioCallback> &callbackObj)
{
    DHLOGI("%s: Register audio device, name: %s, device: %d.", AUDIO_LOG, GetAnonyString(adpName).c_str(), devId);
    if (audiomgr_ == nullptr) {
        DHLOGE("%s: Audio manager is null.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = audiomgr_->AddAudioDevice(adpName, devId, capability, callbackObj);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register audio device failed, ret = %d", AUDIO_LOG, ret);
        return HDF_FAILURE;
    }

    DHLOGI("%s: Register audio device success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t DAudioManagerInterfaceImpl::UnRegisterAudioDevice(const std::string &adpName, int32_t devId)
{
    DHLOGI("%s: UnRegister audio device, name: %s, device: %d.", AUDIO_LOG, GetAnonyString(adpName).c_str(), devId);
    if (audiomgr_ == nullptr) {
        DHLOGE("%s: Audio manager is null.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = audiomgr_->RemoveAudioDevice(adpName, devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: UnRegister audio devcie failed. ret = %d", AUDIO_LOG, ret);
        return HDF_FAILURE;
    }

    DHLOGI("%s: UnRegister audio device success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t DAudioManagerInterfaceImpl::NotifyEvent(const std::string &adpName, int32_t devId, const AudioEvent &event)
{
    DHLOGI("%s: NotifyEvent type: %d, content: %s.", AUDIO_LOG, event.type, event.content.c_str());
    if (audiomgr_ == nullptr) {
        DHLOGE("%s: Audio manager is null.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = audiomgr_->Notify(adpName, devId, event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify audio event failed. ret = %d", AUDIO_LOG, ret);
        return HDF_FAILURE;
    }

    DHLOGI("%s: NotifyEvent success.", AUDIO_LOG);
    return HDF_SUCCESS;
}
} // V1_0
} // AudioExt
} // Daudio
} // HDI
} // OHOS
