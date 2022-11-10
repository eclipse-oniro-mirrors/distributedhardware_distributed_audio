/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef AUDIO_TRANSPORT_START_STATUS_H
#define AUDIO_TRANSPORT_START_STATUS_H

#include "audio_transport_context.h"

namespace OHOS {
namespace DistributedHardware {
class AudioTransportStartStatus : public AudioTransportStatus {
public:
    explicit AudioTransportStartStatus(std::shared_ptr<AudioTransportContext>& stateContext);
    ~AudioTransportStartStatus() override {}
    int32_t Start(std::shared_ptr<IAudioChannel> audioChannel, std::shared_ptr<IAudioProcessor> processor) override;
    int32_t Stop(std::shared_ptr<IAudioChannel> audioChannel, std::shared_ptr<IAudioProcessor> processor) override;
    int32_t Pause(std::shared_ptr<IAudioProcessor> processor) override;
    int32_t Restart(const AudioParam &localParam, const AudioParam &remoteParam,
        std::shared_ptr<IAudioProcessor> processor) override;
    TransportStateType GetStateType() override;

private:
    std::weak_ptr<AudioTransportContext> stateContext_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // AUDIO_TRANSPORT_START_STATUS_H
