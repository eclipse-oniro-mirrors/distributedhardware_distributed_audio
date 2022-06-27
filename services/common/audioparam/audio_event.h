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

#ifndef OHOS_DAUDIO_AUDIO_EVENT_H
#define OHOS_DAUDIO_AUDIO_EVENT_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    EVENT_UNKNOWN = 0,
    CTRL_OPENED = 1,
    CTRL_CLOSED = 2,
    VOLUME_CHANGE = 3,
    VOLUME_GET = 4,
    VOLUME_MIN_GET = 5,
    VOLUME_MAX_GET = 6,
    VOLUME_SET = 7,
    VOLUME_MUTE_SET = 8,
    OPEN_CTRL = 9,
    CLOSE_CTRL = 10,
    OPEN_MIC = 11,
    CLOSE_MIC = 12,
    SET_PARAM = 13,
    SEND_PARAM = 14,
    OPEN_SPEAKER = 15,
    CLOSE_SPEAKER = 16,
    NOTIFY_OPEN_SPEAKER_RESULT = 17,
    NOTIFY_OPEN_MIC_RESULT = 18,
    NOTIFY_OPEN_CTRL_RESULT = 19,
    AUDIO_ENCODER_ERR = 20,
    AUDIO_DECODER_ERR = 21,
    SPEAKER_OPENED = 22,
    SPEAKER_CLOSED = 23,
    MIC_OPENED = 24,
    MIC_CLOSED = 25,
    DATA_OPENED = 26,
    DATA_CLOSED = 27,
} AudioEventType;

typedef struct {
    AudioEventType type;
    std::string content;
} AudioEvent;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_AUDIO_EVENT_H
