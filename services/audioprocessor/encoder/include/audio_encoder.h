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

#ifndef OHOS_AUDIO_ENCODER_H
#define OHOS_AUDIO_ENCODER_H

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

#include "avcodec_common.h"
#include "avcodec_audio_encoder.h"
#include "format.h"

#include "audio_data.h"
#include "audio_event.h"
#include "audio_param.h"
#include "iaudio_codec.h"
#include "iaudio_codec_callback.h"

namespace OHOS {
namespace DistributedHardware {
class AudioEncoder : public IAudioCodec, public std::enable_shared_from_this<AudioEncoder> {
public:
    AudioEncoder() = default;
    ~AudioEncoder();

    int32_t ConfigureAudioCodec(const AudioCommonParam &codecParam,
        const std::shared_ptr<IAudioCodecCallback> &codecCallback) override;
    int32_t ReleaseAudioCodec() override;
    int32_t StartAudioCodec() override;
    int32_t StopAudioCodec() override;
    int32_t FeedAudioData(const std::shared_ptr<AudioData> &inputData) override;

    void OnInputBufferAvailable(uint32_t index);
    void OnOutputBufferAvailable(uint32_t index, Media::AVCodecBufferInfo info, Media::AVCodecBufferFlag flag);
    void OnOutputFormatChanged(const Media::Format &format);
    void OnError(const AudioEvent &event);

private:
    int32_t InitAudioEncoder(const AudioCommonParam &codecParam);
    int32_t SetEncoderFormat(const AudioCommonParam &codecParam);
    void StartInputThread();
    void StopInputThread();
    void IncreaseWaitEncodeCnt();
    void ReduceWaitEncodeCnt();
    void InputEncodeAudioData();
    int32_t ProcessData(const std::shared_ptr<AudioData> &audioData, const int32_t bufferIndex);
    int64_t GetEncoderTimeStamp();
    int32_t EncodeDone(const std::shared_ptr<AudioData> &outputData);

private:
    constexpr static const char *LOG_TAG = "AudioEncoder";
    constexpr static int32_t AUDIO_ENCODER_QUEUE_MAX = 100;
    constexpr static uint32_t ENCODE_WAIT_MILLISECONDS = 50;
    constexpr static int32_t INVALID_MEMORY_SIZE = -1;
    const static std::string ENCODE_MIME_AAC;

    std::mutex mtxData_;
    std::mutex mtxCnt_;
    std::thread encodeThread_;
    std::condition_variable encodeCond_;

    std::atomic<bool> isEncoderRunning_ = false;
    int64_t firstInputTimeUs_ = 0;
    int64_t inputTimeStampUs_ = 0;
    int64_t outputTimeStampUs_ = 0;
    int32_t waitOutputCount_ = 0;

    Media::Format cfgFormat_;
    Media::Format outputFormat_;
    AudioCommonParam codecParam_;
    std::weak_ptr<IAudioCodecCallback> codecCallback_;
    std::shared_ptr<Media::AVCodecAudioEncoder> audioEncoder_ = nullptr;
    std::shared_ptr<Media::AVCodecCallback> audioEncoderCallback_ = nullptr;
    std::queue<std::shared_ptr<AudioData>> inputBufQueue_;
    std::queue<uint32_t> bufIndexQueue_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AUDIO_ENCODER_H
