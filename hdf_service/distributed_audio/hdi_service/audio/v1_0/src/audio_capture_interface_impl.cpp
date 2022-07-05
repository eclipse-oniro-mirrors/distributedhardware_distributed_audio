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

#include "audio_capture_interface_impl.h"

#include <hdf_base.h>
#include <unistd.h>
#include "sys/time.h"
#include <securec.h>

#include "daudio_constants.h"
#include "daudio_log.h"

using namespace OHOS::DistributedHardware;
namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
AudioCaptureInterfaceImpl::AudioCaptureInterfaceImpl(const std::string adpName, const AudioDeviceDescriptorHAL &desc,
    const AudioSampleAttributesHAL &attrs, const sptr<IDAudioCallback> &callback)
    : adapterName_(adpName), devDesc_(desc), devAttrs_(attrs), audioExtCallback_(callback)
{
    DHLOGD("%s: Distributed Audio Capture constructed, id(%d).", AUDIO_LOG, desc.pins);
}

AudioCaptureInterfaceImpl::~AudioCaptureInterfaceImpl()
{
    DHLOGD("%s: Distributed Audio Capture destructed, id(%d).", AUDIO_LOG, devDesc_.pins);
}

int32_t AudioCaptureInterfaceImpl::GetCapturePosition(uint64_t &frames, AudioTimeStampHAL &time)
{
    DHLOGI("%s: Get capture position, not support yet.", AUDIO_LOG);
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::CaptureFrame(std::vector<uint8_t> &frame, uint64_t requestBytes,
    uint64_t &replyBytes)
{
    DHLOGI("%s: Capture frame[samplerate: %d, channelmask: %d, bitformat: %d].", AUDIO_LOG, devAttrs_.sampleRate,
        devAttrs_.channelCount, devAttrs_.format);
    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    std::lock_guard<std::mutex> captureLck(captureMtx_);
    if (captureStatus_ != CAPTURE_STATUS_START) {
        DHLOGE("%s: Capture status wrong, return false.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    AudioData audioData;
    int32_t ret = audioExtCallback_->ReadStreamData(adapterName_, devDesc_.pins, audioData);
    if (ret != HDF_SUCCESS) {
        DHLOGE("%s: Write stream data failed.", AUDIO_LOG);
        return HDF_FAILURE;
    }

    frame.resize(AUDIO_DATA_SIZE_DEFAULT);
    ret = memcpy_s(frame.data(), frame.size(), audioData.data.data(), audioData.data.size());
    if (ret != EOK) {
        DHLOGE("%s: AudioCaptureInterfaceImpl CaptureFrame memcpy_s failed ret: %d.", AUDIO_LOG, ret);
        return HDF_FAILURE;
    }

    ReadStreamWait(startTime);
    DHLOGI("%s: Capture audio frame success.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Start()
{
    DHLOGI("%s: Start capture.", AUDIO_LOG);
    std::lock_guard<std::mutex> captureLck(captureMtx_);
    captureStatus_ = CAPTURE_STATUS_START;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Stop()
{
    DHLOGI("%s: Stop capture.", AUDIO_LOG);
    std::lock_guard<std::mutex> captureLck(captureMtx_);
    captureStatus_ = CAPTURE_STATUS_STOP;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::Pause()
{
    DHLOGI("%s: Pause capture.", AUDIO_LOG);
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
    DHLOGI("%s: Turn stand by mode, not support yet.", AUDIO_LOG);
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::AudioDevDump(int32_t range, int32_t fd)
{
    DHLOGI("%s: Dump audio info, not support yet.", AUDIO_LOG);
    (void)range;
    (void)fd;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::CheckSceneCapability(const AudioSceneDescriptorHAL &scene, bool &support)
{
    DHLOGI("%s: Check scene capability.", AUDIO_LOG);
    (void)scene;
    support = false;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SelectScene(const AudioSceneDescriptorHAL &scene)
{
    DHLOGI("%s: Select audio scene, not support yet.", AUDIO_LOG);
    (void)scene;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetMute(bool mute)
{
    DHLOGI("%s: Set mute, not support yet.", AUDIO_LOG);
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetMute(bool &mute)
{
    DHLOGI("%s: Get mute, not support yet.", AUDIO_LOG);
    (void)mute;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetVolume(float volume)
{
    DHLOGI("%s: Can not set vol not by this interface.", AUDIO_LOG);
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetVolume(float &volume)
{
    DHLOGI("%s: Can not get vol not by this interface.", AUDIO_LOG);
    (void)volume;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetGainThreshold(float &min, float &max)
{
    DHLOGI("%s: Get gain threshold, not support yet.", AUDIO_LOG);
    min = 0;
    max = 0;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetGain(float gain)
{
    DHLOGI("%s: Set gain, not support yet.", AUDIO_LOG);
    (void)gain;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetGain(float &gain)
{
    DHLOGI("%s: Get gain, not support yet.", AUDIO_LOG);
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

int32_t AudioCaptureInterfaceImpl::SetSampleAttributes(const AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Set sample attributes.", AUDIO_LOG);
    devAttrs_ = attrs;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetSampleAttributes(AudioSampleAttributesHAL &attrs)
{
    DHLOGI("%s: Get sample attributes.", AUDIO_LOG);
    attrs = devAttrs_;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetCurrentChannelId(uint32_t &channelId)
{
    DHLOGI("%s: Get current channel id, not support yet.", AUDIO_LOG);
    (void)channelId;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::SetExtraParams(const std::string &keyValueList)
{
    DHLOGI("%s: Set extra parameters, not support yet.", AUDIO_LOG);
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetExtraParams(std::string &keyValueList)
{
    DHLOGI("%s: Get extra parameters, not support yet.", AUDIO_LOG);
    (void)keyValueList;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::ReqMmapBuffer(int32_t reqSize, AudioMmapBufferDescripterHAL &desc)
{
    DHLOGI("%s: Request mmap buffer, not support yet.", AUDIO_LOG);
    (void)reqSize;
    (void)desc;
    return HDF_SUCCESS;
}

int32_t AudioCaptureInterfaceImpl::GetMmapPosition(uint64_t &frames, AudioTimeStampHAL &time)
{
    DHLOGI("%s: Get mmap position, not support yet.", AUDIO_LOG);
    (void)frames;
    (void)time;
    return HDF_SUCCESS;
}

const AudioDeviceDescriptorHAL &AudioCaptureInterfaceImpl::GetCaptureDesc()
{
    return devDesc_;
}

void AudioCaptureInterfaceImpl::ReadStreamWait(const struct timeval &startTime)
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
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
