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

#ifndef OHOS_AUDIO_DATA_H
#define OHOS_AUDIO_DATA_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
typedef struct {
    int32_t sampleRate;
    int32_t channel;
    int32_t format;
    int32_t size;
    int32_t frames;
    int32_t period;
    int32_t sizePerFrame;
    std::string filePath;
} AudioBufferInfo;

class AudioBuffer {
public:
    AudioBuffer() = default;
    ~AudioBuffer();
    explicit AudioBuffer(const int32_t size);
    explicit AudioBuffer(const AudioBufferInfo &info);

    int32_t Size() const;
    uint8_t *Data() const;
    AudioBufferInfo GetInfo()
    {
        return param_;
    }

    int32_t WirteBufferToFile(const std::string &path);

private:
    AudioBuffer(const AudioBuffer &) = delete;
    AudioBuffer &operator = (const AudioBuffer &) = delete;

    AudioBufferInfo param_;
    uint8_t *data_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AUDIO_DATA_H