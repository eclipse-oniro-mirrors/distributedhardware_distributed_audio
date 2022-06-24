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

#ifndef OHOS_IAUDIO_CODEC_H
#define OHOS_IAUDIO_CODEC_H

#include <memory>

#include "audio_param.h"
#include "audio_data.h"
#include "iaudio_codec.h"
#include "iaudio_codec_callback.h"

namespace OHOS {
namespace DistributedHardware {
class IAudioCodec {
public:
    IAudioCodec() = default;
    virtual ~IAudioCodec() = default;

    virtual int32_t ConfigureAudioCodec(const AudioCommonParam &codecParam,
        const std::shared_ptr<IAudioCodecCallback> &codecCallback) = 0;
    virtual int32_t ReleaseAudioCodec() = 0;
    virtual int32_t StartAudioCodec() = 0;
    virtual int32_t StopAudioCodec() = 0;
    virtual int32_t FeedAudioData(const std::shared_ptr<AudioData> &inputData) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IAUDIO_CODEC_H
