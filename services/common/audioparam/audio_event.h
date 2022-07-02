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
    OPEN_CTRL = 1,
    CLOSE_CTRL = 2,
    CTRL_OPENED = 3,
    CTRL_CLOSED = 4,
    NOTIFY_OPEN_CTRL_RESULT = 5,
    DATA_OPENED = 6,
    DATA_CLOSED = 7,

    OPEN_SPEAKER = 11,
    CLOSE_SPEAKER = 12,
    SPEAKER_OPENED = 13,
    SPEAKER_CLOSED = 14,
    NOTIFY_OPEN_SPEAKER_RESULT = 15,

    OPEN_MIC = 21,
    CLOSE_MIC = 22,
    MIC_OPENED = 23,
    MIC_CLOSED = 24,
    NOTIFY_OPEN_MIC_RESULT = 25,

    VOLUME_SET = 31,
    VOLUME_GET = 32,
    VOLUME_CHANGE = 33,
    VOLUME_MIN_GET = 34,
    VOLUME_MAX_GET = 35,
    VOLUME_MUTE_SET = 36,

    SET_PARAM = 41,
    SEND_PARAM = 42,

    AUDIO_ENCODER_ERR = 51,
    AUDIO_DECODER_ERR = 52,
} AudioEventType;

typedef struct {
    AudioEventType type;
    std::string content;
} AudioEvent;

typedef enum {
    AUDIO_EVENT_UNKNOWN = 0,
    AUDIO_EVENT_VOLUME_SET = 1,
    AUDIO_EVENT_VOLUME_GET = 2,
    AUDIO_EVENT_VOLUME_CHANGE = 3,
    AUDIO_EVENT_OPEN_SPK_RESULT = 4,
    AUDIO_EVENT_OPEN_MIC_RESULT = 5,
} AudioEventHDF;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_AUDIO_EVENT_H
