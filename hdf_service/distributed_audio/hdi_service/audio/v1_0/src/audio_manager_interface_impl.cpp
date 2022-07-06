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

#include "audio_manager_interface_impl.h"

#include <hdf_base.h>
#include "hdf_device_object.h"
#include <sstream>

#include "daudio_constants.h"
#include "daudio_errcode.h"
#include "daudio_events.h"
#include "daudio_log.h"
#include "daudio_utils.h"

using namespace OHOS::DistributedHardware;
namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
AudioManagerInterfaceImpl *AudioManagerInterfaceImpl::mgr = nullptr;
std::mutex AudioManagerInterfaceImpl::mutex_mgr;
extern "C" IAudioManager *AudioManagerImplGetInstance(void)
{
    return AudioManagerInterfaceImpl::GetAudioManager();
}

AudioManagerInterfaceImpl::AudioManagerInterfaceImpl()
{
    DHLOGD("%s: Distributed Audio Manager constructed.", AUDIO_LOG);
}

AudioManagerInterfaceImpl::~AudioManagerInterfaceImpl()
{
    DHLOGD("%s: Distributed Audio Manager destructed.", AUDIO_LOG);
}

int32_t AudioManagerInterfaceImpl::GetAllAdapters(std::vector<AudioAdapterDescriptorHAL> &descriptors)
{
    DHLOGI("%s: Get all distributed audio adapters.", AUDIO_LOG);
    std::lock_guard<std::mutex> adpLck(adapterMapMtx_);
    for (auto &adp : mapAudioAdapter_) {
        descriptors.push_back(adp.second->GetAdapterDesc());
    }

    DHLOGI("%s: Get adapters success, total is (%zu). ", AUDIO_LOG, mapAudioAdapter_.size());
    return HDF_SUCCESS;
}

int32_t AudioManagerInterfaceImpl::LoadAdapter(const AudioAdapterDescriptorHAL &descriptor,
    sptr<IAudioAdapter> &adapter)
{
    DHLOGI("%s: Load distributed audio adapter: %s.", AUDIO_LOG, GetAnonyString(descriptor.adapterName).c_str());
    std::lock_guard<std::mutex> adpLck(adapterMapMtx_);
    auto adp = mapAudioAdapter_.find(descriptor.adapterName);
    if (adp == mapAudioAdapter_.end()) {
        DHLOGE("%s: Load audio adapter failed, can not find adapter.", AUDIO_LOG);
        adapter = nullptr;
        return HDF_FAILURE;
    }

    int32_t ret = adp->second->AdapterLoad();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Load audio adapter failed, adapter return: %d.", AUDIO_LOG, ret);
        adapter = nullptr;
        return HDF_FAILURE;
    }

    adapter = adp->second;
    DHLOGI("%s: Load adapter success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioManagerInterfaceImpl::UnloadAdapter(const std::string &adpName)
{
    DHLOGI("%s: Unload distributed audio adapter: %s.", AUDIO_LOG, GetAnonyString(adpName).c_str());
    std::lock_guard<std::mutex> adpLck(adapterMapMtx_);
    auto adp = mapAudioAdapter_.find(adpName);
    if (adp == mapAudioAdapter_.end()) {
        DHLOGE("%s: Unload audio adapter failed, can not find adapter.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = adp->second->AdapterUnload();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Unload audio adapter failed, adapter return: %d.", AUDIO_LOG, ret);
        return HDF_FAILURE;
    }
    DHLOGI("%s: Unload adapter success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioManagerInterfaceImpl::AddAudioDevice(const std::string &adpName, const uint32_t devId,
    const std::string &caps, const sptr<IDAudioCallback> &callback)
{
    DHLOGI("%s: Add audio device name: %s, device: %d.", AUDIO_LOG, GetAnonyString(adpName).c_str(), devId);
    std::lock_guard<std::mutex> adpLck(adapterMapMtx_);
    auto adp = mapAudioAdapter_.find(adpName);
    if (adp == mapAudioAdapter_.end()) {
        int32_t ret = CreateAdapter(adpName, devId, callback);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Create audio adapter failed.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_FAIL;
        }
    }

    adp = mapAudioAdapter_.find(adpName);
    if (adp == mapAudioAdapter_.end()) {
        DHLOGE("%s: Can not find adapter. device name: %s", AUDIO_LOG, GetAnonyString(adpName).c_str());
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    switch (GetDevTypeByDHId(devId)) {
        case AUDIO_DEVICE_TYPE_SPEAKER:
            adp->second->SetSpeakerCallback(callback);
            break;
        case AUDIO_DEVICE_TYPE_MIC:
            adp->second->SetMicCallback(callback);
            break;
        case AUDIO_DEVICE_TYPE_UNKNOWN:
        default:
            DHLOGE("%s: DhId is illegal, devType is unknow.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_FAIL;
    }
    int32_t ret = adp->second->AddAudioDevice(devId, caps);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Add audio device failed, adapter return: %d.", AUDIO_LOG, ret);
        return ERR_DH_AUDIO_HDF_FAIL;
    }

    DAudioDevEvent event = { adpName,
                             devId,
                             HDF_AUDIO_DEVICE_ADD,
                             0,
                             adp->second->GetVolumeGroup(devId),
                             adp->second->GetInterruptGroup(devId) };
    ret = NotifyFwk(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify audio fwk failed, ret = %d.", AUDIO_LOG, ret);
        return ret;
    }
    DHLOGI("%s: Add audio device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioManagerInterfaceImpl::RemoveAudioDevice(const std::string &adpName, const uint32_t devId)
{
    DHLOGI("%s: Remove audio device name: %s, device: %d.", AUDIO_LOG, GetAnonyString(adpName).c_str(), devId);
    std::lock_guard<std::mutex> adpLck(adapterMapMtx_);
    auto adp = mapAudioAdapter_.find(adpName);
    if (adp == mapAudioAdapter_.end()) {
        DHLOGE("%s: Audio device has not been created.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_OPERATION;
    }

    int32_t ret = adp->second->RemoveAudioDevice(devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Remove audio device failed, adapter return: %d.", AUDIO_LOG, ret);
        return ERR_DH_AUDIO_HDF_FAIL;
    }

    DAudioDevEvent event = { adpName,
                             devId,
                             HDF_AUDIO_DEVICE_REMOVE,
                             0,
                             adp->second->GetVolumeGroup(devId),
                             adp->second->GetInterruptGroup(devId) };
    ret = NotifyFwk(event);
    if (ret != DH_SUCCESS) {
        DHLOGD("%s: Notify audio fwk failed, ret = %d.", AUDIO_LOG, ret);
    }
    if (adp->second->isPortsNoReg()) {
        mapAudioAdapter_.erase(adpName);
    }
    DHLOGI("%s: Remove audio device success, mapAudioAdapter_ size() is : %d .", AUDIO_LOG, mapAudioAdapter_.size());
    return DH_SUCCESS;
}

int32_t AudioManagerInterfaceImpl::Notify(const std::string &adpName, const uint32_t devId, const AudioEvent &event)
{
    DHLOGI("%s: Notify event, adapter name: %s. event type: %d", AUDIO_LOG,
        GetAnonyString(adpName).c_str(), event.type);
    std::lock_guard<std::mutex> adpLck(adapterMapMtx_);
    auto adp = mapAudioAdapter_.find(adpName);
    if (adp == mapAudioAdapter_.end()) {
        DHLOGE("%s: Notify failed, can not find adapter.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_OPERATION;
    }

    int32_t ret = adp->second->Notify(devId, event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify failed, adapter return: %d.", AUDIO_LOG, ret);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

int32_t AudioManagerInterfaceImpl::NotifyFwk(const DAudioDevEvent &event)
{
    DHLOGI("%s: Notify audio fwk event(type:%d, adapter:%s, pin:%d).", AUDIO_LOG, event.eventType,
        GetAnonyString(event.adapterName).c_str(), event.devId);
    std::stringstream ss;
    ss << "EVENT_TYPE=" << event.eventType << ";NID=" << event.adapterName << ";PIN=" << event.devId << ";VID=" <<
        event.volGroupId << ";IID=" << event.iptGroupId;
    std::string eventInfo = ss.str();
    int ret = HdfDeviceObjectSetServInfo(deviceObject_, eventInfo.c_str());
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Set service info failed, ret = %d.", AUDIO_LOG, ret);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    ret = HdfDeviceObjectUpdate(deviceObject_);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Update service info failed, ret = %d.", AUDIO_LOG, ret);
        return ERR_DH_AUDIO_HDF_FAIL;
    }

    DHLOGI("%s: Notify audio fwk success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioManagerInterfaceImpl::CreateAdapter(const std::string &adpName, const uint32_t devId,
    const sptr<IDAudioCallback> &callback)
{
    if (callback == nullptr) {
        DHLOGE("%s: Adapter callback is null.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    if (devId != PIN_OUT_DAUDIO_DEFAULT && devId != PIN_IN_DAUDIO_DEFAULT) {
        DHLOGE("%s: Pin is not default, can not create audio adapter.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }

    AudioAdapterDescriptorHAL desc = { adpName };
    sptr<AudioAdapterInterfaceImpl> adapter(new AudioAdapterInterfaceImpl(desc));
    if (adapter == nullptr) {
        DHLOGE("%s: Create new audio adapter failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    mapAudioAdapter_.insert(std::make_pair(adpName, adapter));
    return DH_SUCCESS;
}

void AudioManagerInterfaceImpl::SetDeviceObject(struct HdfDeviceObject *deviceObject)
{
    deviceObject_ = deviceObject;
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOSf
