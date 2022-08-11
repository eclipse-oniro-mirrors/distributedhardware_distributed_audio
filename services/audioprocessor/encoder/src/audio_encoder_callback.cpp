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

#include "audio_encoder_callback.h"

#include "daudio_log.h"
#include "audio_event.h"

namespace OHOS {
namespace DistributedHardware {
void AudioEncoderCallback::OnError(Media::AVCodecErrorType errorType, int32_t errorCode)
{
    DHLOGE("%s: OnError. Error type: %d, Error code: %d.", LOG_TAG, errorType, errorCode);
    std::shared_ptr<AudioEncoder> targetEncoderNode = audioEncoder_.lock();
    if (targetEncoderNode == nullptr) {
        DHLOGE("%s: audioEncoder is nullptr.", LOG_TAG);
        return;
    }
    AudioEvent encoderErr = {AUDIO_ENCODER_ERR, ""};
    targetEncoderNode->OnError(encoderErr);
}

void AudioEncoderCallback::OnInputBufferAvailable(uint32_t index)
{
    DHLOGD("%s: OnInputBufferAvailable. index %u.", LOG_TAG, index);
    std::shared_ptr<AudioEncoder> targetEncoderNode = audioEncoder_.lock();
    if (targetEncoderNode == nullptr) {
        DHLOGE("%s: audioEncoder is nullptr.", LOG_TAG);
        return;
    }
    targetEncoderNode->OnInputBufferAvailable(index);
}

void AudioEncoderCallback::OnOutputFormatChanged(const Media::Format &format)
{
    DHLOGI("%s: OnOutputFormatChanged.", LOG_TAG);
    std::shared_ptr<AudioEncoder> targetEncoderNode = audioEncoder_.lock();
    if (targetEncoderNode == nullptr) {
        DHLOGE("%s: audioEncoder is nullptr.", LOG_TAG);
        return;
    }
    targetEncoderNode->OnOutputFormatChanged(format);
}

void AudioEncoderCallback::OnOutputBufferAvailable(uint32_t index, Media::AVCodecBufferInfo info,
    Media::AVCodecBufferFlag flag)
{
    DHLOGD("%s: OnOutputBufferAvailable. index %u.", LOG_TAG, index);
    std::shared_ptr<AudioEncoder> targetEncoderNode = audioEncoder_.lock();
    if (targetEncoderNode == nullptr) {
        DHLOGE("%s: audioEncoder is nullptr.", LOG_TAG);
        return;
    }
    targetEncoderNode->OnOutputBufferAvailable(index, info, flag);
}
} // namespace DistributedHardware
} // namespace OHOS
