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

#ifndef OHOS_AUDIO_RENDER_INTERFACE_IMPL_H
#define OHOS_AUDIO_RENDER_INTERFACE_IMPL_H

#include <mutex>
#include <string>

#include "v1_0/iaudio_render.h"
#include "v1_0/id_audio_manager.h"

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
using OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioData;
using OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioParameter;
using OHOS::HDI::DistributedAudio::Audioext::V1_0::IDAudioCallback;

typedef enum {
    RENDER_STATUS_OPEN = 0,
    RENDER_STATUS_CLOSE,
    RENDER_STATUS_START,
    RENDER_STATUS_STOP,
    RENDER_STATUS_PAUSE,
} AudioRenderStatus;

class AudioRenderInterfaceImpl : public IAudioRender {
public:
    AudioRenderInterfaceImpl(const std::string adpName, const AudioDeviceDescriptorHAL &desc,
        const AudioSampleAttributesHAL &attrs, const sptr<IDAudioCallback> &callback);

    virtual ~AudioRenderInterfaceImpl();

    int32_t GetLatency(uint32_t &ms) override;
    int32_t RenderFrame(const std::vector<uint8_t> &frame, uint64_t requestBytes, uint64_t &replyBytes) override;
    int32_t GetRenderPosition(uint64_t &frames, AudioTimeStampHAL &time) override;
    int32_t SetRenderSpeed(float speed) override;
    int32_t GetRenderSpeed(float &speed) override;
    int32_t SetChannelMode(AudioChannelModeHAL mode) override;
    int32_t GetChannelMode(AudioChannelModeHAL &mode) override;
    int32_t RegCallback(const sptr<IAudioRenderCallback> &cbObj) override;
    int32_t DrainBuffer(AudioDrainNotifyTypeHAL type) override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Pause() override;
    int32_t Resume() override;
    int32_t Flush() override;
    int32_t TurnStandbyMode() override;
    int32_t AudioDevDump(int32_t range, int32_t fd) override;
    int32_t CheckSceneCapability(const AudioSceneDescriptorHAL &scene, bool &support) override;
    int32_t SelectScene(const AudioSceneDescriptorHAL &scene) override;
    int32_t SetMute(bool mute) override;
    int32_t GetMute(bool &mute) override;
    int32_t SetVolume(float volume) override;
    int32_t GetVolume(float &volume) override;
    int32_t GetGainThreshold(float &min, float &max) override;
    int32_t SetGain(float gain) override;
    int32_t GetGain(float &gain) override;
    int32_t GetFrameSize(uint64_t &size) override;
    int32_t GetFrameCount(uint64_t &count) override;
    int32_t SetSampleAttributes(const AudioSampleAttributesHAL &attrs) override;
    int32_t GetSampleAttributes(AudioSampleAttributesHAL &attrs) override;
    int32_t GetCurrentChannelId(uint32_t &channelId) override;
    int32_t SetExtraParams(const std::string &keyValueList) override;
    int32_t GetExtraParams(std::string &keyValueList) override;
    int32_t ReqMmapBuffer(int32_t reqSize, AudioMmapBufferDescripterHAL &desc) override;
    int32_t GetMmapPosition(uint64_t &frames, AudioTimeStampHAL &time) override;

    const AudioDeviceDescriptorHAL &GetRenderDesc();
    void SetVolumeInner(const uint32_t vol);
    void SetVolumeRangeInner(const uint32_t maxVol, const uint32_t minVol);
    uint32_t GetVolumeInner();
    uint32_t GetMaxVolumeInner();
    uint32_t GetMinVolumeInner();

private:
    const char *AUDIO_LOG = "AudioRenderInterfaceImpl";
    std::string adapterName_;
    AudioDeviceDescriptorHAL devDesc_;
    AudioSampleAttributesHAL devAttrs_;

    float renderSpeed_ = 0;
    std::mutex volMtx_;
    uint32_t vol_ = 0;
    uint32_t volMax_ = 0;
    uint32_t volMin_ = 0;
    std::mutex renderMtx_;
    AudioChannelModeHAL channelMode_ = AUDIO_CHANNEL_NORMAL;
    AudioRenderStatus renderStatus_ = RENDER_STATUS_CLOSE;
    sptr<IDAudioCallback> audioExtCallback_ = nullptr;
    sptr<IAudioRenderCallback> renderCallback_ = nullptr;
};
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
#endif // OHOS_AUDIO_RENDER_INTERFACE_IMPL_H