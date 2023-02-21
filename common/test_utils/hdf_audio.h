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

#ifndef OHOS_HDF_AUDIO_H
#define OHOS_HDF_AUDIO_H

#include <thread>

#include "audio_types.h"
#include "audio_capturer.h"
#include "audio_renderer.h"
#include "audio_base.h"
#include "audio_buffer.h"

namespace OHOS {
namespace DistributedHardware {
class HDFAudioCaptureObj : public AudioObject {
public:
    HDFAudioCaptureObj();
    ~HDFAudioCaptureObj() override = default;
    int32_t Init(const AudioBufferInfo &info) override;
    void Release() override;
    void RunThread() override;
    void Start() override;
    void Stop() override;
    int32_t CaptureFrame(const int32_t time) override;
private:

    struct AudioDeviceDescriptor captureDesc_;
    struct AudioSampleAttributes captureAttr_;
};

class HDFAudioRenderObj : public AudioObject {
public:
    HDFAudioRenderObj();
    ~HDFAudioRenderObj() override = default;
    int32_t Init(const AudioBufferInfo &info) override;
    void Release() override;
    void RunThread() override;
    void Start() override;
    void Stop() override;
    int32_t CaptureFrame(const int32_t time) override;;
private:

    struct AudioDeviceDescriptor renderDesc_;
    struct AudioSampleAttributes renderAttr_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HDF_AUDIO_H