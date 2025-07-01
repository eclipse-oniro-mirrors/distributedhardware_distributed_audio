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

#ifndef SINKCTRLTRANSONCHANNELEVENT_FUZZER_H
#define SINKCTRLTRANSONCHANNELEVENT_FUZZER_H

#define FUZZ_PROJECT_NAME "sourcectrltransonchannelevent_fuzzer"
#include "iaudio_ctrltrans_callback.h"
namespace OHOS {
namespace DistributedHardware {
class SourceCtrlTransOnChannelEventFuzzer : public IAudioCtrlTransCallback {
public:
    SourceCtrlTransOnChannelEventFuzzer() = default;
    ~SourceCtrlTransOnChannelEventFuzzer() = default;

    void OnCtrlTransEvent(const AVTransEvent &event) override
    {
        (void)event;
    }
    void OnCtrlTransMessage(const std::shared_ptr<AVTransMessage> &message) override
    {
        (void)message;
    }
};
}
}
#endif