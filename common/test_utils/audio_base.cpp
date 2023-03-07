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

#include "audio_base.h"

#include <iostream>
#include <sys/stat.h>
#include "audio_info.h"
#include "audio_buffer.h"
#include "daudio_errorcode.h"

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

int32_t AudioObject::ReadInt32(FILE *fd)
{
    char tmp[INT32_SIZE];
    fread(tmp, INT8_SIZE, INT32_SIZE, fd);
    int32_t res =
        (int32_t)tmp[0] + (int32_t)tmp[1] * OFFSET_1 + (int32_t)tmp[2] * OFFSET_2 + (int32_t)tmp[3] * OFFSET_3;
    return res;
}

int32_t AudioObject::ReadInt16(FILE *fd)
{
    char tmp[INT16_SIZE];
    fread(tmp, INT8_SIZE, INT16_SIZE, fd);
    int32_t res = (int32_t)tmp[0] + (int32_t)tmp[1] * OFFSET_1;
    return res;
}

int32_t AudioObject::ReadWavFile(const std::string &path, AudioBufferInfo &info)
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

int32_t AudioObject::ReadPcmFile(const std::string &path, AudioBufferInfo &info)
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
        fclose(fd);
        return res;
    }
    data_ = std::make_unique<AudioBuffer>(info);
    fread(data_->Data(), 1, info.size, fd);
    fclose(fd);
    return DH_SUCCESS;
}

int32_t AudioObject::SaveAudioData(const std::string &path)
{
    if (data_ == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }
    int32_t res = data_->WirteBufferToFile(path);
    cout << "[*]Save file in: " << path << endl;
    if (res != DH_SUCCESS) {
        cout << "Write audio file failed." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    return DH_SUCCESS;
}

int32_t GetEnvNoise(const std::shared_ptr<AudioBuffer> &data)
{
    int32_t max = 0;
    for (int32_t i = 0; i < data->GetInfo().size / (int32_t)sizeof(int16_t); i++) {
        int16_t f = abs((reinterpret_cast<int16_t *>data->Data())[i]);
        if (f > max) {
            max = f;
        }
    }
    return max;
}

bool IsFrameHigh(const int16_t *data, const int32_t size, int32_t threshhold)
{
    int32_t max = 0;
    for (int32_t i = 0; i < size; i++) {
        int16_t f = abs(data[i]);
        if (f > max) {
            max = f;
        }
    }
    return (max >= threshhold ? true : false);
}

int64_t GetNowTimeUs()
{
    std::chrono::microseconds nowUs =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
    return nowUs.count();
}

int64_t RecordBeepTime(const uint8_t *base, const AudioBufferInfo &info, bool &status)
{
    int32_t threshhold = 10000;
    if (IsFrameHigh(reinterpret_cast<int16_t *>base, info.sizePerFrame / sizeof(int16_t), threshhold) == true &&
        status == true) {
        status = false;
        return GetNowTimeUs();
    } else if (IsFrameHigh(reinterpret_cast<int16_t *>base, info.sizePerFrame / sizeof(int16_t), threshhold) == false) {
        status = true;
    }

    return 0;
}
} // namespace DistributedHardware
} // namespace OHOS