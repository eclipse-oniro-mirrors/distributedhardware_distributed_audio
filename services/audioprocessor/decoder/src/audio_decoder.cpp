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

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioDecoder"

namespace OHOS {
namespace DistributedHardware {
const std::string AudioDecoder::DECODE_MIME_AAC = "audio/mp4a-latm";

AudioDecoder::~AudioDecoder()
{
    if (audioDecoder_ != nullptr) {
        DHLOGI("Release audio codec.");
        StopAudioCodec();
        ReleaseAudioCodec();
    }
}

int32_t AudioDecoder::ConfigureAudioCodec(const AudioCommonParam &codecParam,
    const std::shared_ptr<IAudioCodecCallback> &codecCallback)
{
    DHLOGI("Configure audio codec.");
    if (!IsInDecodeRange(codecParam) || codecCallback == nullptr) {
        DHLOGE("Codec param error or callback is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    codecParam_ = codecParam;
    codecCallback_ = codecCallback;

    int32_t ret = InitAudioDecoder(codecParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("Init audio decoder fail. Error code %d.", ret);
        return ret;
    }

    ret = SetDecoderFormat(codecParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("Set decoder format fail. Error code %d.", ret);
        return ret;
    }
    return DH_SUCCESS;
}

bool AudioDecoder::IsInDecodeRange(const AudioCommonParam &codecParam)
{
    if (codecParam.channelMask >= CHANNEL_MASK_MIN && codecParam.channelMask <= CHANNEL_MASK_MAX &&
        codecParam.sampleRate >= SAMPLE_RATE_MIN && codecParam.sampleRate <= SAMPLE_RATE_MAX &&
        codecParam.bitFormat == SAMPLE_S16LE && codecParam.codecType == AUDIO_CODEC_AAC) {
        return true;
    }

    DHLOGE("Param error, codec type %d, channel count %d, sample rate %d, sample format %d.",
        codecParam.codecType, codecParam.channelMask, codecParam.sampleRate, codecParam.bitFormat);
    return false;
}

int32_t AudioDecoder::InitAudioDecoder(const AudioCommonParam &codecParam)
{
    DHLOGI("Init audio decoder.");
    audioDecoder_ = Media::AudioDecoderFactory::CreateByMime(DECODE_MIME_AAC);
    if (audioDecoder_ == nullptr) {
        DHLOGE("Create audio decoder fail.");
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }

    decoderCallback_ = std::make_shared<AudioDecoderCallback>(shared_from_this());
    int32_t ret = audioDecoder_->SetCallback(decoderCallback_);
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Set decoder callback fail. Error code %d.", ret);
        decoderCallback_ = nullptr;
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }

    return DH_SUCCESS;
}

int32_t AudioDecoder::SetDecoderFormat(const AudioCommonParam &codecParam)
{
    if (audioDecoder_ == nullptr) {
        DHLOGE("Decoder is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    DHLOGI("Set encoder format, codec type %d, channel count %d, sample rate %d, sample format %d.",
        codecParam.codecType, codecParam.channelMask, codecParam.sampleRate, codecParam.bitFormat);
    cfgFormat_.PutIntValue("channel_count", codecParam.channelMask);
    cfgFormat_.PutIntValue("sample_rate", codecParam.sampleRate);
    cfgFormat_.PutIntValue("audio_sample_format",
        static_cast<AudioStandard::AudioSampleFormat>(codecParam.bitFormat));

    int32_t ret = audioDecoder_->Configure(cfgFormat_);
    if (ret != Media::MSERR_OK) {
        DHLOGE("Configure decoder format fail. Error code %d.", ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }

    ret = audioDecoder_->Prepare();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Decoder prepare fail. Error code %d.", ret);
        return ERR_DH_AUDIO_CODEC_CONFIG;
    }
    return DH_SUCCESS;
}

int32_t AudioDecoder::ReleaseAudioCodec()
{
    DHLOGI("Release audio codec.");
    if (audioDecoder_ == nullptr) {
        DHLOGE("Decoder is null.");
        return DH_SUCCESS;
    }

    int32_t ret = audioDecoder_->Release();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Decoder release fail. Error code %d.", ret);
        return ERR_DH_AUDIO_BAD_OPERATE;
    }
    decoderCallback_ = nullptr;
    audioDecoder_ = nullptr;

    DHLOGI("Release audio codec end.");
    return DH_SUCCESS;
}

int32_t AudioDecoder::StartAudioCodec()
{
    DHLOGI("Start audio codec.");
    if (audioDecoder_ == nullptr) {
        DHLOGE("Decoder is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int32_t ret = audioDecoder_->Start();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Decoder start fail. Error code %d.", ret);
        return ERR_DH_AUDIO_CODEC_START;
    }
    StartInputThread();
    return DH_SUCCESS;
}

void AudioDecoder::StartInputThread()
{
    DHLOGI("Start input thread.");
    decodeThread_ = std::thread(&AudioDecoder::InputDecodeAudioData, this);
    isDecoderRunning_.store(true);
}

int32_t AudioDecoder::StopAudioCodec()
{
    DHLOGI("Stop audio codec.");
    StopInputThread();
    if (audioDecoder_ == nullptr) {
        DHLOGE("Decoder is null.");
        return DH_SUCCESS;
    }

    bool isSuccess = true;
    int32_t ret = audioDecoder_->Flush();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Decoder flush fail. Error type: %d.", ret);
        isSuccess = false;
    }
    ret = audioDecoder_->Stop();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Decoder stop fail. Error type: %d.", ret);
        isSuccess = false;
    }
    if (!isSuccess) {
        return ERR_DH_AUDIO_CODEC_STOP;
    }

    firstInputTimeUs_ = 0;
    inputTimeStampUs_ = 0;
    outputTimeStampUs_ = 0;
    waitOutputCount_ = 0;
    DHLOGI("Stop audio codec end.");
    return DH_SUCCESS;
}

void AudioDecoder::StopInputThread()
{
    isDecoderRunning_.store(false);
    decodeCond_.notify_all();
    if (decodeThread_.joinable()) {
        decodeThread_.join();
    }

    std::lock_guard<std::mutex> dataLock(mtxData_);
    std::queue<uint32_t>().swap(bufIndexQueue_);
    std::queue<std::shared_ptr<AudioData>>().swap(inputBufQueue_);
    DHLOGI("Stop input thread success.");
}

int32_t AudioDecoder::FeedAudioData(const std::shared_ptr<AudioData> &inputData)
{
    DHLOGD("Feed audio data.");
    if (!isDecoderRunning_.load()) {
        DHLOGE("Decoder is stopped.");
        return ERR_DH_AUDIO_CODEC_INPUT;
    }
    if (inputData == nullptr) {
        DHLOGE("Input data is nullptr.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    std::lock_guard<std::mutex> dataLock(mtxData_);
    while (inputBufQueue_.size() > AUDIO_DECODER_QUEUE_MAX) {
        DHLOGE("Input data queue overflow.");
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
                [this]() {
                    return (!inputBufQueue_.empty() && !bufIndexQueue_.empty()) || !isDecoderRunning_.load();
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
            DHLOGE("Decoder is stopped or null.");
            return;
        } else if (ret != DH_SUCCESS) {
            DHLOGE("Process data fail. Error type: %d.", ret);
            continue;
        }
    }
}

int32_t AudioDecoder::ProcessData(const std::shared_ptr<AudioData> &audioData, const int32_t bufferIndex)
{
    if (!isDecoderRunning_.load() || audioDecoder_ == nullptr) {
        DHLOGE("Decoder is stopped or null, isRunning %d.", isDecoderRunning_.load());
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    auto inMem = audioDecoder_->GetInputBuffer(bufferIndex);
    if (inMem == nullptr) {
        DHLOGE("Get input buffer fail.");
        return ERR_DH_AUDIO_CODEC_INPUT;
    }
    if (inMem->GetSize() == INVALID_MEMORY_SIZE || static_cast<size_t>(inMem->GetSize()) < audioData->Size()) {
        DHLOGE("Input buffer size error. Memory size %d, data size %zu.",
            inMem->GetSize(), audioData->Size());
        return ERR_DH_AUDIO_CODEC_INPUT;
    }

    errno_t err = memcpy_s(inMem->GetBase(), inMem->GetSize(), audioData->Data(), audioData->Size());
    if (err != EOK) {
        DHLOGE("Copy input data fail. Error code %d. Memory size %d, data size %zu.",
            err, inMem->GetSize(), audioData->Size());
        return ERR_DH_AUDIO_BAD_OPERATE;
    }

    inputTimeStampUs_ = GetDecoderTimeStamp();
    Media::AVCodecBufferInfo bufferInfo = {inputTimeStampUs_, static_cast<int32_t>(audioData->Size()), 0};
    auto bufferFlag =
        bufferInfo.presentationTimeUs == 0 ? Media::AVCODEC_BUFFER_FLAG_CODEC_DATA : Media::AVCODEC_BUFFER_FLAG_NONE;
    DHLOGD("Queue input buffer. AVCodec Info: input time stamp %lld, data size %zu.",
        (long long)bufferInfo.presentationTimeUs, audioData->Size());
    int32_t ret = audioDecoder_->QueueInputBuffer(bufferIndex, bufferInfo, bufferFlag);
    if (ret != Media::MSERR_OK) {
        DHLOGE("Queue input buffer fail. Error code %d", ret);
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
    DHLOGD("Wait decoder output frames number is %d.", waitOutputCount_);
}

void AudioDecoder::ReduceWaitDecodeCnt()
{
    std::lock_guard<std::mutex> countLock(mtxCnt_);
    if (waitOutputCount_ <= 0) {
        DHLOGE("Wait decoder output count %d.", waitOutputCount_);
    }
    waitOutputCount_--;
    DHLOGD("Wait decoder output frames number is %d.", waitOutputCount_);
}

void AudioDecoder::OnInputBufferAvailable(uint32_t index)
{
    std::lock_guard<std::mutex> countLock(mtxData_);
    while (bufIndexQueue_.size() > AUDIO_DECODER_QUEUE_MAX) {
        DHLOGE("Index queue overflow.");
        bufIndexQueue_.pop();
    }

    bufIndexQueue_.push(index);
    decodeCond_.notify_all();
}

void AudioDecoder::OnOutputBufferAvailable(uint32_t index, Media::AVCodecBufferInfo info,
    Media::AVCodecBufferFlag flag)
{
    if (!isDecoderRunning_.load() || audioDecoder_ == nullptr) {
        DHLOGE("Decoder is stopped or null, isRunning %d.", isDecoderRunning_.load());
        return;
    }

    auto outMem = audioDecoder_->GetOutputBuffer(index);
    if (outMem == nullptr) {
        DHLOGE("Get output buffer fail. index %u.", index);
        return;
    }
    if (info.size <= 0 || info.size > outMem->GetSize()) {
        DHLOGE("Codec output info error. AVCodec info: size %d, memory size %d.",
            info.size, outMem->GetSize());
        return;
    }

    auto outBuf = std::make_shared<AudioData>(static_cast<size_t>(info.size));
    errno_t err = memcpy_s(outBuf->Data(), outBuf->Size(), outMem->GetBase(), static_cast<size_t>(info.size));
    if (err != EOK) {
        DHLOGE("Copy output data fail. Error code %d. Buffer Size %zu, AVCodec info: size %d.",
            err, outBuf->Size(), info.size);
        return;
    }
    outBuf->SetInt64("timeUs", info.presentationTimeUs);
    outputTimeStampUs_ = info.presentationTimeUs;
    DHLOGD("Get output buffer. AVCodec info: output time stamp %lld, data size %zu.",
        (long long)info.presentationTimeUs, outBuf->Size());

    ReduceWaitDecodeCnt();
    err = DecodeDone(outBuf);
    if (err != DH_SUCCESS) {
        DHLOGE("Decode done fail. Error code: %d.", err);
        return;
    }

    err = audioDecoder_->ReleaseOutputBuffer(index);
    if (err != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Release output buffer fail. Error code: %d, index %u.", err, index);
    }
}

void AudioDecoder::OnOutputFormatChanged(const Media::Format &format)
{
    if (format.GetFormatMap().empty()) {
        DHLOGE("The first changed output frame format is null.");
        return;
    }
    outputFormat_ = format;
}

void AudioDecoder::OnError(const AudioEvent &event)
{
    DHLOGE("Decoder error.");
    std::shared_ptr<IAudioCodecCallback> cbObj = codecCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("Codec callback is null.");
        return;
    }
    cbObj->OnCodecStateNotify(event);
}

int32_t AudioDecoder::DecodeDone(const std::shared_ptr<AudioData> &outputData)
{
    DHLOGD("Decode done.");
    std::shared_ptr<IAudioCodecCallback> cbObj = codecCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("Codec callback is null.");
        return ERR_DH_AUDIO_BAD_VALUE;
    }
    cbObj->OnCodecDataDone(outputData);
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS