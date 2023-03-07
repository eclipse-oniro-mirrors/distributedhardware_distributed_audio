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

#include "audio_capture_interface_impl.h"

#include <hdf_base.h>
#include <unistd.h>
#include "sys/time.h"
#include <securec.h>

#include "daudio_constants.h"
#include "daudio_log.h"
#include "daudio_utils.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioCaptureInterfaceImpl"

using namespace OHOS::DistributedHardware;
namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
AudioCaptureInterfaceImpl::AudioCaptureInterfaceImpl(const std::string &adpName, const AudioDeviceDescriptor &desc,
    const AudioSampleAttributes &attrs, const sptr<IDAudioCallback> &callback)
    : AudioCaptureInterfaceImplBase(desc), adapterName_(adpName), devDesc_(desc),
    devAttrs_(attrs), audioExtCallback_(callback)
{
    devAttrs_.frameSize = CalculateFrameSize(attrs.sampleRate, attrs.channelCount, attrs.format, timeInterval_, false);
    DHLOGD("Distributed audio capture constructed, id(%d).", desc.pins);
}

AudioCaptureInterfaceImpl::~AudioCaptureInterfaceImpl()
{
    DHLOGD("Distributed audio capture destructed, id(%d).", devDesc_.pins);
}

int32_t AudioCaptureInterfaceImpl::GetCapturePosition(uint64_t &frames, AudioTimeStamp &time)
{
    DHLOGI("Get capture position, not support yet.");
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::CaptureFrame(std::vector<int8_t> &frame, uint64_t requestBytes)
{
    DHLOGI("Capture frame[sampleRate: %d, channelCount: %d, format: %d, frameSize: %d].", devAttrs_.sampleRate,
        devAttrs_.channelCount, devAttrs_.format, devAttrs_.frameSize);

    std::lock_guard<std::mutex> captureLck(captureMtx_);
    if (captureStatus_ != CAPTURE_STATUS_START) {
        DHLOGE("Capture status wrong, return false.");
        return HDF_FAILURE;
    }

    AudioData audioData;
    if (audioExtCallback_ == nullptr) {
        DHLOGE("Callback is nullptr.");
        return HDF_FAILURE;
    }
    int32_t ret = audioExtCallback_->ReadStreamData(adapterName_, devDesc_.pins, audioData);
    if (ret != HDF_SUCCESS) {
        DHLOGE("Read stream data failed.");
        return HDF_FAILURE;
    }

    frame.resize(devAttrs_.frameSize);
    ret = memcpy_s(frame.data(), frame.size(), audioData.data.data(), audioData.data.size());
    if (ret != EOK) {
        DHLOGE("Copy capture frame failed, error code %d.", ret);
        return HDF_FAILURE;
    }

    DHLOGI("Capture audio frame success.");
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Start()
{
    DHLOGI("Start capture.");
    std::lock_guard<std::mutex> captureLck(captureMtx_);
    captureStatus_ = CAPTURE_STATUS_START;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Stop()
{
    DHLOGI("Stop capture.");
    std::lock_guard<std::mutex> captureLck(captureMtx_);
    captureStatus_ = CAPTURE_STATUS_STOP;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Pause()
{
    DHLOGI("Pause capture.");
    std::lock_guard<std::mutex> captureLck(captureMtx_);
    captureStatus_ = CAPTURE_STATUS_PAUSE;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Resume()
{
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Flush()
{
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::TurnStandbyMode()
{
    DHLOGI("Turn stand by mode, not support yet.");
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::AudioDevDump(int32_t range, int32_t fd)
{
    DHLOGI("Dump audio info, not support yet.");
    (void)range;
    (void)fd;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::IsSupportsPauseAndResume(bool &supportPause, bool &supportResume)
{
    DHLOGI("Check whether pause and resume is supported, not support yet.");
    (void)supportPause;
    (void)supportResume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::CheckSceneCapability(const AudioSceneDescriptor &scene, bool &supported)
{
    DHLOGI("Check scene capability.");
    (void)scene;
    supported = false;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SelectScene(const AudioSceneDescriptor &scene)
{
    DHLOGI("Select audio scene, not support yet.");
    (void)scene;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetMute(bool mute)
{
    DHLOGI("Set mute, not support yet.");
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetMute(bool &mute)
{
    DHLOGI("Get mute, not support yet.");
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetVolume(float volume)
{
    DHLOGI("Can not set vol not by this interface.");
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetVolume(float &volume)
{
    DHLOGI("Can not get vol not by this interface.");
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetGainThreshold(float &min, float &max)
{
    DHLOGI("Get gain threshold, not support yet.");
    min = 0;
    max = 0;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetGain(float gain)
{
    DHLOGI("Set gain, not support yet.");
    (void)gain;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetGain(float &gain)
{
    DHLOGI("Get gain, not support yet.");
    gain = 1.0;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetFrameSize(uint64_t &size)
{
    (void)size;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetFrameCount(uint64_t &count)
{
    (void)count;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetSampleAttributes(const AudioSampleAttributes &attrs)
{
    DHLOGI("Set sample attributes.");
    devAttrs_ = attrs;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetSampleAttributes(AudioSampleAttributes &attrs)
{
    DHLOGI("Get sample attributes.");
    attrs = devAttrs_;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetCurrentChannelId(uint32_t &channelId)
{
    DHLOGI("Get current channel id, not support yet.");
    (void)channelId;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetExtraParams(const std::string &keyValueList)
{
    DHLOGI("Set extra parameters, not support yet.");
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetExtraParams(std::string &keyValueList)
{
    DHLOGI("Get extra parameters, not support yet.");
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::ReqMmapBuffer(int32_t reqSize, AudioMmapBufferDescriptor &desc)
{
    DHLOGI("Request mmap buffer, not support yet.");
    (void)reqSize;
    (void)desc;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetMmapPosition(uint64_t &frames, AudioTimeStamp &time)
{
    DHLOGI("Get mmap position, not support yet.");
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::AddAudioEffect(uint64_t effectid)
{
    DHLOGI("Add audio effect, not support yet.");
    (void)effectid;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::RemoveAudioEffect(uint64_t effectid)
{
    DHLOGI("Remove audio effect, not support yet.");
    (void)effectid;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetFrameBufferSize(uint64_t &bufferSize)
{
    DHLOGI("Get frame buffer size, not support yet.");
    (void)bufferSize;
    return HDF_SUCCESS;
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
