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

#ifndef OHOS_IAUDIO_CTRL_TRANSPORT_H
#define OHOS_IAUDIO_CTRL_TRANSPORT_H

#include "iaudio_ctrl_trans_callback.h"

#include "audio_param.h"

namespace OHOS {
namespace DistributedHardware {
class IAudioCtrlTransport {
public:
    virtual ~IAudioCtrlTransport() = default;

    virtual int32_t SetUp(const std::shared_ptr<IAudioCtrlTransCallback> &callback) = 0;
    virtual int32_t Release() = 0;
    virtual int32_t Start() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t SendAudioEvent(const std::shared_ptr<AudioEvent> &event) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IAUDIO_CTRL_TRANSPORT_H
