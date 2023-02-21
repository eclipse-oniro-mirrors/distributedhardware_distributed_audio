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

#ifndef OHOS_LOCAL_AUDIO_H
#define OHOS_LOCAL_AUDIO_H

#include <thread>

#include "audio_capturer.h"
#include "audio_renderer.h"
#include "audio_info.h"
#include "audio_base.h"

namespace OHOS {
namespace DistributedHardware {
class AudioCaptureObj : public AudioObject {
public:
    AudioCaptureObj();
    ~AudioCaptureObj() override = default;
    int32_t Init(const AudioBufferInfo &info) override;
    void Release() override;
    void RunThread() override;
    void Start() override;
    void Stop() override;
    int32_t CaptureFrame(const int32_t time) override;
private:
    OHOS::AudioStandard::AudioCapturerOptions opts_;
    std::unique_ptr<OHOS::AudioStandard::AudioCapturer> capturer_ = nullptr;
};

class AudioRenderObj : public AudioObject {
public:
    AudioRenderObj();
    ~AudioRenderObj() override = default;
    int32_t Init(const AudioBufferInfo &info) override;
    void Release() override;
    void RunThread() override;
    void Start() override;
    void Stop() override;
    int32_t CaptureFrame(const int32_t time) override;
private:

    OHOS::AudioStandard::AudioRendererOptions opts_;
    std::unique_ptr<OHOS::AudioStandard::AudioRenderer> render_ = nullptr;
};

} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_LOCAL_AUDIO_H