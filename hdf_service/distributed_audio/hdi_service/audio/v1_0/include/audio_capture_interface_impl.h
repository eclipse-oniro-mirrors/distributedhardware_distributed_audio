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

#ifndef OHOS_AUDIO_CAPTURE_INTERFACE_IMPL_H
#define OHOS_AUDIO_CAPTURE_INTERFACE_IMPL_H

#include <mutex>
#include <string>

#include "v1_0/iaudio_capture.h"
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
    CAPTURE_STATUS_OPEN = 0,
    CAPTURE_STATUS_CLOSE,
    CAPTURE_STATUS_START,
    CAPTURE_STATUS_STOP,
    CAPTURE_STATUS_PAUSE,
} AudioCaptureStatus;

class AudioCaptureInterfaceImpl : public IAudioCapture {
public:
    AudioCaptureInterfaceImpl(const std::string adpName, const AudioDeviceDescriptorHAL &desc,
        const AudioSampleAttributesHAL &attrs, const sptr<IDAudioCallback> &callback);

    virtual ~AudioCaptureInterfaceImpl();

    int32_t GetCapturePosition(uint64_t &frames, AudioTimeStampHAL &time) override;
    int32_t CaptureFrame(std::vector<uint8_t> &frame, uint64_t requestBytes, uint64_t &replyBytes) override;
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
    const AudioDeviceDescriptorHAL &GetCaptureDesc();
    void ReadStreamWait(const struct timeval &startTime);

private:
    const char *AUDIO_LOG = "AudioCaptureInterfaceImpl";
    std::string adapterName_;
    AudioDeviceDescriptorHAL devDesc_;
    AudioSampleAttributesHAL devAttrs_;

    std::mutex captureMtx_;
    AudioCaptureStatus captureStatus_ = CAPTURE_STATUS_CLOSE;
    sptr<IDAudioCallback> audioExtCallback_ = nullptr;
};
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
#endif // OHOS_AUDIO_CAPTURE_INTERFACE_IMPL_H