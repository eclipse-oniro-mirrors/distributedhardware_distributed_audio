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

#include "audio_buffer.h"

#include <iostream>

#include "daudio_errorcode.h"

namespace OHOS {
namespace DistributedHardware {
AudioBuffer::AudioBuffer(const int32_t size)
{
    if (size > 0) {
        data_ = new (std::nothrow) uint8_t[size] {0};
        if (data_ != nullptr) {
            param_.size = size;
            param_.channel = 0;
            param_.format = 0;
            param_.sampleRate = 0;
        }
    }
}

AudioBuffer::AudioBuffer(const AudioBufferInfo &info)
{
    if (info.size > 0) {
        param_ = info;
        data_ = new (std::nothrow) uint8_t[info.size] { 0 };
    }
}

int32_t AudioBuffer::Size() const
{
    return param_.size;
}

uint8_t *AudioBuffer::Data() const
{
    return data_;
}

AudioBuffer::~AudioBuffer()
{
    if (data_ != nullptr) {
        delete[] data_;
        data_ = nullptr;
    }
    param_.size = 0;
}

int32_t AudioBuffer::WirteBufferToFile(const std::string &path)
{
    if (data_ == nullptr || param_.size == 0) {
        return ERR_DH_AUDIO_FAILED;
    }

    FILE *fp = fopen(path.c_str(), "wb");
    fwrite(data_, sizeof(uint8_t), param_.size, fp);
    fclose(fp);
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
