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

#include "audio_decoder_callback.h"

#include "daudio_log.h"
#include "audio_event.h"

namespace OHOS {
namespace DistributedHardware {
void AudioDecoderCallback::OnError(Media::AVCodecErrorType errorType, int32_t errorCode)
{
    DHLOGE("%s: OnError. Error type: %d, Error code: %d ", LOG_TAG, errorType, errorCode);
    std::shared_ptr<AudioDecoder> targetDecoderNode = audioDecoder_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("%s: audioDecoder is nullptr.", LOG_TAG);
        return;
    }
    AudioEvent decoderErr = {AUDIO_DECODER_ERR, ""};
    targetDecoderNode->OnError(decoderErr);
}

void AudioDecoderCallback::OnInputBufferAvailable(uint32_t index)
{
    DHLOGD("%s: OnInputBufferAvailable. index %u.", LOG_TAG, index);
    std::shared_ptr<AudioDecoder> targetDecoderNode = audioDecoder_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("%s: audioDecoder is nullptr.", LOG_TAG);
        return;
    }
    targetDecoderNode->OnInputBufferAvailable(index);
}

void AudioDecoderCallback::OnOutputFormatChanged(const Media::Format &format)
{
    DHLOGI("%s: OnOutputFormatChanged.", LOG_TAG);
    std::shared_ptr<AudioDecoder> targetDecoderNode = audioDecoder_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("%s: audioDecoder is nullptr.", LOG_TAG);
        return;
    }
    targetDecoderNode->OnOutputFormatChanged(format);
}

void AudioDecoderCallback::OnOutputBufferAvailable(uint32_t index, Media::AVCodecBufferInfo info,
    Media::AVCodecBufferFlag flag)
{
    DHLOGD("%s: OnOutputBufferAvailable. index %u.", LOG_TAG, index);
    std::shared_ptr<AudioDecoder> targetDecoderNode = audioDecoder_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("%s: audioDecoder is nullptr.", LOG_TAG);
        return;
    }
    targetDecoderNode->OnOutputBufferAvailable(index, info, flag);
}
} // namespace DistributedHardware
} // namespace OHOS
