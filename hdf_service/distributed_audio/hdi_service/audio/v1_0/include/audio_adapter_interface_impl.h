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

#ifndef OHOS_AUDIO_ADAPTER_INTERFACE_IMPL_H
#define OHOS_AUDIO_ADAPTER_INTERFACE_IMPL_H

#include <condition_variable>
#include <map>
#include <mutex>

#include "audio_capture_interface_impl.h"
#include "audio_render_interface_impl.h"
#include "v1_0/iaudio_adapter.h"
#include "v1_0/id_audio_manager.h"
#include "v1_0/types.h"

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
using OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioEvent;
using OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioParameter;
using OHOS::HDI::DistributedAudio::Audioext::V1_0::IDAudioCallback;

typedef enum {
    STATUS_ONLINE = 0,
    STATUS_OFFLINE,
    STATUS_LOAD,
    STATUS_UNLOAD,
    STATUS_CREATE_RENDER,
} AudioAdapterStatus;

typedef enum {
    EVENT_DEV_CLOSED = 0,
    EVENT_OPEN_SPK,
    EVENT_CLOSE_SPK,
    EVENT_OPEN_MIC,
    EVENT_CLOSE_MIC,
} AudioDeviceEvent;

typedef enum {
    EVENT_GET_VOLUME = 1,
    EVENT_GET_MIN_VOLUME = 2,
    EVENT_GET_MAX_VOLUME = 3,
    EVENT_IS_STREAM_MUTE = 4,
} VolumeEventType;

class AudioAdapterInterfaceImpl : public IAudioAdapter {
public:
    AudioAdapterInterfaceImpl(const AudioAdapterDescriptorHAL &desc);
    virtual ~AudioAdapterInterfaceImpl();

    void SetSpeakerCallback(const sptr<IDAudioCallback> &speakerCallback);
    void SetMicCallback(const sptr<IDAudioCallback> &micCallback);
    int32_t InitAllPorts() override;
    int32_t CreateRender(const AudioDeviceDescriptorHAL &desc, const AudioSampleAttributesHAL &attrs,
        sptr<IAudioRender> &render) override;
    int32_t DestoryRender(const AudioDeviceDescriptorHAL &desc) override;
    int32_t CreateCapture(const AudioDeviceDescriptorHAL &desc, const AudioSampleAttributesHAL &attrs,
        sptr<IAudioCapture> &capture) override;
    int32_t DestoryCapture(const AudioDeviceDescriptorHAL &desc) override;
    int32_t GetPortCapability(const AudioPortHAL &port, AudioPortCapabilityHAl &capability) override;
    int32_t SetPassthroughMode(const AudioPortHAL &port, AudioPortPassthroughModeHAL mode) override;
    int32_t GetPassthroughMode(const AudioPortHAL &port, AudioPortPassthroughModeHAL &mode) override;
    int32_t UpdateAudioRoute(const AudioRouteHAL &route, int32_t &handle) override;
    int32_t ReleaseAudioRoute(int32_t handle) override;
    int32_t SetAudioParameters(AudioExtParamKeyHAL key, const std::string &condition,
        const std::string &value) override;
    int32_t GetAudioParameters(AudioExtParamKeyHAL key, const std::string &condition, std::string &value) override;
    int32_t RegAudioParamObserver(const sptr<IAudioParamCallback> &cbObj) override;

public:
    AudioAdapterDescriptorHAL GetAdapterDesc();
    std::string GetDeviceCapabilitys(const uint32_t devId);
    int32_t AdapterLoad();
    int32_t AdapterUnload();
    int32_t Notify(const uint32_t devId, const AudioEvent &event);
    int32_t AddAudioDevice(const uint32_t devId, const std::string &caps);
    int32_t RemoveAudioDevice(const uint32_t devId);
    int32_t OpenRenderDevice(const AudioDeviceDescriptorHAL &desc, const AudioSampleAttributesHAL &attrs);
    int32_t CloseRenderDevice(const AudioDeviceDescriptorHAL &desc);
    int32_t OpenCaptureDevice(const AudioDeviceDescriptorHAL &desc, const AudioSampleAttributesHAL &attrs);
    int32_t CloseCaptureDevice(const AudioDeviceDescriptorHAL &desc);
    uint32_t GetVolumeGroup(const uint32_t devId);
    uint32_t GetInterruptGroup(const uint32_t devId);
    bool isPortsNoReg();

private:
    int32_t SetAudioVolume(const std::string& condition, const std::string &param);
    int32_t GetAudioVolume(const std::string& condition, std::string &param);
    int32_t HandleFocusChangeEvent(const AudioEvent &event);
    int32_t HandleRenderStateChangeEvent(const AudioEvent &event);
    int32_t HandleVolumeChangeEvent(const AudioEvent &event);
    int32_t HandleSANotifyEvent(const AudioEvent &event);
    int32_t WaitForSANotify(const AudioDeviceEvent &event);
    int32_t HandleDeviceClosed(const AudioEvent &event);
    int32_t getEventTypeFromCondition(const std::string& condition);

private:
    const char *AUDIO_LOG = "AudioAdapterInterfaceImpl";
    static constexpr uint8_t WAIT_SECONDS = 10;
    AudioAdapterDescriptorHAL adpDescriptor_;
    AudioAdapterStatus status_ = STATUS_OFFLINE;

    sptr<IDAudioCallback> extSpeakerCallback_ = nullptr;
    sptr<IDAudioCallback> extMicCallback_ = nullptr;
    sptr<IAudioParamCallback> paramCallback_ = nullptr;
    sptr<AudioRenderInterfaceImpl> audioRender_ = nullptr;
    AudioParameter renderParam_;
    sptr<AudioCaptureInterfaceImpl> audioCapture_ = nullptr;
    AudioParameter captureParam_;

    std::mutex devMapMtx_;
    std::mutex captureOptMtx_;
    std::mutex renderOptMtx_;
    std::map<uint32_t, std::string> mapAudioDevice_;
    std::mutex spkWaitMutex_;
    std::condition_variable spkWaitCond_;
    std::mutex micWaitMutex_;
    std::condition_variable micWaitCond_;

    bool isSpkOpened_ = false;
    bool isMicOpened_ = false;
    bool spkNotifyFlag_ = false;
    bool micNotifyFlag_ = false;

    uint32_t spkPinInUse_ = 0;
    uint32_t micPinInUse_ = 0;
    uint32_t streamMuteStatus_ = 0;

    const std::string NOT_MUTE_STATUS = "0";
    const std::string IS_MUTE_STATUS = "1";
};
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
#endif // OHOS_AUDIO_ADAPTER_INTERFACE_IMPL_H