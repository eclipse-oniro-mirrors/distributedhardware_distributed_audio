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

#include "audio_lowlatency_processor.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioLowlatencyProcessor"

namespace OHOS {
namespace DistributedHardware {
int32_t AudioLowlatencyProcessor::ConfigureAudioProcessor(const AudioCommonParam &localDevParam,
    const AudioCommonParam &remoteDevParam, const std::shared_ptr<IAudioProcessorCallback> &procCallback)
{
    DHLOGI("Configure audio processor.");
    if (procCallback == nullptr) {
        DHLOGE("Processor callback is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    procCallback_ = procCallback;
    DHLOGI("Frame size is not default 4096, do not need to config audio processor.");
    return DH_SUCCESS;
}

int32_t AudioLowlatencyProcessor::ReleaseAudioProcessor()
{
    DHLOGI("Release audio processor.");
    return DH_SUCCESS;
}

int32_t AudioLowlatencyProcessor::StartAudioProcessor()
{
    DHLOGI("Start audio processor.");
    return DH_SUCCESS;
}

int32_t AudioLowlatencyProcessor::StopAudioProcessor()
{
    DHLOGI("Stop audio processor.");
    return DH_SUCCESS;
}

int32_t AudioLowlatencyProcessor::FeedAudioProcessor(const std::shared_ptr<AudioData> &inputData)
{
    DHLOGD("Feed audio processor.");
    if (inputData == nullptr) {
        DHLOGE("Input data is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    auto cbObj = procCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("Processor callback is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    cbObj->OnAudioDataDone(inputData);
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS