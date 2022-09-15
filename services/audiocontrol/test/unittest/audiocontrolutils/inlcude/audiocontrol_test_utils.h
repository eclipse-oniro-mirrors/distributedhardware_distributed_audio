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

#ifndef OHOS_DAUDIO_SOURCE_CONTROL_TEST_UTILS_H
#define OHOS_DAUDIO_SOURCE_CONTROL_TEST_UTILS_H

#include "audio_event.h"
#include "iaudio_event_callback.h"
#include "iaudio_ctrl_transport.h"

namespace OHOS {
namespace DistributedHardware {
class MockIAudioEventCallback : public IAudioEventCallback {
public:
    explicit MockIAudioEventCallback()
    {
    }
    ~MockIAudioEventCallback() {}

    void NotifyEvent(const AudioEvent &event)
    {
        (void) event;
    }
};

class MockIAudioCtrlTransport : public IAudioCtrlTransport {
public:
    explicit MockIAudioCtrlTransport(std::string devId) : devId_(devId) {}
    ~MockIAudioCtrlTransport() {}

    int32_t SetUp(const std::shared_ptr<IAudioCtrlTransCallback> &callback) override
    {
        return 0;
    }

    int32_t Release() override
    {
        return 0;
    }

    int32_t Start() override
    {
        return 0;
    }

    int32_t Stop() override
    {
        return 0;
    }

    int32_t SendAudioEvent(const AudioEvent &event) override
    {
        (void) event;
        return 0;
    }

private:
    const std::string devId_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_SOURCE_CONTROL_TEST_UTILS_H
