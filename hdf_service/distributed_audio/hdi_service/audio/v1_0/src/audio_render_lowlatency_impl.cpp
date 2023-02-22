/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_render_lowlatency_impl.h"

#include <hdf_base.h>
#include <unistd.h>
#include "sys/time.h"

#include "daudio_constants.h"
#include "daudio_events.h"
#include "daudio_log.h"
#include "daudio_utils.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioRenderLowLatencyImpl"

using namespace OHOS::DistributedHardware;
namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
AudioRenderLowLatencyImpl::AudioRenderLowLatencyImpl(const std::string &adpName, const AudioDeviceDescriptor &desc,
    const AudioSampleAttributes &attrs, const sptr<IDAudioCallback> &callback)
    : adapterName_(adpName), devDesc_(desc), devAttrs_(attrs), audioExtCallback_(callback)
{
    devAttrs_.frameSize = CalculateFrameSize(attrs.sampleRate, attrs.channelCount, attrs.format, timeInterval_, true);
    DHLOGI("Distributed audio render constructed, id(%d). framesize(%d)", desc.pins, devAttrs_.frameSize);
}

AudioRenderLowLatencyImpl::~AudioRenderLowLatencyImpl()
{
    UnInitAshmem();
    DHLOGI("Distributed audio render destructed, id(%d).", devDesc_.pins);
}

int32_t AudioRenderLowLatencyImpl::InitAshmem(int32_t ashmemLength)
{
    std::string memory_name = "Render ShareMemory";
    if (ashmemLength < DAUDIO_MIN_ASHMEM_LEN || ashmemLength > DAUDIO_MAX_ASHMEM_LEN) {
        DHLOGE("Init ashmem failed. length is illegal");
        return HDF_FAILURE;
    }
    ashmem_ = OHOS::Ashmem::CreateAshmem(memory_name.c_str(), ashmemLength);
    if (ashmem_ == nullptr) {
        DHLOGE("Create ashmem failed.");
        return HDF_FAILURE;
    }
    bool ret = ashmem_->MapReadAndWriteAshmem();
    if (ret != true) {
        DHLOGE("Mmap ashmem failed.");
        return HDF_FAILURE;
    }
    fd_ = ashmem_->GetAshmemFd();
    DHLOGI("Init Ashmem success, fd: %d, length: %d", fd_, ashmemLength);
    return HDF_SUCCESS;
}

void AudioRenderLowLatencyImpl::UnInitAshmem()
{
    if (ashmem_ != nullptr) {
        ashmem_->UnmapAshmem();
        ashmem_->CloseAshmem();
        ashmem_ = nullptr;
        DHLOGI("UnInitAshmem success.");
    }
}

int32_t AudioRenderLowLatencyImpl::GetAshmemInfo(int &fd, int &ashmemLength, int &lengthPerTrans)
{
    fd = fd_;
    ashmemLength = ashmemLength_;
    lengthPerTrans = lengthPerTrans_;
    DHLOGI("Get ashmemInfo. fd: %d, ashmemLength: %d, lengthPerTrans: %d", fd, ashmemLength, lengthPerTrans);
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetLatency(uint32_t &ms)
{
    DHLOGI("Get render device latency, not support yet.");
    ms = 0;
    return HDF_SUCCESS;
}

float AudioRenderLowLatencyImpl::GetFadeRate(uint32_t currentIndex, const uint32_t durationIndex)
{
    if (currentIndex > durationIndex) {
        return 1.0f;
    }

    float fadeRate = static_cast<float>(currentIndex) / durationIndex * DAUDIO_FADE_NORMALIZATION_FACTOR;
    if (fadeRate < 1) {
        return pow(fadeRate, DAUDIO_FADE_POWER_NUM) / DAUDIO_FADE_NORMALIZATION_FACTOR;
    }
    return -pow(fadeRate - DAUDIO_FADE_MAXIMUM_VALUE, DAUDIO_FADE_POWER_NUM) /
        DAUDIO_FADE_NORMALIZATION_FACTOR + 1;
}

int32_t AudioRenderLowLatencyImpl::FadeInProcess(const uint32_t durationFrame,
    int8_t* frameData, const size_t frameLength)
{
    int16_t* frame = reinterpret_cast<int16_t *>(frameData);
    const size_t newFrameLength = frameLength / 2;

    for (size_t k = 0; k < newFrameLength; ++k) {
        float rate = GetFadeRate(currentFrame_ * newFrameLength + k, durationFrame * newFrameLength);
        frame[k] = currentFrame_ == durationFrame - 1 ? frame[k] : static_cast<int16_t>(rate * frame[k]);
    }
    DHLOGI("Fade-in frame[currentFrame: %d].", currentFrame_);
    ++currentFrame_;
    currentFrame_ = currentFrame_ >= durationFrame ? durationFrame - 1 : currentFrame_;

    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::RenderFrame(const std::vector<int8_t> &frame, uint64_t &replyBytes)
{
    DHLOGI("Render frame. not support in low-latency render");
    (void)devAttrs_.sampleRate;
    (void)devAttrs_.channelCount;
    (void)devAttrs_.format;

    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetRenderPosition(uint64_t &frames, AudioTimeStamp &time)
{
    DHLOGI("Get render position, not support yet.");
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SetRenderSpeed(float speed)
{
    DHLOGI("Set render speed, control render speed is not support yet.");
    renderSpeed_ = speed;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetRenderSpeed(float &speed)
{
    DHLOGI("Get render speed, control render speed is not support yet.");
    speed = renderSpeed_;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SetChannelMode(AudioChannelMode mode)
{
    DHLOGI("Set channel mode, control channel mode is not support yet.");
    channelMode_ = mode;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetChannelMode(AudioChannelMode &mode)
{
    DHLOGI("Get channel mode, control channel mode is not support yet.");
    mode = channelMode_;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::RegCallback(const sptr<IAudioCallback> &audioCallback, int8_t cookie)
{
    DHLOGI("Register render callback.");
    (void)cookie;
    renderCallback_ = audioCallback;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::DrainBuffer(AudioDrainNotifyType &type)
{
    DHLOGI("Drain audio buffer, not support yet.");
    (void)type;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::IsSupportsDrain(bool &support)
{
    DHLOGI("Check whether drain is supported, not support yet.");
    (void)support;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::Start()
{
    DHLOGI("Start render mmap.");
    DAudioEvent event = { HDF_AUDIO_EVENT_MMAP_START, "" };
    if (audioExtCallback_ == nullptr) {
        DHLOGE("Callback is nullptr.");
        return HDF_FAILURE;
    }
    int32_t ret = audioExtCallback_->NotifyEvent(adapterName_, devDesc_.pins, event);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Start render mmap failed.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::Stop()
{
    DHLOGI("Stop render mmap.");
    DAudioEvent event = { HDF_AUDIO_EVENT_MMAP_STOP, "" };
    int32_t ret = audioExtCallback_->NotifyEvent(adapterName_, devDesc_.pins, event);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Stop render mmap failed.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::Pause()
{
    DHLOGI("Pause render.");
    std::lock_guard<std::mutex> renderLck(renderMtx_);
    renderStatus_ = RENDER_STATUS_PAUSE;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::Resume()
{
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::Flush()
{
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::TurnStandbyMode()
{
    DHLOGI("Turn stand by mode, not support yet.");
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::AudioDevDump(int32_t range, int32_t fd)
{
    DHLOGI("Dump audio info, not support yet.");
    (void)range;
    (void)fd;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::IsSupportsPauseAndResume(bool &supportPause, bool &supportResume)
{
    DHLOGI("Check whether pause and resume is supported, not support yet.");
    (void)supportPause;
    (void)supportResume;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::CheckSceneCapability(const AudioSceneDescriptor &scene, bool &supported)
{
    DHLOGI("Check scene capability.");
    (void)scene;
    (void)supported;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SelectScene(const AudioSceneDescriptor &scene)
{
    DHLOGI("Select audio scene, not support yet.");
    (void)scene;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SetMute(bool mute)
{
    DHLOGI("Set mute, not support yet.");
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetMute(bool &mute)
{
    DHLOGI("Get mute, not support yet.");
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SetVolume(float volume)
{
    DHLOGI("Can not set vol not by this interface.");
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetVolume(float &volume)
{
    DHLOGI("Can not get vol not by this interface.");
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetGainThreshold(float &min, float &max)
{
    DHLOGI("Get gain threshold, not support yet.");
    min = 0;
    max = 0;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SetGain(float gain)
{
    DHLOGI("Set gain, not support yet.");
    (void)gain;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetGain(float &gain)
{
    DHLOGI("Get gain, not support yet.");
    gain = 1.0;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetFrameSize(uint64_t &size)
{
    (void)size;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetFrameCount(uint64_t &count)
{
    (void)count;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SetSampleAttributes(const AudioSampleAttributes &attrs)
{
    DHLOGI("Set sample attributes.");
    devAttrs_ = attrs;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetSampleAttributes(AudioSampleAttributes &attrs)
{
    DHLOGI("Get sample attributes.");
    attrs = devAttrs_;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetCurrentChannelId(uint32_t &channelId)
{
    DHLOGI("Get current channel id, not support yet.");
    (void)channelId;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::SetExtraParams(const std::string &keyValueList)
{
    DHLOGI("Set extra parameters, not support yet.");
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetExtraParams(std::string &keyValueList)
{
    DHLOGI("Get extra parameters, not support yet.");
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::ReqMmapBuffer(int32_t reqSize, AudioMmapBufferDescriptor &desc)
{
    DHLOGI("Request mmap buffer.");
    int32_t minSize = CalculateSampleNum(devAttrs_.sampleRate, minTimeInterval_);
    int32_t maxSize = CalculateSampleNum(devAttrs_.sampleRate, maxTimeInterval_);
    int32_t realSize = reqSize;
    if (reqSize < minSize) {
        realSize = minSize;
    } else if (reqSize > maxSize) {
        realSize = maxSize;
    }
    DHLOGI("ReqMmap buffer realsize : %d, minsize: %d, maxsize:%d.", realSize, minSize, maxSize);
    desc.totalBufferFrames = realSize;
    ashmemLength_ = realSize * devAttrs_.channelCount * devAttrs_.format;
    DHLOGI("Init ashmem real sample size : %d, length: %d.", realSize, ashmemLength_);
    int32_t ret = InitAshmem(ashmemLength_);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Init ashmem error..");
        return HDF_FAILURE;
    }
    desc.memoryFd = fd_;
    desc.transferFrameSize = CalculateSampleNum(devAttrs_.sampleRate, timeInterval_);
    lengthPerTrans_ = desc.transferFrameSize * devAttrs_.channelCount * devAttrs_.format;
    desc.isShareable = false;
    ret = audioExtCallback_->RefreshAshmemInfo(adapterName_, devDesc_.pins, fd_, ashmemLength_, lengthPerTrans_);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Refresh ashmem info failed.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetMmapPosition(uint64_t &frames, AudioTimeStamp &time)
{
    DHLOGI("Get mmap position.");
    (void)time;
    if (audioExtCallback_ == nullptr) {
        DHLOGE("Callback is nullptr.");
        return HDF_FAILURE;
    }
    uint64_t timeStamp;
    int32_t ret = audioExtCallback_->ReadMmapPosition(adapterName_, devDesc_.pins, frames, timeStamp);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Read mmap position failed.");
        return HDF_FAILURE;
    }
    DHLOGI("Read mmap position. frames: %d, timeStamp: %d", frames, timeStamp);
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::AddAudioEffect(uint64_t effectid)
{
    DHLOGI("Add audio effect, not support yet.");
    (void)effectid;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::RemoveAudioEffect(uint64_t effectid)
{
    DHLOGI("Remove audio effect, not support yet.");
    (void)effectid;
    return HDF_SUCCESS;
}

int32_t AudioRenderLowLatencyImpl::GetFrameBufferSize(uint64_t &bufferSize)
{
    DHLOGI("Get frame buffer size, not support yet.");
    (void)bufferSize;
    return HDF_SUCCESS;
}

const AudioDeviceDescriptor &AudioRenderLowLatencyImpl::GetRenderDesc()
{
    return devDesc_;
}

void AudioRenderLowLatencyImpl::SetVolumeInner(const uint32_t vol)
{
    std::lock_guard<std::mutex> volLck(volMtx_);
    vol_ = vol;
}


void AudioRenderLowLatencyImpl::SetVolumeRangeInner(const uint32_t volMax, const uint32_t volMin)
{
    std::lock_guard<std::mutex> volLck(volMtx_);
    volMin_ = volMin;
    volMax_ = volMax;
}

uint32_t AudioRenderLowLatencyImpl::GetVolumeInner()
{
    std::lock_guard<std::mutex> volLck(volMtx_);
    return vol_;
}

uint32_t AudioRenderLowLatencyImpl::GetMaxVolumeInner()
{
    std::lock_guard<std::mutex> volLck(volMtx_);
    return volMax_;
}

uint32_t AudioRenderLowLatencyImpl::GetMinVolumeInner()
{
    std::lock_guard<std::mutex> volLck(volMtx_);
    return volMin_;
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
