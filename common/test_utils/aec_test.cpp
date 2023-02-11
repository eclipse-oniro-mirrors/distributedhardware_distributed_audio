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

#include "aec_test.h"

#include <iostream>
#include "daudio_errorcode.h"

using namespace std;

namespace OHOS {
namespace DistributedHardware {
int32_t AecTest::Init()
{
    cout << "Input file path: ";
    string path;
    cin >> path;
    cout << endl;

    size_t pos = path.find(".pcm");
    if (pos != string::npos) {
        if (render_.ReadPcmFile(path, info_) != DH_SUCCESS) {
            return ERR_DH_AUDIO_FAILED;
        }
    } else {
        cout << "File path is wrong." << endl;
        return ERR_DH_AUDIO_FAILED;
    }

    render_.Init(info_);
    capture_.Init(info_);
    return DH_SUCCESS;
}

int32_t AecTest::Process()
{
    capture_.StartCaptureFrame(render_.GetPeriod());
    render_.StartRenderFrame();
    capture_.StopCaptureFrame();
    return DH_SUCCESS;
}

void AecTest::Release()
{
    render_.Release();
    capture_.Release();
}
} // namespace DistributedHardware
} // namespace OHOS