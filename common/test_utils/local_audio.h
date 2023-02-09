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
#include "audio_buffer.h"

namespace OHOS {
namespace DistributedHardware {
class AudioObject {
public:
    AudioObject() = default;
    ~AudioObject() = default;
    virtual int32_t Init(const AudioBufferInfo &info) = 0;
    virtual void Release() = 0;
    virtual void RunThread() = 0;
    int32_t ReadAudioInfo(AudioBufferInfo &info);

protected:
    AudioBufferInfo info_;
    std::thread runThread_;
    std::unique_ptr<AudioBuffer> data_ = nullptr;

private:
    int32_t CmdReadInt();
};

class AudioCaptureObj : public AudioObject {
public:
    int32_t Init(const AudioBufferInfo &info) override;
    void Release() override;
    void RunThread() override;
    int32_t CaptureFrame(const int32_t time);

private:
    OHOS::AudioStandard::AudioCapturerOptions opts_;
    std::unique_ptr<OHOS::AudioStandard::AudioCapturer> capturer_ = nullptr;
};


class AudioRenderObj : public AudioObject {
public:
    int32_t Init(const AudioBufferInfo &info) override;
    void Release() override;
    void RunThread() override;
    int32_t RenderFrame();
    int32_t ReadWavFile(const std::string &path, AudioBufferInfo &info);
    int32_t ReadPcmFile(const std::string &path, AudioBufferInfo &info);

private:
    int32_t ReadInt32(FILE *fd);
    int32_t ReadInt16(FILE *fd);

    OHOS::AudioStandard::AudioRendererOptions opts_;
    std::unique_ptr<OHOS::AudioStandard::AudioRenderer> render_ = nullptr;
};

} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_LOCAL_AUDIO_H