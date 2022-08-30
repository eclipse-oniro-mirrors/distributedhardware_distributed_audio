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

#ifndef OHOS_ISAUDIO_CHANNEL_H
#define OHOS_ISAUDIO_CHANNEL_H

#include "audio_data.h"
#include "audio_event.h"
#include "iaudio_channel_listener.h"

namespace OHOS {
namespace DistributedHardware {
class IAudioChannel {
public:
    virtual ~IAudioChannel() = default;

    virtual int32_t CreateSession(const std::shared_ptr<IAudioChannelListener> &listener,
        const std::string &sessionName) = 0;
    virtual int32_t ReleaseSession() = 0;
    virtual int32_t OpenSession() = 0;
    virtual int32_t CloseSession() = 0;
    virtual int32_t SendData(const std::shared_ptr<AudioData> &data) = 0;
    virtual int32_t SendEvent(const std::shared_ptr<AudioEvent> &event) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ISAUDIO_CHANNEL_H
