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
#include <string>

#include "audio_buffer.h"
#include "cycle_test.h"
#include "daudio_errorcode.h"
#include "local_audio.h"
#include "hdf_audio.h"

using namespace std;

const string CMD_QUIT = "quit";
const string CMD_QUIT_EXT = "0";
const string CMD_LOCAL_CAPTURE = "lmic";
const string CMD_LOCAL_CAPTURE_EXT = "20";
const string CMD_LOCAL_RENDER = "lspk";
const string CMD_LOCAL_RENDER_EXT = "21";
const string CMD_CYCLE_TEST = "cycle";
const string CMD_CYCLE_TEST_EXT = "22";
const string CMD_HDF_CAPTURE = "hmic";
const string CMD_HDF_CAPTURE_EXT = "23";
const string CMD_HDF_RENDER = "hspk";
const string CMD_HDF_RENDER_EXT = "24";
namespace OHOS {
namespace DistributedHardware {

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
            LocalCapture();
            continue;
        }

        if (cmd == CMD_LOCAL_RENDER || cmd == CMD_LOCAL_RENDER_EXT) {
            LocalRender();
            continue;
        }

        if (cmd == CMD_CYCLE_TEST || cmd == CMD_CYCLE_TEST_EXT) {
            AudioCycleTest();
            continue;
        }

        if (cmd == CMD_HDF_CAPTURE || cmd == CMD_HDF_CAPTURE_EXT) {
            HDFCapture();
            continue;
        }

        if (cmd == CMD_HDF_RENDER || cmd == CMD_HDF_RENDER_EXT) {
            HDFRender();
            continue;
        }
    }
}

void DAudioTestUtils::LocalCapture()
{
    cout << "[LocalCapture]" << endl;
    AudioCaptureObj capture;
    AudioBufferInfo info;
    int32_t res = capture.ReadAudioInfo(info);
    if (res != DH_SUCCESS) {
        return;
    }
    res = capture.Init(info);
    if (res != DH_SUCCESS) {
        return;
    }
    cout << "Input capture time(s): ";
    cin >> time_;
    cout << endl;
    capture.CaptureFrame(time_);
    capture.Start();
    capture.Stop();
    capture.SaveAudioData("/data/mic.pcm");
    capture.Release();
}

void DAudioTestUtils::LocalRender()
{
    cout << "[LocalRender]" << endl;
    AudioRenderObj render;
    AudioBufferInfo info;

    cout << "Input file path: ";
    cin >> path_;
    cout << endl;

    size_t pos = path_.find(".wav");
    if (pos != string::npos) {
        if (render.ReadWavFile(path_, info) != DH_SUCCESS) {
            return;
        }
    }
    pos = path_.find(".pcm");
    if (pos != string::npos) {
        if (render.ReadPcmFile(path_, info) != DH_SUCCESS) {
            return;
        }
    }

    int32_t res = render.Init(info);
    if (res != DH_SUCCESS) {
        return;
    }
    render.Start();
    render.Stop();
    render.Release();
}

void DAudioTestUtils::AudioCycleTest()
{
    cout << "[CycleTest]" << endl;
    CycleTest test;
    int32_t res = test.Init();
    if (res != DH_SUCCESS) {
        return;
    }
    test.Process();
    test.Release();
}

void DAudioTestUtils::HDFCapture()
{
    cout << "[HDFCapture]" << endl;
    HDFAudioCaptureObj capture;
    AudioBufferInfo info;
    int32_t res = capture.ReadAudioInfo(info);
    if (res != DH_SUCCESS) {
        return;
    }
    res = capture.Init(info);
    if (res != DH_SUCCESS) {
        return;
    }
    cout << "Input capture time(s): ";
    cin >> time_;
    cout << endl;
    capture.CaptureFrame(time_);
    capture.Start();
    capture.Stop();
    capture.SaveAudioData("/data/mic.pcm");
    capture.Release();
}

void DAudioTestUtils::HDFRender()
{
    cout << "[HDFRender]" << endl;
    HDFAudioRenderObj render;
    AudioBufferInfo info;

    cout << "Input file path: ";
    cin >> path_;
    cout << endl;

    size_t pos = path_.find(".wav");
    if (pos != string::npos) {
        if (render.ReadWavFile(path_, info) != DH_SUCCESS) {
            return;
        }
    }
    pos = path_.find(".pcm");
    if (pos != string::npos) {
        if (render.ReadPcmFile(path_, info) != DH_SUCCESS) {
            return;
        }
    }

    int32_t res = render.Init(info);
    if (res != DH_SUCCESS) {
        return;
    }
    render.Start();
    render.Stop();
    render.Release();
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