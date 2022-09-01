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

#ifndef OHOS_MOCK_AUDIO_TRANSPORT_CALLBACK_H
#define OHOS_MOCK_AUDIO_TRANSPORT_CALLBACK_H

#include "audio_event.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "iaudio_processor_callback.h"

namespace OHOS {
namespace DistributedHardware {
class MockAudioTransportCallback : public IAudioDataTransCallback {
public:
    MockAudioTransportCallback() = default;
    ~MockAudioTransportCallback() = default;

    int32_t OnStateChange(const AudioEventType type) override;
    int32_t OnDecodeTransDataDone(const std::shared_ptr<AudioData> &audioData) override;
};

int32_t MockAudioTransportCallback::OnStateChange(const AudioEventType type)
{
    DHLOGD("Test : On state change, state: %d.", type);
    return DH_SUCCESS;
}

int32_t MockAudioTransportCallback::OnDecodeTransDataDone(const std::shared_ptr<AudioData> &audioData)
{
    (void) audioData;
    DHLOGE("Test : On decode trans data done.");
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_AUDIO_TRANSPORT_CALLBACK_H
