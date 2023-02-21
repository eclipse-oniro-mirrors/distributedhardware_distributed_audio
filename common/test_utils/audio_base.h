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

#ifndef OHOS_AUDIO_BASE_H
#define OHOS_AUDIO_BASE_H

#include <string>
#include <thread>
#include <vector>

#include "audio_buffer.h"

namespace OHOS {
namespace DistributedHardware {

constexpr int32_t FRAME_PER_SEC = 50;
constexpr int32_t BIT_FORMAT_16 = 2;
constexpr int32_t SECOND_ONE = 1;
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t MICROSECONDS_PER_MILLISECOND = 1000;
constexpr int32_t CAPTURE_TIME_MAX = 600;
constexpr int32_t WAV_SIZE_OFFSET = 36;

class AudioObject {
public:
    AudioObject() = default;
    virtual ~AudioObject() = default;
    virtual int32_t Init(const AudioBufferInfo &info) = 0;
    virtual void Release() = 0;
    virtual void RunThread() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual int32_t CaptureFrame(const int32_t time) = 0;
    int32_t ReadAudioInfo(AudioBufferInfo &info);
    int32_t ReadWavFile(const std::string &path, AudioBufferInfo &info);
    int32_t ReadPcmFile(const std::string &path, AudioBufferInfo &info);
    int32_t SaveAudioData(const std::string &path);
    std::shared_ptr<AudioBuffer> GetData()
    {
        return data_;
    }
    int32_t GetPeriod()
    {
        return info_.period;
    }
    std::vector<int64_t> GetBeepTime()
    {
        return beepTime_;
    }

protected:
    AudioBufferInfo info_;
    std::thread runThread_;
    std::shared_ptr<AudioBuffer> data_ = nullptr;
    std::vector<int64_t> beepTime_;

private:
    int32_t CmdReadInt();
    int32_t ReadInt32(FILE *fd);
    int32_t ReadInt16(FILE *fd);
};

int32_t GetEnvNoise(const std::shared_ptr<AudioBuffer> &data);
bool IsFrameHigh(const int16_t *data, const int32_t size, int32_t threshhold);
int64_t RecordBeepTime(const uint8_t *base, const AudioBufferInfo &info, bool &status);
int64_t GetNowTimeUs();
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AUDIO_BASE_H