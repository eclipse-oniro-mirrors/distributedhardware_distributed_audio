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

#ifndef OHOS_IAUDIO_ENCODER_PROCESSOR_H
#define OHOS_IAUDIO_ENCODER_PROCESSOR_H

#include <memory>

#include "audio_data.h"
#include "audio_event.h"
#include "audio_param.h"
#include "iaudio_codec.h"
#include "iaudio_codec_callback.h"
#include "iaudio_processor.h"
#include "iaudio_processor_callback.h"

namespace OHOS {
namespace DistributedHardware {
class AudioEncoderProcessor : public IAudioProcessor, public IAudioCodecCallback,
    public std::enable_shared_from_this<AudioEncoderProcessor> {
public:
    AudioEncoderProcessor() = default;
    ~AudioEncoderProcessor() override;

    int32_t ConfigureAudioProcessor(const AudioCommonParam &localDevParam, const AudioCommonParam &remoteDevParam,
        const std::shared_ptr<IAudioProcessorCallback> &procCallback) override;
    int32_t ReleaseAudioProcessor() override;
    int32_t StartAudioProcessor() override;
    int32_t StopAudioProcessor() override;
    int32_t FeedAudioProcessor(const std::shared_ptr<AudioData> &inputData) override;

    void OnCodecDataDone(const std::shared_ptr<AudioData> &outputData) override;
    void OnCodecStateNotify(const AudioEvent &event) override;

private:
    std::shared_ptr<IAudioCodec> audioEncoder_ = nullptr;
    AudioCommonParam localDevParam_;
    AudioCommonParam remoteDevParam_;
    std::weak_ptr<IAudioProcessorCallback> procCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IAUDIO_ENCODER_PROCESSOR_H
