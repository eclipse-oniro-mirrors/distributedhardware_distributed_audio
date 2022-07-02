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

#include "audio_render_interface_impl.h"

#include <hdf_base.h>
#include <unistd.h>
#include "sys/time.h"

#include "daudio_constants.h"
#include "daudio_log.h"

using namespace OHOS::DistributedHardware;
namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
AudioRenderInterfaceImpl::AudioRenderInterfaceImpl(const std::string adpName, const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs, const sptr<IDAudioCallback> &callback)
    : adapterName_(adpName), devDesc_(desc), devAttrs_(attrs), audioExtCallback_(callback)
{
    DHLOGD("%s: Distributed Audio Render constructed, id(%d).", AUDIO_LOG, desc.pins);
}

AudioRenderInterfaceImpl::~AudioRenderInterfaceImpl()
{
    DHLOGD("%s: Distributed Audio Render destructed, id(%d).", AUDIO_LOG, devDesc_.pins);
}

int32_t AudioRenderInterfaceImpl::GetLatency(uint32_t &ms)
{
    DHLOGI("%s: Get render device latency, not support yet.", AUDIO_LOG);
    ms = 0;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::RenderFrame(const std::vector<uint8_t> &frame, uint64_t requestBytes,
    uint64_t &replyBytes)
{
    DHLOGI("%s: Render frame[samplerate: %d, channelmask: %d, bitformat: %d].", AUDIO_LOG, devAttrs_.sampleRate,
        devAttrs_.channelCount, devAttrs_.format);
    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    std::lock_guard<std::mutex> renderLck(renderMtx_);
    if (renderStatus_ != RENDER_STATUS_START) {
        DHLOGE("%s: Render status wrong, return false.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    AudioParameter param = { devAttrs_.format, devAttrs_.channelCount, devAttrs_.sampleRate };
    AudioData data = { param, frame };
    int32_t ret = audioExtCallback_->WriteStreamData(adapterName_, devDesc_.pins, data);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Write stream data failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    WriteStreamWait(startTime);
    DHLOGI("%s: Render audio frame success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetRenderPosition(uint64_t &frames, AudioTimeStampHAL &time)
{
    DHLOGI("%s: Get render position, not support yet.", AUDIO_LOG);
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SetRenderSpeed(float speed)
{
    DHLOGI("%s: Set render speed, control render speed is not support yet.", AUDIO_LOG);
    renderSpeed_ = speed;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetRenderSpeed(float &speed)
{
    DHLOGI("%s: Get render speed, control render speed is not support yet.", AUDIO_LOG);
    speed = renderSpeed_;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SetChannelMode(AudioChannelModeHAL mode)
{
    DHLOGI("%s: Set channel mode, control channel mode is not support yet.", AUDIO_LOG);
    channelMode_ = mode;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetChannelMode(AudioChannelModeHAL &mode)
{
    DHLOGI("%s: Get channel mode, control channel mode is not support yet.", AUDIO_LOG);
    mode = channelMode_;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::RegCallback(const sptr<IAudioRenderCallback> &cbObj)
{
    DHLOGI("%s: Register render callback.", AUDIO_LOG);
    renderCallback_ = cbObj;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::DrainBuffer(AudioDrainNotifyTypeHAL type)
{
    DHLOGI("%s: Drain audio buffer, not support yet.", AUDIO_LOG);
    (void)type;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::Start()
{
    DHLOGI("%s: Start render.", AUDIO_LOG);
    std::lock_guard<std::mutex> renderLck(renderMtx_);
    renderStatus_ = RENDER_STATUS_START;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::Stop()
{
    DHLOGI("%s: Stop render.", AUDIO_LOG);
    std::lock_guard<std::mutex> renderLck(renderMtx_);
    renderStatus_ = RENDER_STATUS_STOP;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::Pause()
{
    DHLOGI("%s: Pause render.", AUDIO_LOG);
    std::lock_guard<std::mutex> renderLck(renderMtx_);
    renderStatus_ = RENDER_STATUS_PAUSE;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::Resume()
{
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::Flush()
{
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::TurnStandbyMode()
{
    DHLOGI("%s: Turn stand by mode, not support yet.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::AudioDevDump(int32_t range, int32_t fd)
{
    DHLOGI("%s: Dump audio info, not support yet.", AUDIO_LOG);
    (void)range;
    (void)fd;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::CheckSceneCapability(const AudioSceneDescriptorHAL &scene, bool &support)
{
    DHLOGI("%s: Check scene capability.", AUDIO_LOG);
    (void)scene;
    (void)support;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SelectScene(const AudioSceneDescriptorHAL &scene)
{
    DHLOGI("%s: Select audio scene, not support yet.", AUDIO_LOG);
    (void)scene;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SetMute(bool mute)
{
    DHLOGI("%s: Set mute, not support yet.", AUDIO_LOG);
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetMute(bool &mute)
{
    DHLOGI("%s: Get mute, not support yet.", AUDIO_LOG);
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SetVolume(float volume)
{
    DHLOGI("%s: Can not set vol not by this interface.", AUDIO_LOG);
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetVolume(float &volume)
{
    DHLOGI("%s: Can not get vol not by this interface.", AUDIO_LOG);
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetGainThreshold(float &min, float &max)
{
    DHLOGI("%s: Get gain threshold, not support yet.", AUDIO_LOG);
    min = 0;
    max = 0;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SetGain(float gain)
{
    DHLOGI("%s: Set gain, not support yet.", AUDIO_LOG);
    (void)gain;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetGain(float &gain)
{
    DHLOGI("%s: Get gain, not support yet.", AUDIO_LOG);
    gain = 1.0;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetFrameSize(uint64_t &size)
{
    (void)size;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetFrameCount(uint64_t &count)
{
    (void)count;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SetSampleAttributes(const AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Set sample attributes.", AUDIO_LOG);
    devAttrs_ = attrs;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetSampleAttributes(AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Get sample attributes.", AUDIO_LOG);
    attrs = devAttrs_;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetCurrentChannelId(uint32_t &channelId)
{
    DHLOGI("%s: Get current channel id, not support yet.", AUDIO_LOG);
    (void)channelId;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::SetExtraParams(const std::string &keyValueList)
{
    DHLOGI("%s: Set extra parameters, not support yet.", AUDIO_LOG);
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetExtraParams(std::string &keyValueList)
{
    DHLOGI("%s: Get extra parameters, not support yet.", AUDIO_LOG);
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::ReqMmapBuffer(int32_t reqSize, AudioMmapBufferDescripterHAL &desc)
{
    DHLOGI("%s: Request mmap buffer, not support yet.", AUDIO_LOG);
    (void)reqSize;
    (void)desc;
    return HDF_SUCCESS;
}

int32_t AudioRenderInterfaceImpl::GetMmapPosition(uint64_t &frames, AudioTimeStampHAL &time)
{
    DHLOGI("%s: Get mmap position, not support yet.", AUDIO_LOG);
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

const AudioDeviceDescriptorHAL &AudioRenderInterfaceImpl::GetRenderDesc()
{
    return devDesc_;
}

void AudioRenderInterfaceImpl::WriteStreamWait(const struct timeval &startTime)
{
    struct timeval endTime;
    gettimeofday(&endTime, NULL);
    int32_t passTime =
        (endTime.tv_sec - startTime.tv_sec) * MILLISECOND_PER_SECOND + endTime.tv_usec - startTime.tv_usec;
    if (passTime > AUDIO_FRAME_TIME_INTERFAL_DEFAULT) {
        DHLOGD("%s: Write stream data use more than %d ms.", AUDIO_LOG, AUDIO_FRAME_TIME_INTERFAL_DEFAULT);
        return;
    } else {
        int32_t remainTime = AUDIO_FRAME_TIME_INTERFAL_DEFAULT - passTime;
        usleep(remainTime);
    }

    return;
}

void AudioRenderInterfaceImpl::SetVolumeInner(const uint32_t vol)
{
    std::lock_guard<std::mutex> volLck(volMtx_);
    vol_ = vol;
}

uint32_t AudioRenderInterfaceImpl::GetVolumeInner()
{
    std::lock_guard<std::mutex> volLck(volMtx_);
    return vol_;
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
