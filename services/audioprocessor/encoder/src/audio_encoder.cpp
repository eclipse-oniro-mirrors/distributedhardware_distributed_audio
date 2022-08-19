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

#include "audio_encoder.h"

#include <fstream>

#include "audio_info.h"
#include "avsharedmemory.h"
#include "media_errors.h"
#include "securec.h"

#include "audio_encoder_callback.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
const std::string AudioEncoder::ENCODE_MIME_AAC = "audio/mp4a-latm";

AudioEncoder::~AudioEncoder()
{
    if (audioEncoder_ != nullptr) {
        DHLOGI("~AudioEncoder. Release audio codec.");
        StopAudioCodec();
        ReleaseAudioCodec();
    }
}

int32_t AudioEncoder::ConfigureAudioCodec(const AudioCommonParam &codecParam,
    const std::shared_ptr<IAudioCodecCallback> &codecCallback)
{
    DHLOGI("%s: Configure audio codec.", LOG_TAG);
    if (codecCallback == nullptr) {
        DHLOGE("%s: Codec callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    if (!IsInEncodeRange(codecParam)) {
        DHLOGE("%s: Param error, codec type %d, channel count %d, sample rate %d, sample format %d.", LOG_TAG,
            codecParam.codecType, codecParam.channelMask, codecParam.sampleRate, codecParam.bitFormat);
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    codecParam_ = codecParam;
    codecCallback_ = codecCallback;

    int32_t ret = InitAudioEncoder(codecParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Init audio encoder fail. Error code %d.", LOG_TAG, ret);
        return ret;
    }

    ret = SetEncoderFormat(codecParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Set encoder format fail. Error code %d.", LOG_TAG, ret);
        return ret;
    }

    return DH_SUCCESS;
}

bool AudioEncoder::IsInEncodeRange(const AudioCommonParam &codecParam)
{
    return codecParam.channelMask >= CHANNEL_MASK_MIN && codecParam.channelMask <= CHANNEL_MASK_MAX &&
        codecParam.sampleRate >= SAMPLE_RATE_MIN && codecParam.sampleRate <= SAMPLE_RATE_MAX &&
        codecParam.bitFormat == SAMPLE_S16LE;
}

int32_t AudioEncoder::InitAudioEncoder(const AudioCommonParam &codecParam)
{
    DHLOGI("%s: Init audio encoder.", LOG_TAG);
    switch (codecParam.codecType) {
        case AUDIO_CODEC_AAC:
            audioEncoder_ = Media::AudioEncoderFactory::CreateByMime(ENCODE_MIME_AAC);
            break;
        default:
            DHLOGE("%s: Create encoder fail. Invalid codec type %d.", LOG_TAG, codecParam.codecType);
            return ERR_DH_AUDIO_BAD_VALUE;
    }

    audioEncoderCallback_ = std::make_shared<AudioEncoderCallback>(shared_from_this());
    int32_t ret = audioEncoder_->SetCallback(audioEncoderCallback_);
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Set encoder callback fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }

    return DH_SUCCESS;
}

int32_t AudioEncoder::SetEncoderFormat(const AudioCommonParam &codecParam)
{
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    DHLOGI("%s: Set encoder format, codec type %d, channel count %d, sample rate %d, sample format %d.", LOG_TAG,
        codecParam.codecType, codecParam.channelMask, codecParam.sampleRate, codecParam.bitFormat);
    cfgFormat_.PutIntValue("channel_count", codecParam.channelMask);
    cfgFormat_.PutIntValue("sample_rate", codecParam.sampleRate);
    cfgFormat_.PutIntValue("audio_sample_format", AudioStandard::SAMPLE_S16LE);

    int32_t ret = audioEncoder_->Configure(cfgFormat_);
    if (ret != Media::MSERR_OK) {
        DHLOGE("%s: Configure encoder format fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }

    ret = audioEncoder_->Prepare();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Encoder prepare fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }
    return DH_SUCCESS;
}

int32_t AudioEncoder::ReleaseAudioCodec()
{
    DHLOGI("%s: Release audio codec.", LOG_TAG);
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        return DH_SUCCESS;
    }

    int32_t ret = audioEncoder_->Release();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Encoder release fail. Error type: %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_RELEASE;
    }
    audioEncoderCallback_ = nullptr;
    audioEncoder_ = nullptr;

    DHLOGI("%s: Release audio codec end.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioEncoder::StartAudioCodec()
{
    DHLOGI("%s: Start audio codec.", LOG_TAG);
    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioEncoder_->Start();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Encoder start fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_START;
    }

    StartInputThread();
    isEncoderRunning_.store(true);

    return DH_SUCCESS;
}

void AudioEncoder::StartInputThread()
{
    DHLOGI("%s: Start input thread.", LOG_TAG);
    encodeThread_ = std::thread(&AudioEncoder::InputEncodeAudioData, this);
}

int32_t AudioEncoder::StopAudioCodec()
{
    DHLOGI("%s: Stop audio codec.", LOG_TAG);
    isEncoderRunning_.store(false);
    encodeCond_.notify_all();
    StopInputThread();

    if (audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is null.", LOG_TAG);
        return DH_SUCCESS;
    }

    bool isSuccess = true;
    int32_t ret = audioEncoder_->Flush();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Encoder flush fail. Error type: %d.", LOG_TAG, ret);
        isSuccess = isSuccess && false;
    }
    ret = audioEncoder_->Stop();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Encoder stop fail. Error type: %d.", LOG_TAG, ret);
        isSuccess = isSuccess && false;
    }

    if (!isSuccess) {
        return ERR_DH_AUDIO_CODEC_STOP;
    }

    firstInputTimeUs_ = 0;
    inputTimeStampUs_ = 0;
    outputTimeStampUs_ = 0;
    waitOutputCount_ = 0;
    DHLOGI("%s: Stop audio codec end.", LOG_TAG);
    return DH_SUCCESS;
}

void AudioEncoder::StopInputThread()
{
    if (encodeThread_.joinable()) {
        encodeThread_.join();
    }

    std::lock_guard<std::mutex> dataLock(mtxData_);
    std::queue<uint32_t>().swap(bufIndexQueue_);
    std::queue<std::shared_ptr<AudioData>>().swap(inputBufQueue_);
    DHLOGI("%s: Stop input thread success.", LOG_TAG);
}

int32_t AudioEncoder::FeedAudioData(const std::shared_ptr<AudioData> &inputData)
{
    DHLOGD("%s: Feed audio data.", LOG_TAG);
    if (!isEncoderRunning_.load()) {
        DHLOGE("%s: Encoder is stopped.", LOG_TAG);
        return ERR_DH_AUDIO_CODEC_INPUT;
    }

    if (inputData == nullptr) {
        DHLOGE("%s: Input data is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    std::lock_guard<std::mutex> dataLock(mtxData_);
    while (inputBufQueue_.size() > AUDIO_ENCODER_QUEUE_MAX) {
        DHLOGE("%s: Input data queue overflow.", LOG_TAG);
        inputBufQueue_.pop();
    }
    inputBufQueue_.push(inputData);
    encodeCond_.notify_all();

    return DH_SUCCESS;
}

void AudioEncoder::InputEncodeAudioData()
{
    while (isEncoderRunning_.load()) {
        std::shared_ptr<AudioData> audioData;
        int32_t bufferIndex = 0;
        {
            std::unique_lock<std::mutex> lock(mtxData_);
            encodeCond_.wait_for(lock, std::chrono::milliseconds(ENCODE_WAIT_MILLISECONDS),
                [this]() {
                    return (!inputBufQueue_.empty() && !bufIndexQueue_.empty()) || !isEncoderRunning_.load();
                });

            if (inputBufQueue_.empty() || bufIndexQueue_.empty()) {
                continue;
            }
            bufferIndex = (int32_t)bufIndexQueue_.front();
            bufIndexQueue_.pop();
            audioData = inputBufQueue_.front();
            inputBufQueue_.pop();
        }

        int32_t ret = ProcessData(audioData, bufferIndex);
        if (ret == ERR_DH_AUDIO_BAD_VALUE) {
            DHLOGE("%s: Encoder is stopped or null.", LOG_TAG);
            return;
        } else if (ret != DH_SUCCESS) {
            DHLOGE("%s: Process data fail. Error type: %d.", LOG_TAG, ret);
            continue;
        }
    }
}

int32_t AudioEncoder::ProcessData(const std::shared_ptr<AudioData> &audioData, const int32_t bufferIndex)
{
    if (!isEncoderRunning_.load() || audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is stopped or null, isRunning %d.", LOG_TAG, isEncoderRunning_.load());
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    auto inMem = audioEncoder_->GetInputBuffer(bufferIndex);
    if (inMem == nullptr) {
        DHLOGE("%s: Get input buffer fail.", LOG_TAG);
        return ERR_DH_AUDIO_CODEC_INPUT;
    }

    if (inMem->GetSize() == INVALID_MEMORY_SIZE || static_cast<size_t>(inMem->GetSize()) < audioData->Size()) {
        DHLOGE("%s: Input buffer size error. Memory size %d, data size %zu.", LOG_TAG,
            inMem->GetSize(), audioData->Size());
    }

    errno_t err = memcpy_s(inMem->GetBase(), inMem->GetSize(), audioData->Data(), audioData->Size());
    if (err != EOK) {
        DHLOGE("%s: Copy input data fail. Error code %d. Memory size %d, data size %zu.",
            LOG_TAG, err, inMem->GetSize(), audioData->Size());
        return ERR_DH_AUDIO_BAD_OPERATE;
    }

    inputTimeStampUs_ = GetEncoderTimeStamp();
    Media::AVCodecBufferInfo bufferInfo = {inputTimeStampUs_, static_cast<int32_t>(audioData->Size()), 0};
    DHLOGD("%s: Queue input buffer. AVCodec info: input time stamp %lld, data size %zu.", LOG_TAG,
        (long long)bufferInfo.presentationTimeUs, audioData->Size());

    int32_t ret = audioEncoder_->QueueInputBuffer(bufferIndex, bufferInfo, Media::AVCODEC_BUFFER_FLAG_NONE);
    if (ret != Media::MSERR_OK) {
        DHLOGE("%s: Queue input buffer fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_INPUT;
    }

    IncreaseWaitEncodeCnt();
    return DH_SUCCESS;
}

int64_t AudioEncoder::GetEncoderTimeStamp()
{
    int64_t TimeIntervalStampUs = 0;
    int64_t nowTimeUs = GetNowTimeUs();
    if (firstInputTimeUs_ == 0) {
        firstInputTimeUs_ = nowTimeUs;
        return TimeIntervalStampUs;
    }

    TimeIntervalStampUs = nowTimeUs - firstInputTimeUs_;
    return TimeIntervalStampUs;
}

void AudioEncoder::IncreaseWaitEncodeCnt()
{
    std::lock_guard<std::mutex> lck(mtxCnt_);
    waitOutputCount_++;
    DHLOGD("Wait encoder output frames number is %d.", waitOutputCount_);
}

void AudioEncoder::ReduceWaitEncodeCnt()
{
    std::lock_guard<std::mutex> lck(mtxCnt_);
    if (waitOutputCount_ <= 0) {
        DHLOGE("%s: Wait encoder output count %d.", LOG_TAG, waitOutputCount_);
    }
    waitOutputCount_--;
    DHLOGD("%s: Wait encoder output frames number is %d.", LOG_TAG, waitOutputCount_);
}

void AudioEncoder::OnInputBufferAvailable(uint32_t index)
{
    std::lock_guard<std::mutex> lck(mtxData_);
    while (bufIndexQueue_.size() > AUDIO_ENCODER_QUEUE_MAX) {
        DHLOGE("%s: Index queue overflow.", LOG_TAG);
        bufIndexQueue_.pop();
    }

    bufIndexQueue_.push(index);
    encodeCond_.notify_all();
}

void AudioEncoder::OnOutputBufferAvailable(uint32_t index, Media::AVCodecBufferInfo info,
    Media::AVCodecBufferFlag flag)
{
    if (!isEncoderRunning_.load() || audioEncoder_ == nullptr) {
        DHLOGE("%s: Encoder is stopped or null, isRunning %d.", LOG_TAG, isEncoderRunning_.load());
        return;
    }

    auto outMem = audioEncoder_->GetOutputBuffer(index);
    if (outMem == nullptr) {
        DHLOGE("%s: Get output buffer fail. index %u.", LOG_TAG, index);
        return;
    }

    if (info.size <= 0 || info.size > outMem->GetSize()) {
        DHLOGE("%s: Codec output info error. AVCodec info: size %d, memory size %d.", LOG_TAG,
            info.size, outMem->GetSize());
        return;
    }

    auto outBuf = std::make_shared<AudioData>(static_cast<size_t>(info.size));
    errno_t err = memcpy_s(outBuf->Data(), outBuf->Size(), outMem->GetBase(), info.size);
    if (err != EOK) {
        DHLOGE("%s: Copy output data fail. Error code %d. Output Buffer Size %zu, AVCodec info: size %d.",
            LOG_TAG, err, outBuf->Size(), info.size);
        return;
    }
    outBuf->SetInt64("timeUs", info.presentationTimeUs);
    outputTimeStampUs_ = info.presentationTimeUs;
    DHLOGD("%s: Get output buffer. AVCodec info: output time stamp %lld, data size %zu.", LOG_TAG,
        (long long)info.presentationTimeUs, outBuf->Size());

    ReduceWaitEncodeCnt();
    err = EncodeDone(outBuf);
    if (err != DH_SUCCESS) {
        DHLOGE("%s: Encode done fail. Error code: %d.", LOG_TAG, err);
        return;
    }

    err = audioEncoder_->ReleaseOutputBuffer(index);
    if (err != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Release output buffer fail. Error code: %d, index %u.", LOG_TAG, err, index);
    }
}

void AudioEncoder::OnOutputFormatChanged(const Media::Format &format)
{
    if (format.GetFormatMap().empty()) {
        DHLOGE("%s: The first changed output data format is null.", LOG_TAG);
        return;
    }
    outputFormat_ = format;
}

void AudioEncoder::OnError(const AudioEvent &event)
{
    DHLOGE("%s: Encoder error.", LOG_TAG);
    std::shared_ptr<IAudioCodecCallback> targetCodecCallback = codecCallback_.lock();
    if (targetCodecCallback == nullptr) {
        DHLOGE("%s: Codec callback is null.", LOG_TAG);
        return;
    }

    targetCodecCallback->OnCodecStateNotify(event);
}

int32_t AudioEncoder::EncodeDone(const std::shared_ptr<AudioData> &outputData)
{
    DHLOGD("%s: Encode done.", LOG_TAG);
    if (outputData == nullptr) {
        DHLOGE("%s: Output data is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    std::shared_ptr<IAudioCodecCallback> targetCodecCallback = codecCallback_.lock();
    if (targetCodecCallback == nullptr) {
        DHLOGE("%s: Codec callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    targetCodecCallback->OnCodecDataDone(outputData);
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
