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

#ifndef OHOS_IAUDIO_PROCESSOR_H
#define OHOS_IAUDIO_PROCESSOR_H

#include <memory>

#include "audio_param.h"
#include "audio_data.h"
#include "iaudio_processor_callback.h"

namespace OHOS {
namespace DistributedHardware {
class IAudioProcessor {
public:
    IAudioProcessor() = default;
    virtual ~IAudioProcessor() = default;

    virtual int32_t ConfigureAudioProcessor(const AudioCommonParam &localDevParam,
        const AudioCommonParam &remoteDevParam, const std::shared_ptr<IAudioProcessorCallback> &procCallback) = 0;
    virtual int32_t ReleaseAudioProcessor() = 0;
    virtual int32_t StartAudioProcessor() = 0;
    virtual int32_t StopAudioProcessor() = 0;
    virtual int32_t FeedAudioProcessor(const std::shared_ptr<AudioData> &inputData) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IAUDIO_PROCESSOR_H
