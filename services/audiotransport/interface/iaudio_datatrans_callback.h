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

#ifndef IAUDIO_DATATRANS_CALLBACK_H
#define IAUDIO_DATATRANS_CALLBACK_H

#include "audio_data.h"
#include "audio_event.h"
#include "audio_param.h"

namespace OHOS {
namespace DistributedHardware {
class IAudioDataTransCallback {
public:
    IAudioDataTransCallback() = default;
    virtual ~IAudioDataTransCallback() = default;
    virtual int32_t OnStateChange(const AudioEventType type) = 0;
    virtual int32_t OnDecodeTransDataDone(const std::shared_ptr<AudioData> &audioData) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // IAUDIO_DATATRANS_CALLBACK_H
