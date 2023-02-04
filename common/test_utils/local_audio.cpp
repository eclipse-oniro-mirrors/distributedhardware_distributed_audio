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
int32_t LocalAudio::InitCapture()
{
    int32_t res = InputAudioInfo(cInfo_);
    if (res != DH_SUCCESS) {
        return res;
    }

    capturerOpts_.streamInfo.samplingRate = (OHOS::AudioStandard::AudioSamplingRate)cInfo_.sampleRate;
    capturerOpts_.streamInfo.channels = (OHOS::AudioStandard::AudioChannel)cInfo_.channel;
    capturerOpts_.streamInfo.format = (OHOS::AudioStandard::AudioSampleFormat)cInfo_.format;
    capturerOpts_.capturerInfo.sourceType = OHOS::AudioStandard::SourceType::SOURCE_TYPE_MIC;
    capturerOpts_.streamInfo.encoding = OHOS::AudioStandard::AudioEncodingType::ENCODING_PCM;
    capturerOpts_.capturerInfo.capturerFlags = 0;
    capturer_ = OHOS::AudioStandard::AudioCapturer::Create(capturerOpts_);
    cout << "Create local audio capturer success." << endl;
    return DH_SUCCESS;
}

void LocalAudio::ReleaseCapture()
{
    while (captureThread_.joinable()) {
        captureThread_.join();
    }
    if (capturer_ == nullptr) {
        return;
    }
    capturer_->Release();
}

int32_t LocalAudio::CaptureFrame()
{
    if (capturer_ == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }
    cout << "Input capture time(s): ";
    cin >> cInfo_.period;
    cout << endl;
    if (cInfo_.period <= 0 || cInfo_.period >= CAPTURE_TIME_MAX) {
        cout << "Capture time is invalid." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    cInfo_.frames = FRAME_PER_SEC * cInfo_.period;
    cInfo_.size = cInfo_.sampleRate * cInfo_.channel * BIT_FORMAT_16 * cInfo_.period;
    micData_ = std::make_unique<AudioBuffer>(cInfo_);
    capturer_->Start();
    captureThread_ = std::thread(&LocalAudio::CaptureThread, this);
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (cInfo_.period + SECOND_ONE));
    int32_t res = micData_->WirteBufferToFile(DEFAULT_MIC_PATH);
    cout << "Save mic file in: " << DEFAULT_MIC_PATH << endl;
    if (res != DH_SUCCESS) {
        cout << "Write audio file failed." << endl;
    }
    capturer_->Stop();

    cout << "Local audio capture frame end." << endl;
    return DH_SUCCESS;
}

void LocalAudio::CaptureThread()
{
    if (capturer_ == nullptr || micData_ == nullptr) {
        return;
    }
    cInfo_.sizePerFrame = cInfo_.sampleRate * cInfo_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    int32_t playIndex = 0;
    uint8_t *base = micData_->Data();
    auto data = std::make_unique<AudioBuffer>(cInfo_.sizePerFrame);
    while (playIndex < cInfo_.frames) {
        int32_t readOffSet = 0;
        while (readOffSet < cInfo_.sizePerFrame) {
            int32_t len = capturer_->Read(*(data->Data() + readOffSet), cInfo_.sizePerFrame - readOffSet, true);
            readOffSet += len;
        }
        memcpy_s(base, micData_->Size(), data->Data(), data->Size());
        base += cInfo_.sizePerFrame;
        playIndex++;
    }
}

int32_t LocalAudio::InitRender()
{
    cout << "Create local audio render." << endl;
    cout << "Input file path: ";
    cin >> rInfo_.filePath;
    cout << endl;
    size_t pos = rInfo_.filePath.find(".wav");
    if (pos != string::npos) {
        int32_t res = ReadWavFile(rInfo_.filePath);
        if (res != DH_SUCCESS) {
            return res;
        }
    }
    pos = rInfo_.filePath.find(".pcm");
    if (pos != string::npos) {
        int32_t res = ReadPcmFile(rInfo_.filePath);
        if (res != DH_SUCCESS) {
            return res;
        }
    }
    rInfo_.sizePerFrame = rInfo_.sampleRate * rInfo_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    rInfo_.frames = rInfo_.size / rInfo_.sizePerFrame;
    rInfo_.period = rInfo_.frames / FRAME_PER_SEC;

    rendererOpts_.streamInfo.samplingRate = (OHOS::AudioStandard::AudioSamplingRate)rInfo_.sampleRate;
    rendererOpts_.streamInfo.channels = (OHOS::AudioStandard::AudioChannel)rInfo_.channel;
    rendererOpts_.streamInfo.format = (OHOS::AudioStandard::AudioSampleFormat)rInfo_.format;
    rendererOpts_.streamInfo.encoding = OHOS::AudioStandard::AudioEncodingType::ENCODING_PCM;
    rendererOpts_.rendererInfo.contentType = OHOS::AudioStandard::ContentType::CONTENT_TYPE_MUSIC;
    rendererOpts_.rendererInfo.streamUsage = OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MEDIA;
    rendererOpts_.rendererInfo.rendererFlags = 0;
    renderer_ = OHOS::AudioStandard::AudioRenderer::Create(rendererOpts_);
    cout << "Create local audio render success." << endl;
    return DH_SUCCESS;
}

int32_t LocalAudio::RenderFrame()
{
    if (renderer_ == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }
    renderer_->Start();
    renderThread_ = std::thread(&LocalAudio::RenderThread, this);

    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (rInfo_.period + SECOND_ONE));
    cout << "Local audio render frame end." << endl;
    return DH_SUCCESS;
}

void LocalAudio::ReleaseRender()
{
    while (renderThread_.joinable()) {
        renderThread_.join();
    }
    if (renderer_ == nullptr) {
        return;
    }
    renderer_->Stop();
    renderer_->Release();
}

void LocalAudio::RenderThread()
{
    if (renderer_ == nullptr || spkData_ == nullptr) {
        return;
    }

    int32_t playIndex = 0;
    uint8_t *base = spkData_->Data();
    auto data = std::make_unique<AudioBuffer>(rInfo_.sizePerFrame);
    while (playIndex < rInfo_.frames) {
        memcpy_s(data->Data(), data->Size(), base, rInfo_.sizePerFrame);
        int32_t writeOffSet = 0;
        while (writeOffSet < rInfo_.sizePerFrame) {
            int32_t writeLen = renderer_->Write(data->Data() + writeOffSet, rInfo_.sizePerFrame - writeOffSet);
            writeOffSet += writeLen;
        }
        base += rInfo_.sizePerFrame;
        playIndex++;
    }
    cout << "Render frame number is: " << rInfo_.frames << endl;
}

int32_t LocalAudio::ReadWavFile(const std::string &path)
{
    cout << "Read wav file: " << path << endl;
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
    rInfo_.size = ReadInt32(fd) - WAV_SIZE_OFFSET;
    fseek(fd, WAV_OFFSET_22, SEEK_SET);
    rInfo_.channel = ReadInt16(fd);
    fseek(fd, WAV_OFFSET_24, SEEK_SET);
    rInfo_.sampleRate = ReadInt32(fd);
    rInfo_.format = (int32_t)OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE;
    cout << "======================================" << endl
        << path << " information:" << endl
        << "channels: " << rInfo_.channel << endl
        << "sample_rate: " << rInfo_.sampleRate << endl
        << "length: " << rInfo_.size << endl
        << "======================================" << endl;

    spkData_ = std::make_unique<AudioBuffer>(rInfo_);
    fseek(fd, WAV_OFFSET_44, SEEK_SET);
    fread(spkData_->Data(), 1, rInfo_.size, fd);
    fclose(fd);
    return DH_SUCCESS;
}

int32_t LocalAudio::ReadPcmFile(const std::string &path)
{
    cout << "Read pcm file: " << path << endl;
    FILE *fd = fopen(path.c_str(), "rb");
    if (fd == nullptr) {
        cout << "File not exist." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    fseek(fd, 0, SEEK_END);
    rInfo_.size = ftell(fd);
    rewind(fd);

    int32_t res = InputAudioInfo(rInfo_);
    if (res != DH_SUCCESS) {
        return res;
    }
    spkData_ = std::make_unique<AudioBuffer>(rInfo_);
    fread(spkData_->Data(), 1, rInfo_.size, fd);
    fclose(fd);
    return DH_SUCCESS;
}

int32_t LocalAudio::InputAudioInfo(AudioBufferInfo &info)
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
    <<" information:" << endl
    << "sample_rate: " << info.sampleRate << endl
    << "channels: " << info.channel << endl
    << "format: " << info.format << endl
    << "======================================" << endl;
    return DH_SUCCESS;
}

int32_t LocalAudio::CmdReadInt()
{
    int32_t cmd = 0;
    cin >> cmd;
    cout << endl;
    return cmd;
}

int32_t LocalAudio::ReadInt32(FILE *fd)
{
    char tmp[INT32_SIZE];
    fread(tmp, INT8_SIZE, INT32_SIZE, fd);
    int32_t res =
        (int32_t)tmp[0] + (int32_t)tmp[1] * OFFSET_1 + (int32_t)tmp[2] * OFFSET_2 + (int32_t)tmp[3] * OFFSET_3;
    return res;
}

int32_t LocalAudio::ReadInt16(FILE *fd)
{
    char tmp[INT16_SIZE];
    fread(tmp, INT8_SIZE, INT16_SIZE, fd);
    int32_t res = (int32_t)tmp[0] + (int32_t)tmp[1] * OFFSET_1;
    return res;
}
} // namespace DistributedHardware
} // namespace OHOS