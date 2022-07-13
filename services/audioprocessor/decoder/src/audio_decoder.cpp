/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"){}
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

#include "audio_decoder.h"

#include "audio_info.h"
#include "avsharedmemory.h"
#include "media_errors.h"
#include "securec.h"

#include "audio_decoder_callback.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
const std::string AudioDecoder::DECODE_MIME_AAC = "audio/mp4a-latm";

AudioDecoder::~AudioDecoder()
{
    if (audioDecoder_ != nullptr) {
        DHLOGI("~AudioEncoder. Release audio codec.");
        StopAudioCodec();
        ReleaseAudioCodec();
    }
}

int32_t AudioDecoder::ConfigureAudioCodec(const AudioCommonParam &codecParam,
    const std::shared_ptr<IAudioCodecCallback> &codecCallback)
{
    DHLOGI("%s: Configure audio codec.", LOG_TAG);
    if (codecCallback == nullptr) {
        DHLOGE("%s: Codec callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    if (!IsInDecodeRange(codecParam)) {
        DHLOGE("%s: Param error, codec type %d, channel count %d, sample rate %d, sample format %d.", LOG_TAG,
            codecParam.codecType, codecParam.channelMask, codecParam.sampleRate, codecParam.bitFormat);
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    codecParam_ = codecParam;
    codecCallback_ = codecCallback;

    int32_t ret = InitAudioDecoder(codecParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Init audio decoder fail. Error code %d.", LOG_TAG, ret);
        return ret;
    }

    ret = SetDecoderFormat(codecParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Set decoder format fail. Error code %d.", LOG_TAG, ret);
        return ret;
    }

    return DH_SUCCESS;
}

bool AudioDecoder::IsInDecodeRange(const AudioCommonParam &codecParam)
{
    return codecParam.channelMask >= CHANNEL_MASK_MIN && codecParam.channelMask <= CHANNEL_MASK_MAX &&
        codecParam.sampleRate >= SAMPLE_RATE_MIN && codecParam.sampleRate <= SAMPLE_RATE_MAX &&
        codecParam.bitFormat == SAMPLE_S16LE;
}

int32_t AudioDecoder::InitAudioDecoder(const AudioCommonParam &codecParam)
{
    DHLOGI("%s: Init audio decoder.", LOG_TAG);
    switch (codecParam.codecType) {
        case AUDIO_CODEC_AAC:
            audioDecoder_ = Media::AudioDecoderFactory::CreateByMime(DECODE_MIME_AAC);
            break;
        default:
            DHLOGE("%s: Create decode fail. Invalid codec type %d.", LOG_TAG, codecParam.codecType);
            return ERR_DH_AUDIO_BAD_VALUE;
    }

    audioDecoderCallback_ = std::make_shared<AudioDecoderCallback>(shared_from_this());
    int32_t ret = audioDecoder_->SetCallback(audioDecoderCallback_);
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Set decoder callback fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }

    return DH_SUCCESS;
}

int32_t AudioDecoder::SetDecoderFormat(const AudioCommonParam &codecParam)
{
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    DHLOGI("%s: Set encoder format, codec type %d, channel count %d, sample rate %d, sample format %d.", LOG_TAG,
        codecParam.codecType, codecParam.channelMask, codecParam.sampleRate, codecParam.bitFormat);
    cfgFormat_.PutIntValue("channel_count", codecParam.channelMask);
    cfgFormat_.PutIntValue("sample_rate", codecParam.sampleRate);
    cfgFormat_.PutIntValue("audio_sample_format", AudioStandard::SAMPLE_S16LE);

    int32_t ret = audioDecoder_->Configure(cfgFormat_);
    if (ret != Media::MSERR_OK) {
        DHLOGE("%s: Configure decoder format fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }

    ret = audioDecoder_->Prepare();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Decoder prepare fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }
    return DH_SUCCESS;
}

int32_t AudioDecoder::ReleaseAudioCodec()
{
    DHLOGI("%s: Release audio codec.", LOG_TAG);
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioDecoder_->Release();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Decoder release fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_BAD_OPERATE;
    }
    audioDecoderCallback_ = nullptr;
    audioDecoder_ = nullptr;

    DHLOGI("%s: Release audio codec end.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t AudioDecoder::StartAudioCodec()
{
    DHLOGI("%s: Start audio codec.", LOG_TAG);
    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioDecoder_->Start();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Decoder start fail. Error code %d.", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_START;
    }

    StartInputThread();
    isDecoderRunning_.store(true);

    return DH_SUCCESS;
}

void AudioDecoder::StartInputThread()
{
    DHLOGI("%s: Start input thread.", LOG_TAG);
    decodeThread_ = std::thread(&AudioDecoder::InputDecodeAudioData, this);
}

int32_t AudioDecoder::StopAudioCodec()
{
    DHLOGI("%s: Stop audio codec.", LOG_TAG);
    isDecoderRunning_.store(false);
    StopInputThread();

    if (audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is null.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    bool isSuccess = true;
    int32_t ret = audioDecoder_->Flush();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Decoder flush fail. Error type: %d.", LOG_TAG, ret);
        isSuccess = isSuccess && false;
    }

    ret = audioDecoder_->Stop();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Decoder stop fail. Error type: %d.", LOG_TAG, ret);
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

void AudioDecoder::StopInputThread()
{
    DHLOGI("%s: Stop input thread.", LOG_TAG);
    if (decodeThread_.joinable()) {
        decodeThread_.join();
    }

    std::lock_guard<std::mutex> dataLock(mtxData_);
    std::queue<uint32_t>().swap(bufIndexQueue_);
    std::queue<std::shared_ptr<AudioData>>().swap(inputBufQueue_);
}

int32_t AudioDecoder::FeedAudioData(const std::shared_ptr<AudioData> &inputData)
{
    DHLOGI("%s: Feed audio data.", LOG_TAG);
    if (!isDecoderRunning_.load()) {
        DHLOGE("%s: Decoder is stopped.", LOG_TAG);
        return ERR_DH_AUDIO_CODEC_INPUT;
    }

    if (inputData == nullptr) {
        DHLOGE("%s: Input data is nullptr.", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    std::lock_guard<std::mutex> dataLock(mtxData_);
    while (inputBufQueue_.size() > AUDIO_DECODER_QUEUE_MAX) {
        DHLOGE("%s: Input data queue overflow.", LOG_TAG);
        inputBufQueue_.pop();
    }
    inputBufQueue_.push(inputData);
    decodeCond_.notify_all();

    return DH_SUCCESS;
}

void AudioDecoder::InputDecodeAudioData()
{
    while (isDecoderRunning_.load()) {
        std::shared_ptr<AudioData> audioData;
        int32_t bufferIndex = 0;
        {
            std::unique_lock<std::mutex> lock(mtxData_);
            decodeCond_.wait_for(lock, std::chrono::milliseconds(DECODE_WAIT_MILLISECONDS),
                [this]() { return (!inputBufQueue_.empty() && !bufIndexQueue_.empty()); });

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
            DHLOGE("%s: Decoder is stopped or null.", LOG_TAG);
            return;
        } else if (ret != DH_SUCCESS) {
            DHLOGE("%s: Process data fail. Error type: %d.", LOG_TAG, ret);
            continue;
        }
    }
}

int32_t AudioDecoder::ProcessData(const std::shared_ptr<AudioData> &audioData, const int32_t bufferIndex)
{
    if (!isDecoderRunning_.load() || audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is stopped or null, isRunning %d.", LOG_TAG, isDecoderRunning_.load());
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    auto inMem = audioDecoder_->GetInputBuffer(bufferIndex);
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

    inputTimeStampUs_ = GetDecoderTimeStamp();
    Media::AVCodecBufferInfo bufferInfo = {inputTimeStampUs_, static_cast<int32_t>(audioData->Size()), 0};
    DHLOGI("%s: QueueInputBuffer. AVCodecBufferInfo presentationTimeUs %lld.", LOG_TAG,
        (long long)bufferInfo.presentationTimeUs);

    auto bufferFlag = Media::AVCODEC_BUFFER_FLAG_NONE;
    if (bufferInfo.presentationTimeUs == 0) {
        bufferFlag = Media::AVCODEC_BUFFER_FLAG_CODEC_DATA;
    }

    int32_t ret = audioDecoder_->QueueInputBuffer(bufferIndex, bufferInfo, bufferFlag);
    if (ret != Media::MSERR_OK) {
        DHLOGE("%s: Queue input buffer fail. Error code %d", LOG_TAG, ret);
        return ERR_DH_AUDIO_CODEC_INPUT;
    }

    IncreaseWaitDecodeCnt();
    return DH_SUCCESS;
}

int64_t AudioDecoder::GetDecoderTimeStamp()
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

void AudioDecoder::IncreaseWaitDecodeCnt()
{
    std::lock_guard<std::mutex> countLock(mtxCnt_);
    waitOutputCount_++;
    DHLOGI("%s: Wait decoder output frames number is %d.", LOG_TAG, waitOutputCount_);
}

void AudioDecoder::ReduceWaitDecodeCnt()
{
    std::lock_guard<std::mutex> countLock(mtxCnt_);
    if (waitOutputCount_ <= 0) {
        DHLOGE("%s: Wait decoder output count %d.", LOG_TAG, waitOutputCount_);
    }
    waitOutputCount_--;
    DHLOGI("%s: Wait decoder output frames number is %d.", LOG_TAG, waitOutputCount_);
}

void AudioDecoder::OnInputBufferAvailable(uint32_t index)
{
    std::lock_guard<std::mutex> countLock(mtxData_);
    while (bufIndexQueue_.size() > AUDIO_DECODER_QUEUE_MAX) {
        DHLOGE("%s: Index queue overflow.", LOG_TAG);
        bufIndexQueue_.pop();
    }

    bufIndexQueue_.push(index);
    decodeCond_.notify_all();
}

void AudioDecoder::OnOutputBufferAvailable(uint32_t index, Media::AVCodecBufferInfo info,
    Media::AVCodecBufferFlag flag)
{
    if (!isDecoderRunning_.load() || audioDecoder_ == nullptr) {
        DHLOGE("%s: Decoder is stopped or null, isRunning %d.", LOG_TAG, isDecoderRunning_.load());
        return;
    }

    auto outMem = audioDecoder_->GetOutputBuffer(index);
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
    errno_t err = memcpy_s(outBuf->Data(), outBuf->Size(), outMem->GetBase(), static_cast<size_t>(info.size));
    if (err != EOK) {
        DHLOGE("%s: Copy output data fail. Error code %d. Buffer Size %zu, AVCodec info: size %d.",
            LOG_TAG, err, outBuf->Size(), info.size);
        return;
    }
    outBuf->SetInt64("timeUs", info.presentationTimeUs);
    outputTimeStampUs_ = info.presentationTimeUs;
    DHLOGI("%s: AVCodec info, output time stamp %lld.", LOG_TAG, (long long)info.presentationTimeUs);

    ReduceWaitDecodeCnt();
    err = DecodeDone(outBuf);
    if (err != DH_SUCCESS) {
        DHLOGE("%s: Decode done fail. Error code: %d.", LOG_TAG, err);
        return;
    }

    err = audioDecoder_->ReleaseOutputBuffer(index);
    if (err != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("%s: Release output buffer fail. Error code: %d, index %u.", LOG_TAG, err, index);
    }
}

void AudioDecoder::OnOutputFormatChanged(const Media::Format &format)
{
    if (format.GetFormatMap().empty()) {
        DHLOGE("%s: The first changed output frame format is null.", LOG_TAG);
        return;
    }
    outputFormat_ = format;
}

void AudioDecoder::OnError(const AudioEvent &event)
{
    DHLOGI("%s: Decoder error.", LOG_TAG);
    std::shared_ptr<IAudioCodecCallback> targetCodecCallback = codecCallback_.lock();
    if (targetCodecCallback == nullptr) {
        DHLOGE("%s: Codec callback is null.", LOG_TAG);
        return;
    }

    targetCodecCallback->OnCodecStateNotify(event);
}

int32_t AudioDecoder::DecodeDone(const std::shared_ptr<AudioData> &outputData)
{
    DHLOGI("%s: Decode done.", LOG_TAG);
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