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

#ifndef OHOS_ENCODER_CALLBACK_TEST_H
#define OHOS_ENCODER_CALLBACK_TEST_H

#include "audio_data.h"
#include "audio_event.h"
#include "daudio_log.h"
#include "iaudio_codec_callback.h"
#include "iaudio_codec.h"
#include "audio_param.h"
#include "securec.h"

#include "avcodec_common.h"
#include "media_errors.h"
#include "format.h"

namespace OHOS {
namespace DistributedHardware {
class AudioEncoderCallbackTest : public IAudioCodecCallback {
public:
    AudioEncoderCallbackTest() {};
    ~AudioEncoderCallbackTest() = default;

    void OnCodecDataDone(const std::shared_ptr<AudioData> &outputData) override;
    void OnCodecStateNotify(const AudioEvent &event) override;
};

void AudioEncoderCallbackTest::OnCodecDataDone(const std::shared_ptr<AudioData> &outputData)
{
    (void)outputData;
}

void AudioEncoderCallbackTest::OnCodecStateNotify(const AudioEvent &event)
{
    (void)event;
}
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ENCODER_CALLBACK_TEST_H