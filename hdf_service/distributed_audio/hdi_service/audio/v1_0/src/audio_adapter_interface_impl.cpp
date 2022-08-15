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
AudioAdapterInterfaceImpl::AudioAdapterInterfaceImpl(const AudioAdapterDescriptorHAL &desc)
    : adpDescriptor_(desc)
{
    renderParam_.format = 0;
    renderParam_.channelCount = 0;
    renderParam_.sampleRate = 0;
    renderParam_.period = 0;
    renderParam_.frameSize = 0;
    renderParam_.streamUsage = 0;

    captureParam_.format = 0;
    captureParam_.channelCount = 0;
    captureParam_.sampleRate = 0;
    captureParam_.period = 0;
    captureParam_.frameSize = 0;
    captureParam_.streamUsage = 0;
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

int32_t AudioAdapterInterfaceImpl::SetAudioParameters(AudioExtParamKeyHAL key, const std::string &condition,
    const std::string &value)
{
    DHLOGI("%s: Set audio parameters, key = %d, condition: %s value: %s.", AUDIO_LOG, key, condition.c_str(),
        value.c_str());
    int32_t ret = ERR_DH_AUDIO_HDF_FAIL;
    switch (key) {
        case AudioExtParamKeyHAL::AUDIO_EXT_PARAM_KEY_VOLUME:
            ret = SetAudioVolume(condition, value);
            if (ret != DH_SUCCESS) {
                DHLOGE("%s: Set audio parameters failed.", AUDIO_LOG);
                return HDF_FAILURE;
            }
            break;
        case AudioExtParamKeyHAL::AUDIO_EXT_PARAM_KEY_NONE:
            DHLOGE("%s: Parameter is unknown.", AUDIO_LOG);
            break;
        default:
            DHLOGE("%s: Parameter is invalid.", AUDIO_LOG);
            return HDF_ERR_INVALID_PARAM;
    }
    DHLOGI("%s: Set audio parameters success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::GetAudioParameters(AudioExtParamKeyHAL key, const std::string &condition,
    std::string &value)
{
    DHLOGI("%s: Get audio parameters, key: %d, condition: %s.", AUDIO_LOG, key, condition.c_str());
    int32_t ret = ERR_DH_AUDIO_HDF_FAIL;
    switch (key) {
        case AudioExtParamKeyHAL::AUDIO_EXT_PARAM_KEY_VOLUME:
            ret = GetAudioVolume(condition, value);
            if (ret != DH_SUCCESS) {
                DHLOGE("%s: Get audio parameters failed.", AUDIO_LOG);
                return HDF_FAILURE;
            }
            break;
        case AudioExtParamKeyHAL::AUDIO_EXT_PARAM_KEY_NONE:
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
    paramCallback_ = cbObj;
    return HDF_SUCCESS;
}

AudioAdapterDescriptorHAL AudioAdapterInterfaceImpl::GetAdapterDesc()
{
    adpDescriptor_.ports.clear();
    std::lock_guard<std::mutex> devLck(devMapMtx_);
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
        case HDF_AUDIO_EVENT_FOCUS_CHANGE:
            DHLOGI("%s: Notify event: FOCUS_CHANGE, event content: %s.", AUDIO_LOG, event.content.c_str());
            return HandleFocusChangeEvent(event);
        case HDF_AUDIO_EVENT_RENDER_STATE_CHANGE:
            DHLOGI("%s: Notify event: RENDER_STATE_CHANGE, event content: %s.", AUDIO_LOG, event.content.c_str());
            return HandleRenderStateChangeEvent(event);
        case HDF_AUDIO_EVENT_OPEN_SPK_RESULT:
        case HDF_AUDIO_EVENT_CLOSE_SPK_RESULT:
        case HDF_AUDIO_EVENT_OPEN_MIC_RESULT:
        case HDF_AUDIO_EVENT_CLOSE_MIC_RESULT:
            return HandleSANotifyEvent(event);
        case HDF_AUDIO_EVENT_SPK_CLOSED:
        case HDF_AUDIO_EVENT_MIC_CLOSED:
            return HandleDeviceClosed(event);
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
    {
        std::lock_guard<std::mutex> devLck(devMapMtx_);
        auto dev = mapAudioDevice_.find(devId);
        if (dev == mapAudioDevice_.end()) {
            DHLOGE("%s: Device has not been add, remove device failed.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_INVALID_OPERATION;
        }
        mapAudioDevice_.erase(devId);
    }
    AudioDeviceDescriptorHAL dec;
    if (devId == spkPinInUse_) {
        dec.pins = spkPinInUse_;
        DestoryRender(dec);
    }
    if (devId == micPinInUse_) {
        dec.pins = micPinInUse_;
        DestoryCapture(dec);
    }

    DHLOGI("%s: Remove audio device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::OpenRenderDevice(const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Open render device, pin: %d.", AUDIO_LOG, desc.pins);
    if (isSpkOpened_) {
        DHLOGI("%s: Render already opened.", AUDIO_LOG);
        return DH_SUCCESS;
    }
    std::lock_guard<std::mutex> devLck(renderOptMtx_);
    spkPinInUse_ = desc.pins;
    renderParam_.format = attrs.format;
    renderParam_.channelCount = attrs.channelCount;
    renderParam_.sampleRate = attrs.sampleRate;
    renderParam_.streamUsage = attrs.type;

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

    ret = WaitForSANotify(EVENT_OPEN_SPK);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Wait SA notify failed.", AUDIO_LOG);
        return ret;
    }
    DHLOGI("%s: Open render device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::CloseRenderDevice(const AudioDeviceDescriptorHAL &desc)
{
    DHLOGI("%s: Close render device, pin: %d.", AUDIO_LOG, desc.pins);
    std::lock_guard<std::mutex> devLck(renderOptMtx_);
    if (spkPinInUse_ == 0) {
        DHLOGI("%s: No need close render device.", AUDIO_LOG);
        return DH_SUCCESS;
    }
    renderParam_ = {};
    int32_t ret = extSpeakerCallback_->CloseDevice(adpDescriptor_.adapterName, desc.pins);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Close audio device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_CLOSE_DEVICE_FAIL;
    }

    ret = WaitForSANotify(EVENT_CLOSE_SPK);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Wait SA notify failed.", AUDIO_LOG);
        return ret;
    }
    spkPinInUse_ = 0;
    DHLOGI("%s: Close render device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::OpenCaptureDevice(const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Open capture device, pin: %d.", AUDIO_LOG, desc.pins);
    if (isMicOpened_) {
        DHLOGI("%s: Capture already opened.", AUDIO_LOG);
        return DH_SUCCESS;
    }
    std::lock_guard<std::mutex> devLck(captureOptMtx_);
    micPinInUse_ = desc.pins;
    captureParam_.format = attrs.format;
    captureParam_.channelCount = attrs.channelCount;
    captureParam_.sampleRate = attrs.sampleRate;
    captureParam_.streamUsage = attrs.type;

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

    ret = WaitForSANotify(EVENT_OPEN_MIC);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Wait SA notify failed.", AUDIO_LOG);
        return ret;
    }
    DHLOGI("%s: Open capture device success.", AUDIO_LOG);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::CloseCaptureDevice(const AudioDeviceDescriptorHAL &desc)
{
    DHLOGI("%s: Close capture device, pin: %d.", AUDIO_LOG, desc.pins);
    std::lock_guard<std::mutex> devLck(captureOptMtx_);
    if (micPinInUse_ == 0) {
        DHLOGI("%s: No need close capture device.", AUDIO_LOG);
        return DH_SUCCESS;
    }
    captureParam_ = {};
    int32_t ret = extMicCallback_->CloseDevice(adpDescriptor_.adapterName, desc.pins);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Close audio device failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_CLOSE_DEVICE_FAIL;
    }

    ret = WaitForSANotify(EVENT_CLOSE_MIC);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Wait SA notify failed.", AUDIO_LOG);
        return ret;
    }
    micPinInUse_ = 0;
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
    if (audioRender_ == nullptr) {
        DHLOGD("%s: Render has not been created.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    std::string content = condition;
    std::string valueParam = param;
    int32_t type = getEventTypeFromCondition(content);
    EXT_PARAM_EVENT eventType;
    if (type == VolumeEventType::EVENT_IS_STREAM_MUTE) {
        valueParam = "0";
        eventType = HDF_AUDIO_EVNET_MUTE_SET;
        isStreamMute_ = 1;
    } else {
        eventType = HDF_AUDIO_EVENT_VOLUME_SET;
        isStreamMute_ = 0;
    }
    int32_t ret = SetAudioParamStr(content, VOLUME_LEVEL, valueParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Can not set vol value, ret = %d.", AUDIO_LOG, ret);
    }
    AudioEvent event = { eventType, content };
    ret = extSpeakerCallback_->NotifyEvent(adpDescriptor_.adapterName, audioRender_->GetRenderDesc().pins, event);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: NotifyEvent failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::GetAudioVolume(const std::string& condition, std::string &param)
{
    if (audioRender_ == nullptr) {
        DHLOGD("%s: Render has not been created.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    int32_t type = getEventTypeFromCondition(condition);
    uint32_t vol;
    switch (type) {
        case VolumeEventType::EVENT_GET_VOLUME:
            vol = audioRender_->GetVolumeInner();
            break;
        case VolumeEventType::EVENT_GET_MAX_VOLUME:
            vol = audioRender_->GetMaxVolumeInner();
            break;
        case VolumeEventType::EVENT_GET_MIN_VOLUME:
            vol = audioRender_->GetMinVolumeInner();
            break;
        case VolumeEventType::EVENT_IS_STREAM_MUTE:
            vol = isStreamMute_;
            break;
        default:
            vol = 0;
            DHLOGE("%s: Get volume failed.", AUDIO_LOG);
    }
    param = std::to_string(vol);
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::getEventTypeFromCondition(const std::string &condition)
{
    std::string::size_type position = condition.find_first_of(";");
    int32_t type = std::stoi(condition.substr(11, position - 11));
    return (VolumeEventType)type;
}

int32_t AudioAdapterInterfaceImpl::HandleVolumeChangeEvent(const AudioEvent &event)
{
    DHLOGI("%s: Vol change (%s).", AUDIO_LOG, event.content.c_str());
    if (audioRender_ == nullptr) {
        DHLOGD("%s: Render has not been created.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    int32_t vol = AUDIO_DEFAULT_MIN_VOLUME_LEVEL;
    int32_t ret = GetAudioParamInt(event.content, VOLUME_LEVEL, vol);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get volume value failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }

    if (event.content.rfind("FIRST_VOLUME_CHANAGE", 0) == 0) {
        int32_t maxVol = AUDIO_DEFAULT_MAX_VOLUME_LEVEL;
        ret = GetAudioParamInt(event.content, MAX_VOLUME_LEVEL, maxVol);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Get max volume value failed, use defult max volume.", AUDIO_LOG);
        }
        int32_t minVol = AUDIO_DEFAULT_MIN_VOLUME_LEVEL;
        ret = GetAudioParamInt(event.content, MIN_VOLUME_LEVEL, minVol);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Get min volume value failed, use defult min volume.", AUDIO_LOG);
        }
        audioRender_->SetVolumeInner(vol);
        audioRender_->SetVolumeRangeInner(maxVol, minVol);
        return DH_SUCCESS;
    }

    audioRender_->SetVolumeInner(vol);
    if (paramCallback_ == nullptr) {
        DHLOGE("%s: Audio param observer is null.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    ret = paramCallback_->OnAudioParamNotify(AUDIO_EXT_PARAM_KEY_VOLUME, event.content, std::to_string(vol));
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify vol failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::HandleFocusChangeEvent(const AudioEvent &event)
{
    DHLOGI("%s: Focus change (%s).", AUDIO_LOG, event.content.c_str());
    if (paramCallback_ == nullptr) {
        DHLOGE("%s: Audio param observer is null.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    std::string val = "";
    int32_t ret = paramCallback_->OnAudioParamNotify(AUDIO_EXT_PARAM_KEY_FOCUS, event.content, val);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify Focus failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::HandleRenderStateChangeEvent(const AudioEvent &event)
{
    DHLOGI("%s: RenderState change (%s).", AUDIO_LOG, event.content.c_str());
    if (paramCallback_ == nullptr) {
        DHLOGE("%s: Audio param observer is null.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_NULLPTR;
    }
    std::string val = "";
    int32_t ret = paramCallback_->OnAudioParamNotify(AUDIO_EXT_PARAM_KEY_STATUS, event.content, val);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify render state failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::HandleSANotifyEvent(const AudioEvent &event)
{
    if (event.type == HDF_AUDIO_EVENT_OPEN_SPK_RESULT) {
        DHLOGI("%s: Notify event: OPEN_SPK_RESULT, event content: %s.", AUDIO_LOG, event.content.c_str());
        if (event.content == HDF_EVENT_RESULT_SUCCESS) {
            isSpkOpened_ = true;
        }
        spkNotifyFlag_ = true;
        spkWaitCond_.notify_all();
        return DH_SUCCESS;
    } else if (event.type == HDF_AUDIO_EVENT_CLOSE_SPK_RESULT) {
        DHLOGI("%s: Notify event: CLOSE_SPK_RESULT, event content: %s.", AUDIO_LOG, event.content.c_str());
        if (event.content == HDF_EVENT_RESULT_SUCCESS) {
            isSpkOpened_ = false;
        }
        spkNotifyFlag_ = true;
        spkWaitCond_.notify_all();
        return DH_SUCCESS;
    } else if (event.type == HDF_AUDIO_EVENT_OPEN_MIC_RESULT) {
        DHLOGI("%s: Notify event: OPEN_MIC_RESULT, event content: %s.", AUDIO_LOG, event.content.c_str());
        if (event.content == HDF_EVENT_RESULT_SUCCESS) {
            isMicOpened_ = true;
        }
        micNotifyFlag_ = true;
        micWaitCond_.notify_all();
        return DH_SUCCESS;
    } else if (event.type == HDF_AUDIO_EVENT_CLOSE_MIC_RESULT) {
        DHLOGI("%s: Notify event: CLOSE_MIC_RESULT, event content: %s.", AUDIO_LOG, event.content.c_str());
        if (event.content == HDF_EVENT_RESULT_SUCCESS) {
            isMicOpened_ = false;
        }
        micNotifyFlag_ = true;
        micWaitCond_.notify_all();
        return DH_SUCCESS;
    }
    return ERR_DH_AUDIO_HDF_FAIL;
}

int32_t AudioAdapterInterfaceImpl::WaitForSANotify(const AudioDeviceEvent &event)
{
    if (event == EVENT_OPEN_SPK || event == EVENT_CLOSE_SPK) {
        spkNotifyFlag_ = false;
        std::unique_lock<std::mutex> lck(spkWaitMutex_);
        auto status =
            spkWaitCond_.wait_for(lck, std::chrono::seconds(WAIT_SECONDS), [this]() { return spkNotifyFlag_; });
        if (!status) {
            DHLOGE("%s: Wait spk event: %d timeout(%d)s.", AUDIO_LOG, event, WAIT_SECONDS);
            return ERR_DH_AUDIO_HDF_FAIL;
        }
        if (event == EVENT_OPEN_SPK && isSpkOpened_ != true) {
            DHLOGE("%s: Wait open render device failed.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
        } else if (event == EVENT_CLOSE_SPK && isSpkOpened_ != false) {
            DHLOGE("%s: Wait close render device failed.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_CLOSE_DEVICE_FAIL;
        }
        return DH_SUCCESS;
    }

    if (event == EVENT_OPEN_MIC || event == EVENT_CLOSE_MIC) {
        micNotifyFlag_ = false;
        std::unique_lock<std::mutex> lck(micWaitMutex_);
        auto status =
            micWaitCond_.wait_for(lck, std::chrono::seconds(WAIT_SECONDS), [this]() { return micNotifyFlag_; });
        if (!status) {
            DHLOGE("%s: Wait mic event: %d timeout(%d)s.", AUDIO_LOG, WAIT_SECONDS);
            return ERR_DH_AUDIO_HDF_FAIL;
        }
        if (event == EVENT_OPEN_MIC && isMicOpened_ != true) {
            DHLOGE("%s: Wait open capture device failed.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_OPEN_DEVICE_FAIL;
        } else if (event == EVENT_CLOSE_MIC && isMicOpened_ != false) {
            DHLOGE("%s: Wait close capture device failed.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_CLOSE_DEVICE_FAIL;
        }
        return DH_SUCCESS;
    }
    return DH_SUCCESS;
}

int32_t AudioAdapterInterfaceImpl::HandleDeviceClosed(const AudioEvent &event)
{
    DHLOGI("%s: Handle device closed, event type: %d.", AUDIO_LOG, event.type);
    if (paramCallback_ != nullptr) {
        std::stringstream ss;
        ss << "ERR_EVENT;DEVICE_TYPE=" <<
            (event.type == HDF_AUDIO_EVENT_SPK_CLOSED ? AUDIO_DEVICE_TYPE_SPEAKER : AUDIO_DEVICE_TYPE_MIC) << ";";
        int32_t ret = paramCallback_->OnAudioParamNotify(AudioExtParamKeyHAL::AUDIO_EXT_PARAM_KEY_STATUS, ss.str(),
            std::to_string(EVENT_DEV_CLOSED));
        if (ret != DH_SUCCESS) {
            DHLOGD("%s: Notify fwk failed.", AUDIO_LOG);
        }
    }

    AudioDeviceDescriptorHAL dec;
    if (isSpkOpened_ == true && event.type == HDF_AUDIO_EVENT_SPK_CLOSED) {
        DHLOGI("%s: Render device status error, close render.", AUDIO_LOG);
        dec.pins = spkPinInUse_;
        return DestoryRender(dec);
    } else if (isMicOpened_ == true && event.type == HDF_AUDIO_EVENT_MIC_CLOSED) {
        DHLOGI("%s: Capture device status error, close capture.", AUDIO_LOG);
        dec.pins = micPinInUse_;
        return DestoryCapture(dec);
    }
    DHLOGI("%s: Handle device closed success.", AUDIO_LOG);
    return DH_SUCCESS;
}

bool AudioAdapterInterfaceImpl::isPortsNoReg()
{
    std::lock_guard<std::mutex> devLck(devMapMtx_);
    return mapAudioDevice_.empty();
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS