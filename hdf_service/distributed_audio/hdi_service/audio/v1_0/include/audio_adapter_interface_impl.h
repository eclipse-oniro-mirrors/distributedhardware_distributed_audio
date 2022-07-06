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
    int32_t SetAudioParameters(AudioExtParamKey key, const std::string& condition, const std::string& value) override;
    int32_t GetAudioParameters(AudioExtParamKey key, const std::string& condition, std::string& value) override;
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
    bool isPortsNoReg() const;

private:
    int32_t SetAudioVolume(const std::string& condition, const std::string &param);
    int32_t GetAudioVolume(const std::string& condition, std::string &param);
    int32_t HandleVolumeChangeEvent(const AudioEvent &event);
    int32_t HandleOpenMicEvent(const AudioEvent &event);
    int32_t HandleOpenSpeakerEvent(const AudioEvent &event);

private:
    const char *AUDIO_LOG = "AudioAdapterInterfaceImpl";
    static constexpr uint8_t OPEN_WAIT_SECONDS = 3;
    AudioAdapterDescriptorHAL adpDescriptor_;
    AudioAdapterStatus status_ = STATUS_OFFLINE;

    sptr<IDAudioCallback> extSpeakerCallback_;
    sptr<IDAudioCallback> extMicCallback_;
    sptr<IAudioParamCallback> paramCallback_;
    sptr<AudioRenderInterfaceImpl> audioRender_;
    AudioParameter renderParam_;
    sptr<AudioCaptureInterfaceImpl> audioCapture_;
    AudioParameter captureParam_;

    std::mutex devMapMtx_;
    std::map<uint32_t, std::string> mapAudioDevice_;
    std::mutex spkWaitMutex_;
    std::condition_variable spkWaitCond_;
    std::mutex micWaitMutex_;
    std::condition_variable micWaitCond_;

    bool isSpkOpened_ = false;
    bool isMicOpened_ = false;
    bool spkNotifyFlag_ = false;
    bool micNotifyFlag_ = false;
};
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
#endif // OHOS_AUDIO_ADAPTER_INTERFACE_IMPL_H