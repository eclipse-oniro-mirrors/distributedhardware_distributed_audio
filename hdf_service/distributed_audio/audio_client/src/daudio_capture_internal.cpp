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

#include "daudio_capture_internal.h"

#include <securec.h>

#include "daudio_attribute_internal.h"
#include "daudio_control_internal.h"
#include "daudio_errcode.h"
#include "daudio_log.h"
#include "daudio_scene_internal.h"
#include "daudio_volume_internal.h"

#define HDF_LOG_TAG HDF_AUDIO

namespace OHOS {
namespace DistributedHardware {
static const char * const AUDIO_LOG = "DAudioCaptureInternal";

static int32_t GetCapturePositionInternal(struct AudioCapture *capture, uint64_t *frames, struct AudioTimeStamp *time)
{
    if (capture == nullptr || frames == nullptr || time == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }
    AudioCaptureContext *context = reinterpret_cast<AudioCaptureContext *>(capture);
    AudioTimeStampHAL timeHal;
    int32_t ret = context->proxy_->GetCapturePosition(*frames, timeHal);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:The GetCapturePosition is failed.", AUDIO_LOG);
        return ret;
    }
    time->tvSec = timeHal.tvSec;
    time->tvNSec = timeHal.tvNSec;
    return DH_SUCCESS;
}

static int32_t CaptureFrameInternal(struct AudioCapture *capture, void *frame, uint64_t requestBytes,
    uint64_t *replyBytes)
{
    if (capture == nullptr || frame == nullptr || requestBytes == 0 || replyBytes == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioCaptureContext *context = reinterpret_cast<AudioCaptureContext *>(capture);
    uint8_t *uframe = reinterpret_cast<uint8_t *>(frame);
    std::vector<uint8_t> frameHal;
    int32_t ret = context->proxy_->CaptureFrame(frameHal, requestBytes, *replyBytes);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:The CaptureFrame is failed.", AUDIO_LOG);
        return ret;
    }

    ret = memcpy_s(uframe, requestBytes, frameHal.data(), requestBytes);
    if (ret != EOK) {
        DHLOGE("%s: DaudioCaptureInternal captureFrameInternal memcpy_s failed ret: %d.", AUDIO_LOG, ret);
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
    return DH_SUCCESS;
}

AudioCaptureContext::AudioCaptureContext()
{
    instance_.GetCapturePosition = GetCapturePositionInternal;
    instance_.CaptureFrame = CaptureFrameInternal;

    instance_.control.Start = AudioControlInternal<AudioCaptureContext>::Start;
    instance_.control.Stop = AudioControlInternal<AudioCaptureContext>::Stop;
    instance_.control.Pause = AudioControlInternal<AudioCaptureContext>::Pause;
    instance_.control.Resume = AudioControlInternal<AudioCaptureContext>::Resume;
    instance_.control.Flush = AudioControlInternal<AudioCaptureContext>::Flush;
    instance_.control.TurnStandbyMode = AudioControlInternal<AudioCaptureContext>::TurnStandbyMode;
    instance_.control.AudioDevDump = AudioControlInternal<AudioCaptureContext>::AudioDevDump;

    instance_.attr.GetFrameSize = AudioAttributeInternal<AudioCaptureContext>::GetFrameSize;
    instance_.attr.GetFrameCount = AudioAttributeInternal<AudioCaptureContext>::GetFrameCount;
    instance_.attr.SetSampleAttributes = AudioAttributeInternal<AudioCaptureContext>::SetSampleAttributes;
    instance_.attr.GetSampleAttributes = AudioAttributeInternal<AudioCaptureContext>::GetSampleAttributes;
    instance_.attr.GetCurrentChannelId = AudioAttributeInternal<AudioCaptureContext>::GetCurrentChannelId;
    instance_.attr.SetExtraParams = AudioAttributeInternal<AudioCaptureContext>::SetExtraParams;
    instance_.attr.GetExtraParams = AudioAttributeInternal<AudioCaptureContext>::GetExtraParams;
    instance_.attr.ReqMmapBuffer = AudioAttributeInternal<AudioCaptureContext>::ReqMmapBuffer;
    instance_.attr.GetMmapPosition = AudioAttributeInternal<AudioCaptureContext>::GetMmapPosition;

    instance_.scene.SelectScene = AudioSceneInternal<AudioCaptureContext>::SelectScene;
    instance_.scene.CheckSceneCapability = AudioSceneInternal<AudioCaptureContext>::CheckSceneCapability;

    instance_.volume.SetMute = AudioVolumeInternal<AudioCaptureContext>::SetMute;
    instance_.volume.GetMute = AudioVolumeInternal<AudioCaptureContext>::GetMute;
    instance_.volume.SetVolume = AudioVolumeInternal<AudioCaptureContext>::SetVolume;
    instance_.volume.GetVolume = AudioVolumeInternal<AudioCaptureContext>::GetVolume;
    instance_.volume.GetGainThreshold = AudioVolumeInternal<AudioCaptureContext>::GetGainThreshold;
    instance_.volume.SetGain = AudioVolumeInternal<AudioCaptureContext>::SetGain;
    instance_.volume.GetGain = AudioVolumeInternal<AudioCaptureContext>::GetGain;
}

AudioCaptureContext::~AudioCaptureContext() {}
} // DistributedHardware
} // OHOS