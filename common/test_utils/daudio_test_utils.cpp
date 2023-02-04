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

#include "daudio_test_utils.h"

#include <iostream>

#include "local_audio.h"

using namespace std;

namespace OHOS {
namespace DistributedHardware {

const string CMD_QUIT = "quit";
const string CMD_QUIT_EXT = "0";
const string CMD_LOCAL_CAPTURE = "lcap";
const string CMD_LOCAL_CAPTURE_EXT = "20";
const string CMD_LOCAL_RENDER = "lren";
const string CMD_LOCAL_RENDER_EXT = "21";

void DAudioTestUtils::DoAudioTest()
{
    bool running = true;
    string cmd = "";
    while (running) {
        cout << endl <<"Input test command: ";
        cin >> cmd;
        cout <<endl;

        if (cmd == CMD_QUIT || cmd == CMD_QUIT_EXT) {
            running = false;
            continue;
        }

        if (cmd == CMD_LOCAL_CAPTURE || cmd == CMD_LOCAL_CAPTURE_EXT) {
            LocalAudio capture;
            capture.InitCapture();
            capture.CaptureFrame();
            capture.ReleaseCapture();
        }

        if (cmd == CMD_LOCAL_RENDER || cmd == CMD_LOCAL_RENDER_EXT) {
            LocalAudio render;
            render.InitRender();
            render.RenderFrame();
            render.ReleaseRender();
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS

int32_t main()
{
    cout << "**********************************************************************************" << endl
         << "Distributed Audio Test Demo Bin." << endl
         << "**********************************************************************************" << endl;
    OHOS::DistributedHardware::DAudioTestUtils testBin;
    testBin.DoAudioTest();
    return 0;
}