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

#ifndef AUDIO_TRANSPORT_STATUS_H
#define AUDIO_TRANSPORT_STATUS_H

#include "audio_param.h"
#include "iaudio_channel.h"
#include "iaudio_processor.h"

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    TRANSPORT_STATE_START = 0,
    TRANSPORT_STATE_PAUSE = 1,
    TRANSPORT_STATE_STOP = 2,
} TransportStateType;

class AudioTransportStatus {
public:
    AudioTransportStatus() = default;
    virtual ~AudioTransportStatus() = default;
    virtual int32_t Start(std::shared_ptr<IAudioChannel> audioChannel, std::shared_ptr<IAudioProcessor> processor) = 0;
    virtual int32_t Stop(std::shared_ptr<IAudioChannel> audioChannel, std::shared_ptr<IAudioProcessor> processor) = 0;
    virtual int32_t Pause(std::shared_ptr<IAudioProcessor> processor) = 0;
    virtual int32_t Restart(const AudioParam &localParam, const AudioParam &remoteParam,
        std::shared_ptr<IAudioProcessor> processor) = 0;
    virtual TransportStateType GetStateType() = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // AUDIO_TRANSPORT_STATUS_H
