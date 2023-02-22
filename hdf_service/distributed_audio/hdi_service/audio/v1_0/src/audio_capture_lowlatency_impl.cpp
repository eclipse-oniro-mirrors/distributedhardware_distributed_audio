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

#include "audio_capture_lowlatency_impl.h"

#include <hdf_base.h>
#include <securec.h>
#include <unistd.h>
#include <sys/time.h>

#include "daudio_constants.h"
#include "daudio_events.h"
#include "daudio_log.h"
#include "daudio_utils.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioCaptureLowLatencyImpl"

using namespace OHOS::DistributedHardware;
namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
AudioCaptureLowLatencyImpl::AudioCaptureLowLatencyImpl(const std::string &adpName, const AudioDeviceDescriptor &desc,
    const AudioSampleAttributes &attrs, const sptr<IDAudioCallback> &callback)
    : adapterName_(adpName), devDesc_(desc), devAttrs_(attrs), audioExtCallback_(callback)
{
    devAttrs_.frameSize = CalculateFrameSize(attrs.sampleRate, attrs.channelCount, attrs.format, timeInterval_, true);
    DHLOGI("Distributed audio capture constructed, id(%d). framesize(%d)", desc.pins, devAttrs_.frameSize);
}

AudioCaptureLowLatencyImpl::~AudioCaptureLowLatencyImpl()
{
    UnInitAshmem();
    DHLOGD("Distributed audio capture destructed, id(%d).", devDesc_.pins);
}

int32_t AudioCaptureLowLatencyImpl::InitAshmem(int32_t ashmemLength)
{
    std::string memory_name = "Capture ShareMemory";
    if (ashmemLength < DAUDIO_MIN_ASHMEM_LEN || ashmemLength > DAUDIO_MAX_ASHMEM_LEN) {
        DHLOGE("Init ashmem failed. length is illegal");
        return HDF_FAILURE;
    }
    ashmem_ = OHOS::Ashmem::CreateAshmem(memory_name.c_str(), ashmemLength);
    if (ashmem_ == nullptr) {
        DHLOGE("Create ashmem failed.");
        return HDF_FAILURE;
    }
    if (!ashmem_->MapReadAndWriteAshmem()) {
        DHLOGE("Mmap ashmem failed.");
        return HDF_FAILURE;
    }
    fd_ = ashmem_->GetAshmemFd();
    DHLOGI("Init Ashmem success, fd: %d, length: %d", fd_, ashmemLength);
    return HDF_SUCCESS;
}

void AudioCaptureLowLatencyImpl::UnInitAshmem()
{
    if (ashmem_ != nullptr) {
        ashmem_->UnmapAshmem();
        ashmem_->CloseAshmem();
        ashmem_ = nullptr;
        DHLOGI("UnInitAshmem success.");
    }
}

int32_t AudioCaptureLowLatencyImpl::GetAshmemInfo(int &fd, int &ashmemLength, int &lengthPerTrans)
{
    fd = fd_;
    ashmemLength = ashmemLength_;
    lengthPerTrans = lengthPerTrans_;
    DHLOGI("Get ashmemInfo. fd: %d, ashmemLength: %d, lengthPerTrans: %d", fd, ashmemLength, lengthPerTrans);
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetCapturePosition(uint64_t &frames, AudioTimeStamp &time)
{
    DHLOGI("Get capture position, not support yet.");
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::CaptureFrame(std::vector<int8_t> &frame, uint64_t requestBytes)
{
    DHLOGI("Render frame. not support in low-latency capture");
    (void)devAttrs_.sampleRate;
    (void)devAttrs_.channelCount;
    (void)devAttrs_.format;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::Start()
{
    DHLOGI("Start capture mmap.");
    DAudioEvent event = { HDF_AUDIO_EVENT_MMAP_START_MIC, "" };
    if (audioExtCallback_ == nullptr) {
        DHLOGE("Callback is nullptr.");
        return HDF_FAILURE;
    }
    int32_t ret = audioExtCallback_->NotifyEvent(adapterName_, devDesc_.pins, event);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Start capture mmap failed.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::Stop()
{
    DHLOGI("Stop capture mmap.");
    DAudioEvent event = { HDF_AUDIO_EVENT_MMAP_STOP_MIC, "" };
    int32_t ret = audioExtCallback_->NotifyEvent(adapterName_, devDesc_.pins, event);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Stop capture mmap failed.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::Pause()
{
    DHLOGI("Pause capture.");
    std::lock_guard<std::mutex> captureLck(captureMtx_);
    captureStatus_ = CAPTURE_STATUS_PAUSE;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::Resume()
{
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::Flush()
{
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::TurnStandbyMode()
{
    DHLOGI("Turn stand by mode, not support yet.");
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::AudioDevDump(int32_t range, int32_t fd)
{
    DHLOGI("Dump audio info, not support yet.");
    (void)range;
    (void)fd;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::IsSupportsPauseAndResume(bool &supportPause, bool &supportResume)
{
    DHLOGI("Check whether pause and resume is supported, not support yet.");
    (void)supportPause;
    (void)supportResume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::CheckSceneCapability(const AudioSceneDescriptor &scene, bool &supported)
{
    DHLOGI("Check scene capability.");
    (void)scene;
    supported = false;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::SelectScene(const AudioSceneDescriptor &scene)
{
    DHLOGI("Select audio scene, not support yet.");
    (void)scene;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::SetMute(bool mute)
{
    DHLOGI("Set mute, not support yet.");
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetMute(bool &mute)
{
    DHLOGI("Get mute, not support yet.");
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::SetVolume(float volume)
{
    DHLOGI("Can not set vol not by this interface.");
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetVolume(float &volume)
{
    DHLOGI("Can not get vol not by this interface.");
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetGainThreshold(float &min, float &max)
{
    DHLOGI("Get gain threshold, not support yet.");
    min = 0;
    max = 0;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::SetGain(float gain)
{
    DHLOGI("Set gain, not support yet.");
    (void)gain;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetGain(float &gain)
{
    DHLOGI("Get gain, not support yet.");
    gain = 1.0;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetFrameSize(uint64_t &size)
{
    (void)size;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetFrameCount(uint64_t &count)
{
    (void)count;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::SetSampleAttributes(const AudioSampleAttributes &attrs)
{
    DHLOGI("Set sample attributes.");
    devAttrs_ = attrs;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetSampleAttributes(AudioSampleAttributes &attrs)
{
    DHLOGI("Get sample attributes.");
    attrs = devAttrs_;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetCurrentChannelId(uint32_t &channelId)
{
    DHLOGI("Get current channel id, not support yet.");
    (void)channelId;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::SetExtraParams(const std::string &keyValueList)
{
    DHLOGI("Set extra parameters, not support yet.");
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetExtraParams(std::string &keyValueList)
{
    DHLOGI("Get extra parameters, not support yet.");
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::ReqMmapBuffer(int32_t reqSize, AudioMmapBufferDescriptor &desc)
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
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetMmapPosition(uint64_t &frames, AudioTimeStamp &time)
{
    DHLOGI("Get mmap position, not support yet.");
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::AddAudioEffect(uint64_t effectid)
{
    DHLOGI("Add audio effect, not support yet.");
    (void)effectid;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::RemoveAudioEffect(uint64_t effectid)
{
    DHLOGI("Remove audio effect, not support yet.");
    (void)effectid;
    return HDF_SUCCESS;
}

int32_t AudioCaptureLowLatencyImpl::GetFrameBufferSize(uint64_t &bufferSize)
{
    DHLOGI("Get frame buffer size, not support yet.");
    (void)bufferSize;
    return HDF_SUCCESS;
}

const AudioDeviceDescriptor &AudioCaptureLowLatencyImpl::GetCaptureDesc()
{
    return devDesc_;
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
