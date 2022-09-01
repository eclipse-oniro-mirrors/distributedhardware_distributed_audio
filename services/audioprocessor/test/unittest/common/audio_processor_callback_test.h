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

#ifndef OHOS_DAUDIO_PROCESSOR_CALLBACK_TEST_H
#define OHOS_DAUDIO_PROCESSOR_CALLBACK_TEST_H

#include "audio_data.h"
#include "audio_event.h"
#include "daudio_log.h"
#include "iaudio_processor_callback.h"

namespace OHOS {
namespace DistributedHardware {
class AudioProcessorCallbackTest : public IAudioProcessorCallback {
public:
    AudioProcessorCallbackTest() {};
    ~AudioProcessorCallbackTest() = default;

    void OnAudioDataDone(const std::shared_ptr<AudioData> &outputData) override;
    void OnStateNotify(const AudioEvent &event) override;

private:
    int32_t outputCnt_ = 0;
};

void AudioProcessorCallbackTest::OnAudioDataDone(const std::shared_ptr<AudioData> &outputData)
{
    DHLOGD("Test : On audio data done.");
    outputCnt_++;
    DHLOGD("Test : output the [%d]th processed audiodata, size %d sucessfuly.", outputCnt_, outputData->Size());
}

void AudioProcessorCallbackTest::OnStateNotify(const AudioEvent &event)
{
    DHLOGE("Test : On state notify, event: %d.", event.type);
}
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_PROCESSOR_CALLBACK_TEST_H