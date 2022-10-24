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

#include "audio_transport_status_factory.h"

#include "audio_transport_pause_status.h"
#include "audio_transport_start_status.h"
#include "audio_transport_status.h"
#include "audio_transport_stop_status.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioTransportStatusFactory"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(AudioTransportStatusFactory);

std::shared_ptr<AudioTransportStatus> AudioTransportStatusFactory::CreateState(TransportStateType stateType,
    std::shared_ptr<AudioTransportContext>& stateContext)
{
    std::shared_ptr<AudioTransportStatus> state = nullptr;
    switch (stateType) {
        case TRANSPORT_STATE_START: {
            state = std::make_shared<AudioTransportStartStatus>(stateContext);
            break;
        }
        case TRANSPORT_STATE_PAUSE: {
            state = std::make_shared<AudioTransportPauseStatus>(stateContext);
            break;
        }
        case TRANSPORT_STATE_STOP: {
            state = std::make_shared<AudioTransportStopStatus>(stateContext);
            break;
        }
        default: {
            DHLOGE("AudioTransportStatusFactory create state failed, wrong type %d", stateType);
            return nullptr;
        }
    }
    return state;
}
} // namespace DistributedHardware
} // namespace OHOS
