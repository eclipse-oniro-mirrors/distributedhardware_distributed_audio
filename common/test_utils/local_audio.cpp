/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "local_audio.h"

#include <iostream>

#include <securec.h>
#include "daudio_errorcode.h"

#define DEFAULT_MIC_PATH "/data/mic.pcm"

constexpr int32_t FRAME_PER_SEC = 50;
constexpr int32_t BIT_FORMAT_16 = 2;
constexpr int32_t SECOND_ONE = 1;
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t MICROSECONDS_PER_MILLISECOND = 1000;
constexpr int32_t CAPTURE_TIME_MAX = 600;
constexpr int32_t WAV_SIZE_OFFSET = 36;

constexpr int32_t OFFSET_1 = 256;
constexpr int32_t OFFSET_2 = 65536;
constexpr int32_t OFFSET_3 = 16777216;
constexpr int32_t INT8_SIZE = 1;
constexpr int32_t INT16_SIZE = 2;
constexpr int32_t INT32_SIZE = 4;

using namespace std;
namespace OHOS {
namespace DistributedHardware {
int32_t AudioObject::CmdReadInt()
{
    int32_t cmd = 0;
    cin >> cmd;
    cout << endl;
    return cmd;
}

int32_t AudioObject::ReadAudioInfo(AudioBufferInfo &info)
{
    constexpr int32_t cmd1 = 1;
    constexpr int32_t cmd2 = 2;
    cout << "Input sample rate: " << endl << "[1] 16000" << endl << "[2] 48000" << endl;
    int32_t cmd = CmdReadInt();
    if (cmd == cmd1) {
        info.sampleRate = (int32_t)OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_16000;
    } else if (cmd == cmd2) {
        info.sampleRate = (int32_t)OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_48000;
    } else {
        cout << "Sample rate not support." << endl;
        return ERR_DH_AUDIO_FAILED;
    }

    cout << "Input channel number: " << endl << "[1] 1Ch" << endl << "[2] 2Ch" << endl;
    cmd = CmdReadInt();
    if (cmd == cmd1) {
        info.channel = (int32_t)OHOS::AudioStandard::AudioChannel::MONO;
    } else if (cmd == cmd2) {
        info.channel = (int32_t)OHOS::AudioStandard::AudioChannel::STEREO;
    } else {
        cout << "Channel number not support." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    info.format = (int32_t)OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE;

    cout << "======================================" << endl
    << "information:" << endl
    << "sample_rate: " << info.sampleRate << endl
    << "channels: " << info.channel << endl
    << "format: " << info.format << endl
    << "======================================" << endl;
    return DH_SUCCESS;
}

int32_t AudioCaptureObj::Init(const AudioBufferInfo &info)
{
    cout << "Create local audio capturer." << endl;
    info_ = info;
    opts_.streamInfo.samplingRate = (OHOS::AudioStandard::AudioSamplingRate)info.sampleRate;
    opts_.streamInfo.channels = (OHOS::AudioStandard::AudioChannel)info.channel;
    opts_.streamInfo.format = (OHOS::AudioStandard::AudioSampleFormat)info.format;
    opts_.capturerInfo.sourceType = OHOS::AudioStandard::SourceType::SOURCE_TYPE_MIC;
    opts_.streamInfo.encoding = OHOS::AudioStandard::AudioEncodingType::ENCODING_PCM;
    opts_.capturerInfo.capturerFlags = 0;
    capturer_ = OHOS::AudioStandard::AudioCapturer::Create(opts_);
    cout << "Create local audio capturer success." << endl;
    return DH_SUCCESS;
}

void AudioCaptureObj::Release()
{
    while (runThread_.joinable()) {
        runThread_.join();
    }
    if (capturer_ == nullptr) {
        return;
    }
    capturer_->Stop();
    capturer_->Release();
}

int32_t AudioCaptureObj::CaptureFrame(const int32_t time)
{
    if (capturer_ == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }

    info_.period = time;
    if (info_.period <= 0 || info_.period >= CAPTURE_TIME_MAX) {
        cout << "Capture time is invalid." << endl;
        return ERR_DH_AUDIO_FAILED;
    }

    info_.size = info_.sampleRate * info_.channel * BIT_FORMAT_16 * info_.period;
    data_ = std::make_unique<AudioBuffer>(info_);

    capturer_->Start();
    runThread_ = std::thread(&AudioCaptureObj::RunThread, this);
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    int32_t res = data_->WirteBufferToFile(DEFAULT_MIC_PATH);
    cout << "Save mic file in: " << DEFAULT_MIC_PATH << endl;
    if (res != DH_SUCCESS) {
        cout << "Write audio file failed." << endl;
    }
    capturer_->Stop();

    cout << "Local audio capture frame end." << endl;
    return DH_SUCCESS;
}

int32_t AudioCaptureObj::StartCaptureFrame(const int32_t time)
{
    if (capturer_ == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }

    info_.period = time;
    if (info_.period <= 0 || info_.period >= CAPTURE_TIME_MAX) {
        cout << "Capture time is invalid." << endl;
        return ERR_DH_AUDIO_FAILED;
    }

    info_.size = info_.sampleRate * info_.channel * BIT_FORMAT_16 * info_.period;
    data_ = std::make_unique<AudioBuffer>(info_);

    capturer_->Start();
    runThread_ = std::thread(&AudioCaptureObj::RunThread, this);
    return DH_SUCCESS;
}

int32_t AudioCaptureObj::StopCaptureFrame()
{
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    int32_t res = data_->WirteBufferToFile(DEFAULT_MIC_PATH);
    cout << "Save mic file in: " << DEFAULT_MIC_PATH << endl;
    if (res != DH_SUCCESS) {
        cout << "Write audio file failed." << endl;
    }

    return DH_SUCCESS;
}

void AudioCaptureObj::RunThread()
{
    if (capturer_ == nullptr || data_ == nullptr) {
        return;
    }

    info_.frames = FRAME_PER_SEC * info_.period;
    info_.sizePerFrame = info_.sampleRate * info_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    int32_t playIndex = 0;
    uint8_t *base = data_->Data();
    auto data = std::make_unique<AudioBuffer>(info_.sizePerFrame);
    while (playIndex < info_.frames) {
        int32_t readOffSet = 0;
        while (readOffSet < info_.sizePerFrame) {
            int32_t len = capturer_->Read(*(data->Data() + readOffSet), info_.sizePerFrame - readOffSet, true);
            readOffSet += len;
        }
        memcpy_s(base, data_->Size(), data->Data(), data->Size());
        base += info_.sizePerFrame;
        playIndex++;
    }
    cout << "Capture frame number is: " << info_.frames << endl;
}

int32_t AudioRenderObj::Init(const AudioBufferInfo &info)
{
    cout << "Create local audio render." << endl;

    info_ = info;
    info_.sizePerFrame = info_.sampleRate * info_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    info_.frames = info_.size / info_.sizePerFrame;
    info_.period = info_.frames / FRAME_PER_SEC;

    opts_.streamInfo.samplingRate = (OHOS::AudioStandard::AudioSamplingRate)info_.sampleRate;
    opts_.streamInfo.channels = (OHOS::AudioStandard::AudioChannel)info_.channel;
    opts_.streamInfo.format = (OHOS::AudioStandard::AudioSampleFormat)info_.format;
    opts_.streamInfo.encoding = OHOS::AudioStandard::AudioEncodingType::ENCODING_PCM;
    opts_.rendererInfo.contentType = OHOS::AudioStandard::ContentType::CONTENT_TYPE_MUSIC;
    opts_.rendererInfo.streamUsage = OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MEDIA;
    opts_.rendererInfo.rendererFlags = 0;
    render_ = OHOS::AudioStandard::AudioRenderer::Create(opts_);
    cout << "Create local audio render success." << endl;
    return DH_SUCCESS;
}

void AudioRenderObj::Release()
{
    while (runThread_.joinable()) {
        runThread_.join();
    }
    if (render_ == nullptr) {
        return;
    }
    render_->Stop();
    render_->Release();
}

int32_t AudioRenderObj::RenderFrame()
{
    if (render_ == nullptr) {
        cout << "Render is null" << endl;
        return ERR_DH_AUDIO_NULLPTR;
    }
    render_->Start();
    runThread_ = std::thread(&AudioRenderObj::RunThread, this);

    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    cout << "Local audio render frame end." << endl;
    return DH_SUCCESS;
}

int32_t AudioRenderObj::StartRenderFrame()
{
    if (render_ == nullptr) {
        cout << "Render is null" << endl;
        return ERR_DH_AUDIO_NULLPTR;
    }
    render_->Start();
    runThread_ = std::thread(&AudioRenderObj::RunThread, this);
    return DH_SUCCESS;
}

int32_t AudioRenderObj::StopRenderFrame()
{
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    return DH_SUCCESS;
}

void AudioRenderObj::RunThread()
{
    if (render_ == nullptr || data_ == nullptr) {
        return;
    }

    int32_t playIndex = 0;
    uint8_t *base = data_->Data();
    auto data = std::make_unique<AudioBuffer>(info_.sizePerFrame);
    while (playIndex < info_.frames) {
        memcpy_s(data->Data(), data->Size(), base, info_.sizePerFrame);
        int32_t writeOffSet = 0;
        while (writeOffSet < info_.sizePerFrame) {
            int32_t writeLen = render_->Write(data->Data() + writeOffSet, info_.sizePerFrame - writeOffSet);
            writeOffSet += writeLen;
        }
        base += info_.sizePerFrame;
        playIndex++;
    }
    cout << "Render frame number is: " << info_.frames << endl;
}

int32_t AudioRenderObj::ReadWavFile(const std::string &path, AudioBufferInfo &info)
{
    FILE *fd = fopen(path.c_str(), "rb");
    if (fd == nullptr) {
        cout << "File not exist." << endl;
        return ERR_DH_AUDIO_FAILED;
    }

    constexpr int32_t WAV_OFFSET_4 = 4;
    constexpr int32_t WAV_OFFSET_22 = 22;
    constexpr int32_t WAV_OFFSET_24 = 24;
    constexpr int32_t WAV_OFFSET_44 = 44;

    rewind(fd);
    fseek(fd, WAV_OFFSET_4, SEEK_SET);
    info.size = ReadInt32(fd) - WAV_SIZE_OFFSET;
    fseek(fd, WAV_OFFSET_22, SEEK_SET);
    info.channel = ReadInt16(fd);
    fseek(fd, WAV_OFFSET_24, SEEK_SET);
    info.sampleRate = ReadInt32(fd);
    info.format = (int32_t)OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE;
    cout << "======================================" << endl
        << path << " information:" << endl
        << "channels: " << info.channel << endl
        << "sample_rate: " << info.sampleRate << endl
        << "length: " << info.size << endl
        << "======================================" << endl;

    data_ = std::make_unique<AudioBuffer>(info);
    fseek(fd, WAV_OFFSET_44, SEEK_SET);
    fread(data_->Data(), 1, info.size, fd);
    fclose(fd);

    return DH_SUCCESS;
}

int32_t AudioRenderObj::ReadPcmFile(const std::string &path, AudioBufferInfo &info)
{
    FILE *fd = fopen(path.c_str(), "rb");
    if (fd == nullptr) {
        cout << "File not exist." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    fseek(fd, 0, SEEK_END);
    info.size = ftell(fd);
    rewind(fd);

    int32_t res = ReadAudioInfo(info);
    if (res != DH_SUCCESS) {
        return res;
    }
    data_ = std::make_unique<AudioBuffer>(info);
    fread(data_->Data(), 1, info.size, fd);
    fclose(fd);
    return DH_SUCCESS;
}

int32_t AudioRenderObj::ReadInt32(FILE *fd)
{
    char tmp[INT32_SIZE];
    fread(tmp, INT8_SIZE, INT32_SIZE, fd);
    int32_t res =
        (int32_t)tmp[0] + (int32_t)tmp[1] * OFFSET_1 + (int32_t)tmp[2] * OFFSET_2 + (int32_t)tmp[3] * OFFSET_3;
    return res;
}

int32_t AudioRenderObj::ReadInt16(FILE *fd)
{
    char tmp[INT16_SIZE];
    fread(tmp, INT8_SIZE, INT16_SIZE, fd);
    int32_t res = (int32_t)tmp[0] + (int32_t)tmp[1] * OFFSET_1;
    return res;
}
} // namespace DistributedHardware
} // namespace OHOS