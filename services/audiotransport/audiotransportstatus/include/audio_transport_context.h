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

#ifndef AUDIO_TRANSPORT_CONTEXT_H
#define AUDIO_TRANSPORT_CONTEXT_H

#include "audio_transport_status.h"
#include "iaudio_channel.h"
#include "iaudio_processor.h"

namespace OHOS {
namespace DistributedHardware {
class AudioTransportContext : public std::enable_shared_from_this<AudioTransportContext> {
public:
    AudioTransportContext() = default;
    ~AudioTransportContext() = default;
    void SetTransportStatus(TransportStateType stateType);
    int32_t GetTransportStatusType();
    void SetAudioChannel(std::shared_ptr<IAudioChannel> &audioChannel);
    void SetAudioProcessor(std::shared_ptr<IAudioProcessor> &processor);
    int32_t Start();
    int32_t Stop();
    int32_t Pause();
    int32_t Restart(const AudioParam &localParam, const AudioParam &remoteParam);

private:
    std::shared_ptr<AudioTransportStatus> currentState_;
    std::shared_ptr<IAudioChannel> audioChannel_ = nullptr;
    std::shared_ptr<IAudioProcessor> processor_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // AUDIO_TRANSPORT_CONTEXT_H
