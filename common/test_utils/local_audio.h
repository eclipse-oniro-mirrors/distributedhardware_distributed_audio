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
class LocalAudio {
public:
    LocalAudio() = default;
    ~LocalAudio() = default;

    int32_t InitCapture();
    void ReleaseCapture();
    int32_t CaptureFrame();
    int32_t InitRender();
    void ReleaseRender();
    int32_t RenderFrame();

private:
    void CaptureThread();
    void RenderThread();
    int32_t ReadWavFile(const std::string &path);
    int32_t ReadPcmFile(const std::string &path);
    int32_t CmdReadInt();
    int32_t ReadInt32(FILE *fd);
    int32_t ReadInt16(FILE *fd);
    int32_t InputAudioInfo(AudioBufferInfo &info);

    AudioBufferInfo cInfo_;
    AudioBufferInfo rInfo_;
    std::thread captureThread_;
    std::thread renderThread_;

    OHOS::AudioStandard::AudioCapturerOptions capturerOpts_;
    OHOS::AudioStandard::AudioRendererOptions rendererOpts_;
    std::unique_ptr<OHOS::AudioStandard::AudioCapturer> capturer_ = nullptr;
    std::unique_ptr<OHOS::AudioStandard::AudioRenderer> renderer_ = nullptr;
    std::unique_ptr<AudioBuffer> micData_ = nullptr;
    std::unique_ptr<AudioBuffer> spkData_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_LOCAL_AUDIO_H