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

#include "cycle_test.h"

#include <iostream>

#include "local_audio.h"
#include "hdf_audio.h"
#include "daudio_errorcode.h"

using namespace std;

namespace OHOS {
namespace DistributedHardware {
void CycleTest::GetMode()
{
    cout << "Input audio cycle test mode: " << endl
         << "[1] Use Audio FrameWork play, Audio FrameWork capture." << endl
         << "[2] Use Audio HDF play, Audio HDF capture." << endl;
    int32_t mode;
    cin >> mode;
    cout << endl;
    if (mode != MODE_LR_LC && mode != MODE_HR_HC) {
        cout << "Mode is invalid." << endl;
        return;
    }
    mode_ = static_cast<CYCLE_MODE>(mode);

    cout << "Input speaker file path(/data/): ";
    cin >> spkFile_;
    cout << endl;
    cout << "Input mic file path(/data/): ";
    cin >> micFile_;
    cout << endl;

    string dir = "/data/";
    spkFile_ = dir + spkFile_;
    micFile_ = dir + micFile_;
    return;
}

int32_t CycleTest::Init()
{
    GetMode();
    switch (static_cast<CYCLE_MODE>(mode_)) {
        case MODE_INVALID:
            render_ = make_unique<AudioRenderObj>();
            capture_ = make_unique<AudioCaptureObj>();
            break;
        case MODE_LR_LC:
            render_ = make_unique<AudioRenderObj>();
            capture_ = make_unique<AudioCaptureObj>();
            break;
        case MODE_HR_HC:
            render_ = make_unique<HDFAudioRenderObj>();
            capture_ = make_unique<HDFAudioCaptureObj>();
            break;
        default:
            break;
    }

    bool ready = false;
    size_t pos = spkFile_.find(".pcm");
    if (pos != string::npos) {
        if (render_->ReadPcmFile(spkFile_, info_) != DH_SUCCESS) {
            return ERR_DH_AUDIO_FAILED;
        }
        ready = true;
    }
    pos = spkFile_.find(".wav");
    if (pos != string::npos) {
        if (render_->ReadWavFile(spkFile_, info_) != DH_SUCCESS) {
            return ERR_DH_AUDIO_FAILED;
        }
        ready = true;
    }
    if (ready != true) {
        cout << "File path is wrong." << endl;
        return ERR_DH_AUDIO_FAILED;
    }

    render_->Init(info_);
    capture_->Init(info_);
    return DH_SUCCESS;
}

int32_t CycleTest::Process()
{
    if (capture_ == nullptr || render_ == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }
    capture_->CaptureFrame(render_->GetPeriod());
    capture_->Start();
    render_->Start();
    capture_->Stop();
    capture_->SaveAudioData(micFile_);

    auto start = render_->GetBeepTime();
    auto stop = capture_->GetBeepTime();
    if (start.size() != stop.size()) {
        cout << "Record num is not equal(" << start.size() << " " << stop.size() << ")." << endl;
        return ERR_DH_AUDIO_FAILED;
    }

    for (size_t i = 0; i < start.size(); i++) {
        cout << "Send: " << start[i] << " Received: " << stop[i] << endl;
        cout << "Time is: " << stop[i] - start[i] << endl;
    }
    return DH_SUCCESS;
}

void CycleTest::Release()
{
    if (capture_ != nullptr) {
        capture_->Release();
    }

    if (render_ != nullptr) {
        render_->Release();
    }
}
} // namespace DistributedHardware
} // namespace OHOS