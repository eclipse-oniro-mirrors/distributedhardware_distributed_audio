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

using namespace std;
namespace OHOS {
namespace DistributedHardware {
AudioCaptureObj::AudioCaptureObj()
{
    opts_.capturerInfo.sourceType = OHOS::AudioStandard::SourceType::SOURCE_TYPE_MIC;
    opts_.streamInfo.encoding = OHOS::AudioStandard::AudioEncodingType::ENCODING_PCM;
    opts_.capturerInfo.capturerFlags = 0;
}

int32_t AudioCaptureObj::Init(const AudioBufferInfo &info)
{
    cout << "[1]Create local audio capturer." << endl;
    info_ = info;
    opts_.streamInfo.samplingRate = (OHOS::AudioStandard::AudioSamplingRate)info.sampleRate;
    opts_.streamInfo.channels = (OHOS::AudioStandard::AudioChannel)info.channel;
    opts_.streamInfo.format = (OHOS::AudioStandard::AudioSampleFormat)info.format;
    capturer_ = OHOS::AudioStandard::AudioCapturer::Create(opts_);
    if (capturer_ == nullptr) {
        cout << "[1]Create local audio capturer failed." << endl;
        return ERR_DH_AUDIO_NULLPTR;
    }
    cout << "[1]Create local audio capturer success." << endl;
    return DH_SUCCESS;
}

void AudioCaptureObj::Release()
{
    cout << "[4]Release local audio capturer." << endl;
    while (runThread_.joinable()) {
        runThread_.join();
    }
    if (capturer_ != nullptr) {
        capturer_->Release();
    }
    cout << "[4]Release local audio capturer success." << endl;
}

int32_t AudioCaptureObj::CaptureFrame(const int32_t time)
{
    info_.period = time;
    if (info_.period <= 0 || info_.period >= CAPTURE_TIME_MAX) {
        cout << "Capture time is invalid." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    info_.size = info_.sampleRate * info_.channel * BIT_FORMAT_16 * info_.period;
    info_.frames = FRAME_PER_SEC * info_.period;
    info_.sizePerFrame = info_.sampleRate * info_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    data_ = std::make_unique<AudioBuffer>(info_);
    return DH_SUCCESS;
}

void AudioCaptureObj::Start()
{
    cout << "[2]Start local capture thread." << endl;
    if (capturer_ == nullptr) {
        return;
    }
    capturer_->Start();
    runThread_ = std::thread(&AudioCaptureObj::RunThread, this);
    while (!runThread_.joinable()) {
    }
    cout << "[2]Start local capture thread success." << endl;
}

void AudioCaptureObj::Stop()
{
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    if (capturer_ != nullptr) {
        capturer_->Stop();
    }
    cout << "[3]Stop local capture thread success." << endl;
}

void AudioCaptureObj::RunThread()
{
    if (capturer_ == nullptr || data_ == nullptr || data_->Data() == nullptr) {
        return;
    }

    int32_t playIndex = 0;
    uint8_t *base = data_->Data();
    auto data = std::make_unique<AudioBuffer>(info_.sizePerFrame);
    while (playIndex < info_.frames) {
        int32_t readOffSet = 0;
        while (readOffSet < info_.sizePerFrame) {
            int32_t len = capturer_->Read(*(data->Data() + readOffSet), info_.sizePerFrame - readOffSet, true);
            readOffSet += len;
        }
        if (memcpy_s(base, data_->Size(), data->Data(), data->Size()) != EOK) {
            cout << "Copy buffer failed." << endl;
            break;
        }
        base += info_.sizePerFrame;
        playIndex++;
    }
    cout << "[*]Capture frame number is: " << info_.frames << endl;
}

AudioRenderObj::AudioRenderObj()
{
    opts_.streamInfo.encoding = OHOS::AudioStandard::AudioEncodingType::ENCODING_PCM;
    opts_.rendererInfo.contentType = OHOS::AudioStandard::ContentType::CONTENT_TYPE_MUSIC;
    opts_.rendererInfo.streamUsage = OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MEDIA;
    opts_.rendererInfo.rendererFlags = 0;
}

int32_t AudioRenderObj::Init(const AudioBufferInfo &info)
{
    cout << "[1]Create local audio render." << endl;

    info_ = info;
    info_.sizePerFrame = info_.sampleRate * info_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    info_.frames = info_.size / info_.sizePerFrame;
    info_.period = info_.frames / FRAME_PER_SEC;

    opts_.streamInfo.samplingRate = (OHOS::AudioStandard::AudioSamplingRate)info_.sampleRate;
    opts_.streamInfo.channels = (OHOS::AudioStandard::AudioChannel)info_.channel;
    opts_.streamInfo.format = (OHOS::AudioStandard::AudioSampleFormat)info_.format;
    render_ = OHOS::AudioStandard::AudioRenderer::Create(opts_);
    if (render_ == nullptr) {
        cout << "[1]Create local audio render failed." << endl;
        return ERR_DH_AUDIO_NULLPTR;
    }
    cout << "[1]Create local audio render success." << endl;
    return DH_SUCCESS;
}

void AudioRenderObj::Release()
{
    cout << "[4]Release local audio render." << endl;
    while (runThread_.joinable()) {
        runThread_.join();
    }
    if (render_ != nullptr) {
        render_->Release();
    }
    cout << "[4]Release local audio render success." << endl;
}

void AudioRenderObj::Start()
{
    cout << "[2]Start local play thread." << endl;
    if (render_ == nullptr) {
        return;
    }
    render_->Start();
    runThread_ = std::thread(&AudioRenderObj::RunThread, this);
    while (!runThread_.joinable()) {
    }
    cout << "[2]Start local play thread success." << endl;
}

void AudioRenderObj::Stop()
{
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    if (render_ != nullptr) {
        render_->Stop();
    }
    cout << "[2]Stop local play thread success." << endl;
}

void AudioRenderObj::RunThread()
{
    if (render_ == nullptr || data_ == nullptr || data_->Data() == nullptr) {
        return;
    }

    int32_t playIndex = 0;
    uint8_t *base = data_->Data();
    auto data = std::make_unique<AudioBuffer>(info_.sizePerFrame);
    while (playIndex < info_.frames) {
        if (memcpy_s(data->Data(), data->Size(), base, info_.sizePerFrame) != EOK) {
            cout << "Copy buffer failed." << endl;
            break;
        }
        int32_t writeOffSet = 0;
        while (writeOffSet < info_.sizePerFrame) {
            int32_t writeLen = render_->Write(data->Data() + writeOffSet, info_.sizePerFrame - writeOffSet);
            writeOffSet += writeLen;
        }
        base += info_.sizePerFrame;
        playIndex++;
    }
    cout << "[*]Render frame number is: " << info_.frames << endl;
}

int32_t AudioRenderObj::CaptureFrame(const int32_t time)
{
    (void) time;
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS