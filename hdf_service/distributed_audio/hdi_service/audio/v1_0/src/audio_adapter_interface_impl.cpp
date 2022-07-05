/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "audio_adapter_interface_impl.h"

#include <hdf_base.h>

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
AudioAdapterInterfaceImpl::AudioAdapterInterfaceImpl(const AudioAdapterDescriptorHAL &desc)
    : adpDescriptor_(desc)
{
    DHLOGD("%s: Distributed Audio Adapter constructed, name(%s).", AUDIO_LOG, GetAnonyString(desc.adapterName).c_str());
}

AudioAdapterInterfaceImpl::~AudioAdapterInterfaceImpl()
{
    DHLOGD("%s: Distributed Audio Adapter destructed, name(%s).", AUDIO_LOG,
        GetAnonyString(adpDescriptor_.adapterName).c_str());
}


void AudioAdapterInterfaceImpl::SetSpeakerCallback(const sptr<IDAudioCallback> &speakerCallback)
{
    extSpeakerCallback_ = speakerCallback;
}

void AudioAdapterInterfaceImpl::SetMicCallback(const sptr<IDAudioCallback> &micCallback)
{
    extMicCallback_ = micCallback;
}

int32_t AudioAdapterInterfaceImpl::InitAllPorts()
{
    DHLOGI("%s: Init (%zu) distributed audio ports.", AUDIO_LOG, mapAudioDevice_.size());
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::CreateRender(const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs, sptr<IAudioRender> &render)
{
    DHLOGI("%s: Create distributed audio render, {pin: %zu, sampleRate: %zu, channel: %zu, formats: %zu}.", AUDIO_LOG,
        desc.pins, attrs.sampleRate, attrs.channelCount, attrs.format);
    render = nullptr;
    {
        std::lock_guard<std::mutex> devLck(devMapMtx_);
        if (mapAudioDevice_.find(desc.pins) == mapAudioDevice_.end()) {
            DHLOGE("%s: Can not find device, create render failed.", AUDIO_LOG);
            return HDF_FAILURE;
        }
    }
    audioRender_ = new AudioRenderInterfaceImpl(adpDescriptor_.adapterName, desc, attrs, extSpeakerCallback_);
    if (audioRender_ == nullptr) {
        DHLOGE("%s: Create render failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = OpenRenderDevice(desc, attrs);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Open render device failed.", AUDIO_LOG);
        audioRender_ = nullptr;
        return HDF_FAILURE;
    }
    render = audioRender_;
    DHLOGI("%s: Create render success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::DestoryRender(const AudioDeviceDescriptorHAL &desc)
{
    DHLOGI("%s: Destory distributed audio render, {pin: %zu}.", AUDIO_LOG, desc.pins);
    if (audioRender_ == nullptr) {
        DHLOGD("%s: Render has not been created, do not need destory.", AUDIO_LOG);
        return HDF_SUCCESS;
    }
    if (desc.pins != audioRender_->GetRenderDesc().pins) {
        DHLOGD("%s: Render pin is wrong, destory render failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = CloseRenderDevice(desc);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Close render device failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }
    audioRender_ = nullptr;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::CreateCapture(const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs, sptr<IAudioCapture> &capture)
{
    DHLOGI("%s: Create distributed audio capture, {pin: %zu, sampleRate: %zu, channel: %zu, formats: %zu}.", AUDIO_LOG,
        desc.pins, attrs.sampleRate, attrs.channelCount, attrs.format);
    capture = nullptr;
    {
        std::lock_guard<std::mutex> devLck(devMapMtx_);
        if (mapAudioDevice_.find(desc.pins) == mapAudioDevice_.end()) {
            DHLOGE("%s: Can not find device, create capture failed.", AUDIO_LOG);
            return HDF_FAILURE;
        }
    }
    audioCapture_ = new AudioCaptureInterfaceImpl(adpDescriptor_.adapterName, desc, attrs, extMicCallback_);
    if (audioCapture_ == nullptr) {
        DHLOGE("%s: Create capture failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = OpenCaptureDevice(desc, attrs);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Open capture device failed.", AUDIO_LOG);
        audioCapture_ = nullptr;
        return HDF_FAILURE;
    }
    capture = audioCapture_;
    DHLOGI("%s: Create capture success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::DestoryCapture(const AudioDeviceDescriptorHAL &desc)
{
    DHLOGI("%s: Destory distributed audio capture, {pin: %zu}.", AUDIO_LOG, desc.pins);
    if (audioCapture_ == nullptr) {
        DHLOGD("%s: Capture has not been created, do not need destory.", AUDIO_LOG);
        return HDF_SUCCESS;
    }
    if (desc.pins != audioCapture_->GetCaptureDesc().pins) {
        DHLOGD("%s: Capture pin is wrong, destory capture failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    int32_t ret = CloseCaptureDevice(desc);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Close capture device failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }
    audioCapture_ = nullptr;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::GetPortCapability(const AudioPortHAL &port, AudioPortCapabilityHAl &capability)
{
    DHLOGI("%s: Get audio port capability.", AUDIO_LOG);
    (void)port;
    capability.sampleRateMasks = AUDIO_SAMPLE_RATE_DEFAULT;
    capability.channelCount = AUDIO_CHANNEL_COUNT_DEFAULT;

    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::SetPassthroughMode(const AudioPortHAL &port, AudioPortPassthroughModeHAL mode)
{
    DHLOGI("%s: Distributed audio not support yet.", AUDIO_LOG);
    (void)port;
    (void)mode;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::GetPassthroughMode(const AudioPortHAL &port, AudioPortPassthroughModeHAL &mode)
{
    DHLOGI("%s: Distributed audio not support yet.", AUDIO_LOG);
    (void)port;
    (void)mode;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::UpdateAudioRoute(const AudioRouteHAL &route, int32_t &handle)
{
    (void) route;
    (void) handle;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::ReleaseAudioRoute(int32_t handle)
{
    (void) handle;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::SetAudioParameters(AudioExtParamKey key, const std::string &condition,
    const std::string &value)
{
    DHLOGI("%s: Set audio parameters, key: %d, condition: %s, value: %s.", AUDIO_LOG, key, condition.c_str(),
        value.c_str());
    int32_t ret = ERR_DH_AUDIO_HDF_FAIL;
    switch (key) {
        case AudioExtParamKey::AUDIO_EXT_PARAM_KEY_VOLUME:
            ret = SetAudioVolume(condition, value);
            if (ret != DH_SUCCESS) {
                DHLOGE("%s: Set audio parameters failed.", AUDIO_LOG);
                return HDF_FAILURE;
            }
            break;
        case AudioExtParamKey::AUDIO_EXT_PARAM_KEY_NONE:
            DHLOGE("%s: Parameter is unknown.", AUDIO_LOG);
            break;
        default:
            DHLOGE("%s: Parameter is invalid.", AUDIO_LOG);
            return HDF_ERR_INVALID_PARAM;
    }
    DHLOGI("%s: Set audio parameters success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::GetAudioParameters(AudioExtParamKey key, const std::string &condition,
    std::string &value)
{
    DHLOGI("%s: Get audio parameters, key: %d, condition: %s.", AUDIO_LOG, key, condition.c_str());
    int32_t ret = ERR_DH_AUDIO_HDF_FAIL;
    switch (key) {
        case AudioExtParamKey::AUDIO_EXT_PARAM_KEY_VOLUME:
            ret = GetAudioVolume(condition, value);
            if (ret != DH_SUCCESS) {
                DHLOGE("%s: Get audio parameters failed.", AUDIO_LOG);
                return HDF_FAILURE;
            }
            break;
        case AudioExtParamKey::AUDIO_EXT_PARAM_KEY_NONE:
            DHLOGE("%s: Parameter is unknown.", AUDIO_LOG);
            break;
        default:
            DHLOGE("%s: Parameter is invalid.", AUDIO_LOG);
            return HDF_ERR_INVALID_PARAM;
    }
    DHLOGI("%s: Get audio parameters success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::RegAudioParamObserver(const sptr<IAudioParamCallback> &cbObj)
{
    DHLOGI("%s: Register audio param observer.", AUDIO_LOG);
    if (paramCallback_ == nullptr) {
        DHLOGE("%s: Audio param observer is null.", AUDIO_LOG);
        return HDF_FAILURE;
    }
    paramCallback_ = cbObj;
    return HDF_SUCCESS;
}

AudioAdapterDescriptorHAL AudioAdapterInterfaceImpl::GetAdapterDesc()
{
    adpDescriptor_.ports.clear();
    for (auto pin = mapAudioDevice_.begin(); pin != mapAudioDevice_.end(); pin++) {
        AudioPortHAL port = {0, pin->first, ""};
        adpDescriptor_.ports.emplace_back(port);
    }
    return adpDescriptor_;
}

std::string AudioAdapterInterfaceImpl::GetDeviceCapabilitys(const uint32_t devId)
{
    std::lock_guard<std::mutex> devLck(devMapMtx_);
    std::string caps;
    auto dev = mapAudioDevice_.find(devId);
    if (dev == mapAudioDevice_.end()) {
        DHLOGE("%s: Device not found.", AUDIO_LOG);
        return caps;
    }
    return dev->second;
}

int32_t AudioAdapterInterfaceImpl::AdapterLoad()
{
    status_ = AudioAdapterStatus::STATUS_LOAD;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::AdapterUnload()
{
    status_ = AudioAdapterStatus::STATUS_UNLOAD;
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::Notify(const uint32_t devId, const AudioEvent &event)
{
    switch ((AudioExtParamEvent)event.type) {
        case HDF_AUDIO_EVENT_VOLUME_CHANGE:
            DHLOGI("%s: Notify event: VOLUME_CHANGE, event content: %s.", AUDIO_LOG, event.content.c_str());
            return HandleVolumeChangeEvent(event);
        case HDF_AUDIO_EVENT_OPEN_SPK_RESULT:
            DHLOGI("%s: Notify event: SPEAKER_OPEN, event content: %s.", AUDIO_LOG, event.content.c_str());
            return HandleOpenSpeakerEvent(event);
        case HDF_AUDIO_EVENT_OPEN_MIC_RESULT:
            DHLOGI("%s: Notify event: MIC_OPEN, event content: %s.", AUDIO_LOG, event.content.c_str());
            return HandleOpenMicEvent(event);
        default:
            DHLOGE("%s: Audio event: %d is undefined.", AUDIO_LOG, event.type);
            return ERR_DH_AUDIO_HDF_INVALID_OPERATION;
    }
}

int32_t AudioAdapterInterfaceImpl::AddAudioDevice(const uint32_t devId, const std::string &caps)
{
    DHLOGI("%s: Add distributed audio device %d.", AUDIO_LOG, devId);
    std::lock_guard<std::mutex> devLck(devMapMtx_);
    auto dev = mapAudioDevice_.find(devId);
    if (dev != mapAudioDevice_.end()) {
        DHLOGE("%s: Device has been add, do not repeat add.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_OPERATION;
    }
    mapAudioDevice_.insert(std::make_pair(devId, caps));

    DHLOGI("%s: Add audio device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::RemoveAudioDevice(const uint32_t devId)
{
    DHLOGI("%s: Remove distributed audio device %d.", AUDIO_LOG, devId);
    std::lock_guard<std::mutex> devLck(devMapMtx_);
    auto dev = mapAudioDevice_.find(devId);
    if (dev == mapAudioDevice_.end()) {
        DHLOGE("%s: Device has not been add, remove device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_OPERATION;
    }
    mapAudioDevice_.erase(devId);

    DHLOGI("%s: Remove audio device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::OpenRenderDevice(const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Open render device, pin: %d.", AUDIO_LOG, desc.pins);
    renderParam_.format = attrs.format;
    renderParam_.channelCount = attrs.channelCount;
    renderParam_.sampleRate = attrs.sampleRate;

    int32_t ret = extSpeakerCallback_->SetParameters(adpDescriptor_.adapterName, desc.pins, renderParam_);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Set render parameters failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_SET_PARAM_FAIL;
    }
    ret = extSpeakerCallback_->OpenDevice(adpDescriptor_.adapterName, desc.pins);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Open render device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
    }

    spkNotifyFlag_ = false;
    std::unique_lock<std::mutex> lck(spkWaitMutex_);
    auto status =
        spkWaitCond_.wait_for(lck, std::chrono::seconds(OPEN_WAIT_SECONDS), [this]() { return spkNotifyFlag_; });
    if (!status) {
        DHLOGE("%s: Open render device wait timeout(%d)s.", AUDIO_LOG, OPEN_WAIT_SECONDS);
        return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
    }
    if (isSpkOpened_ != true) {
        DHLOGE("%s: Wait open render device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
    }
    DHLOGI("%s: Open render device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::CloseRenderDevice(const AudioDeviceDescriptorHAL &desc)
{
    DHLOGI("%s: Close render device, pin: %d.", AUDIO_LOG, desc.pins);
    renderParam_ = {};
    int32_t ret = extSpeakerCallback_->CloseDevice(adpDescriptor_.adapterName, desc.pins);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Close audio device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_CLOSE_DEVICE_FAIL;
    }

    isSpkOpened_ = false;
    DHLOGI("%s: Close render device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::OpenCaptureDevice(const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Open capture device, pin: %d.", AUDIO_LOG, desc.pins);
    captureParam_.format = attrs.format;
    captureParam_.channelCount = attrs.channelCount;
    captureParam_.sampleRate = attrs.sampleRate;

    int32_t ret = extMicCallback_->SetParameters(adpDescriptor_.adapterName, desc.pins, captureParam_);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Set audio parameters failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_SET_PARAM_FAIL;
    }
    ret = extMicCallback_->OpenDevice(adpDescriptor_.adapterName, desc.pins);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Open audio device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
    }

    micNotifyFlag_ = false;
    std::unique_lock<std::mutex> lck(micWaitMutex_);
    auto status =
        micWaitCond_.wait_for(lck, std::chrono::seconds(OPEN_WAIT_SECONDS), [this]() { return micNotifyFlag_; });
    if (!status) {
        DHLOGE("%s: Open capture device wait timeout(%ds).", AUDIO_LOG, OPEN_WAIT_SECONDS);
        return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
    }
    if (isMicOpened_ != true) {
        DHLOGE("%s: Wait open capture device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
    }
    DHLOGI("%s: Open capture device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::CloseCaptureDevice(const AudioDeviceDescriptorHAL &desc)
{
    DHLOGI("%s: Close capture device, pin: %d.", AUDIO_LOG, desc.pins);
    captureParam_ = {};
    int32_t ret = extMicCallback_->CloseDevice(adpDescriptor_.adapterName, desc.pins);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Close audio device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_CLOSE_DEVICE_FAIL;
    }

    isMicOpened_ = false;
    DHLOGI("%s: Close capture device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

uint32_t AudioAdapterInterfaceImpl::GetVolumeGroup(const uint32_t devId)
{
    uint32_t volGroup = VOLUME_GROUP_ID_DEFAULT;
    std::lock_guard<std::mutex> devLck(devMapMtx_);
    auto caps = mapAudioDevice_.find(devId);
    if (caps == mapAudioDevice_.end()) {
        DHLOGE("%s: Can not find caps of dev:%u.", AUDIO_LOG, devId);
    }

    int32_t ret = GetAudioParamUInt(caps->second, VOLUME_GROUP_ID, volGroup);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get volume group param failed, use default value, ret = %d.", AUDIO_LOG, ret);
    }
    return volGroup;
}

uint32_t AudioAdapterInterfaceImpl::GetInterruptGroup(const uint32_t devId)
{
    uint32_t iptGroup = INTERRUPT_GROUP_ID_DEFAULT;
    std::lock_guard<std::mutex> devLck(devMapMtx_);
    auto caps = mapAudioDevice_.find(devId);
    if (caps == mapAudioDevice_.end()) {
        DHLOGE("%s: Can not find caps of dev:%u.", AUDIO_LOG, devId);
    }

    int32_t ret = GetAudioParamUInt(caps->second, INTERRUPT_GROUP_ID, iptGroup);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get interrupt group param failed, use default value, ret = %d.", AUDIO_LOG, ret);
    }
    return iptGroup;
}

int32_t AudioAdapterInterfaceImpl::SetAudioVolume(const std::string& condition, const std::string &param)
{
    if (extSpeakerCallback_ == nullptr) {
        DHLOGE("%s: Callback is nullptr.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    std::string content = condition;
    int32_t ret = SetAudioParamStr(content, VOLUME_LEVEL, param);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Can not set vol value, ret = %d.", AUDIO_LOG, ret);
    }
    AudioEvent event = { HDF_AUDIO_EVENT_VOLUME_SET, content };
    ret = extSpeakerCallback_->NotifyEvent(adpDescriptor_.adapterName, audioRender_->GetRenderDesc().pins, event);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: NotifyEvent failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::GetAudioVolume(const std::string& condition, std::string &param)
{
    (void) condition;
    uint32_t vol = audioRender_->GetVolumeInner();
    param = std::to_string(vol);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::HandleVolumeChangeEvent(const AudioEvent &event)
{
    DHLOGI("%s: Vol change (%s).", AUDIO_LOG, event.content.c_str());
    if (paramCallback_ == nullptr) {
        DHLOGE("%s: Audio param observer is null.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    uint32_t vol = 0;
    int32_t ret = GetAudioParamUInt(event.content, VOLUME_LEVEL, vol);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get volume value failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    audioRender_->SetVolumeInner(vol);

    ret = paramCallback_->OnAudioParamNotify(AUDIO_EXT_PARAM_KEY_VOLUME, event.content, std::to_string(vol));
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify vol failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::HandleOpenSpeakerEvent(const AudioEvent &event)
{
    if (event.content == HDF_EVENT_RESULT_SUCCESS) {
        isSpkOpened_ = true;
    }
    spkNotifyFlag_ = true;
    spkWaitCond_.notify_all();
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::HandleOpenMicEvent(const AudioEvent &event)
{
    if (event.content == HDF_EVENT_RESULT_SUCCESS) {
        isMicOpened_ = true;
    }
    micNotifyFlag_ = true;
    micWaitCond_.notify_all();
    return DH_SUCCESS;
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS