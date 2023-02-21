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

#include "hdf_audio.h"

#include <dlfcn.h>
#include <iostream>
#include <securec.h>
#include <string>
#include <thread>

#include "audio_adapter.h"
#include "audio_manager.h"
#include "audio_types.h"
#include "daudio_errorcode.h"

using namespace std;

namespace OHOS {
namespace DistributedHardware {

static struct AudioManager *g_audioManager;
static struct AudioAdapter *g_audioAdapter;
static struct AudioCapture *g_audioCapture;
static struct AudioRender *g_audioRender;
static bool g_micInUse = false;
static bool g_spkInUse = false;
static void *g_handle = nullptr;

static int32_t GetAudioManager()
{
    struct AudioManager *(*func)() = nullptr;
    string audioMgrPath = "/system/lib64/libhdi_audio_client.z.so";
    g_handle = dlopen(audioMgrPath.c_str(), RTLD_LAZY);
    if (g_handle == nullptr) {
        cout << "Open " << audioMgrPath << " failed." <<endl;
        return ERR_DH_AUDIO_FAILED;
    }
    func = (struct AudioManager *(*)())(dlsym(g_handle, "GetAudioManagerFuncs"));
    if (func == nullptr) {
        cout << "Dlsym GetAudioManagerFuncs failed." <<endl;
        return ERR_DH_AUDIO_FAILED;
    }
    g_audioManager = func();
    if (g_audioManager == nullptr) {
        cout << "Get audio manager failed." <<endl;
        return ERR_DH_AUDIO_FAILED;
    }
    return DH_SUCCESS;
}

static int32_t GetAdapter()
{
    struct AudioAdapterDescriptor *descs = nullptr;
    int32_t size = 0;
    constexpr int32_t adpMaxNum = 5;
    int32_t ret = g_audioManager->GetAllAdapters(g_audioManager, &descs, &size);
    if (size > adpMaxNum || size == 0 || descs == nullptr || ret != 0) {
        cout << "Get audio adapters failed." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    struct AudioAdapterDescriptor *primaryDesc = nullptr;
    for (int32_t index = 0; index < size; index++) {
        auto desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
        if (!strcmp(desc->adapterName, "primary")) {
            primaryDesc = desc;
            break;
        }
    }
    if (primaryDesc == nullptr) {
        cout << "Find primary adapter failed." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    ret = g_audioManager->LoadAdapter(g_audioManager, primaryDesc, &g_audioAdapter);
    if (ret != DH_SUCCESS || g_audioAdapter == nullptr) {
        cout << "Load primary adapter failed." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    return DH_SUCCESS;
}

static void ReleaseHDFAudioDevice()
{
    if (g_micInUse != false && g_spkInUse != false) {
        return;
    }
    if (g_audioManager != nullptr) {
        g_audioManager->UnloadAdapter(g_audioManager, g_audioAdapter);
    }
    g_audioManager = nullptr;
    g_audioAdapter = nullptr;
    (void)dlclose(g_handle);
}

static int32_t InitHDFAudioDevice()
{
    if (g_micInUse != false || g_spkInUse != false) {
        return DH_SUCCESS;
    }

    if (GetAudioManager() != DH_SUCCESS) {
        ReleaseHDFAudioDevice();
        return ERR_DH_AUDIO_FAILED;
    }

    if (GetAdapter() != DH_SUCCESS) {
        ReleaseHDFAudioDevice();
        return ERR_DH_AUDIO_FAILED;
    }
    return DH_SUCCESS;
}

HDFAudioCaptureObj::HDFAudioCaptureObj()
{
    int32_t period = 1024;
    int32_t streamId = 1;
    captureDesc_.pins = AudioPortPin::PIN_IN_MIC;
    captureDesc_.desc = nullptr;
    captureAttr_.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    captureAttr_.interleaved = true;
    captureAttr_.streamId = streamId;
    captureAttr_.type = AUDIO_IN_MEDIA;
    captureAttr_.period = period;
    captureAttr_.isBigEndian = false;
    captureAttr_.isSignedData = true;
    captureAttr_.stopThreshold = 0x7fffffff;
}

int32_t HDFAudioCaptureObj::Init(const AudioBufferInfo &info)
{
    cout << "[1]Create hdf audio capture." << endl;
    if (InitHDFAudioDevice() != DH_SUCCESS) {
        return ERR_DH_AUDIO_FAILED;
    }

    info_ = info;
    captureAttr_.channelCount = info.channel;
    captureAttr_.sampleRate = info.sampleRate;
    int32_t ret = g_audioAdapter->CreateCapture(g_audioAdapter, &captureDesc_, &captureAttr_, &g_audioCapture);
    if (ret != DH_SUCCESS || g_audioCapture == nullptr) {
        cout << "[1]CreateCapture failed, ret: " << ret << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    g_micInUse = true;
    cout << "[1]Create hdf audio capture success."<< endl;
    return DH_SUCCESS;
}

void HDFAudioCaptureObj::Release()
{
    cout << "[4]Release hdf audio capture." << endl;
    while (runThread_.joinable()) {
        runThread_.join();
    }
    if (g_audioAdapter != nullptr) {
        g_audioAdapter->DestroyCapture(g_audioAdapter, g_audioCapture);
    }
    g_micInUse = false;
    ReleaseHDFAudioDevice();
    cout << "[4]Release hdf audio capture success." << endl;
}

int32_t HDFAudioCaptureObj::CaptureFrame(const int32_t time)
{
    if (g_audioCapture == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }
    info_.period = time;
    if (info_.period <= 0 || info_.period >= CAPTURE_TIME_MAX) {
        cout << "Capture time is invalid." << endl;
        return ERR_DH_AUDIO_FAILED;
    }
    info_.size = info_.sampleRate * info_.channel * BIT_FORMAT_16 * info_.period;
    info_.frames = FRAME_PER_SEC * info_.period;
    info_.sizePerFrame = info_.sampleRate * info_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    data_ = make_unique<AudioBuffer>(info_);

    return DH_SUCCESS;
}

void HDFAudioCaptureObj::RunThread()
{
    if (g_audioCapture == nullptr || data_ == nullptr || data_->Data() == nullptr) {
        cout << "Capture or data is null." << endl;
        return;
    }

    int32_t playIndex = 0;
    uint64_t size = 0;
    uint8_t *base = data_->Data();
    auto data = make_unique<AudioBuffer>(info_.sizePerFrame);
    while (playIndex < info_.frames) {
        g_audioCapture->CaptureFrame(g_audioCapture, data->Data(), info_.sizePerFrame, &size);
        if (memcpy_s(base, data_->Size(), data->Data(), data->Size()) != EOK) {
            cout << "Copy buffer failed." << endl;
            break;
        }
        base += info_.sizePerFrame;
        playIndex++;
    }
    cout << "[*]Capture frame number is: " << info_.frames << endl;
}

void HDFAudioCaptureObj::Start()
{
    cout << "[2]Start hdf capture thread." << endl;
    if (g_audioCapture == nullptr) {
        return;
    }
    g_audioCapture->control.Start((AudioHandle)g_audioCapture);
    runThread_ = thread(&HDFAudioCaptureObj::RunThread, this);
    while (!runThread_.joinable()) {
    }
    cout << "[2]Start hdf capture thread success." << endl;
}

void HDFAudioCaptureObj::Stop()
{
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    if (g_audioCapture != nullptr) {
        g_audioCapture->control.Stop((AudioHandle)g_audioCapture);
    }
    cout << "[3]Stop hdf capture thread success." << endl;
}

HDFAudioRenderObj::HDFAudioRenderObj()
{
    int32_t period = 1024;
    int32_t streamId = 0;
    renderDesc_.pins = AudioPortPin::PIN_OUT_SPEAKER;
    renderDesc_.desc = nullptr;
    renderAttr_.interleaved = true;
    renderAttr_.streamId = streamId;
    renderAttr_.type = AUDIO_IN_MEDIA;
    renderAttr_.period = period;
    renderAttr_.isBigEndian = false;
    renderAttr_.isSignedData = true;
    renderAttr_.stopThreshold = 0x7fffffff;
    renderAttr_.silenceThreshold = 0;
    renderAttr_.format = AUDIO_FORMAT_TYPE_PCM_32_BIT;
}

int32_t HDFAudioRenderObj::Init(const AudioBufferInfo &info)
{
    cout << "[1]Create hdf audio render." << endl;
    if (InitHDFAudioDevice() != DH_SUCCESS) {
        return ERR_DH_AUDIO_FAILED;
    }

    renderAttr_.sampleRate = info.sampleRate;
    renderAttr_.channelCount = info.channel;
    info_ = info;
    info_.sizePerFrame = info_.sampleRate * info_.channel * BIT_FORMAT_16 / FRAME_PER_SEC;
    info_.frames = info_.size / info_.sizePerFrame;
    info_.period = info_.frames / FRAME_PER_SEC;
    int32_t ret = g_audioAdapter->CreateRender(g_audioAdapter, &renderDesc_, &renderAttr_, &g_audioRender);
    if (ret != DH_SUCCESS || g_audioRender == nullptr) {
        std::cout<<"[1]CreateRender failed, ret: "<< ret << std::endl;
        return ERR_DH_AUDIO_FAILED;
    }
    g_spkInUse = true;
    cout << "[1]Create hdf audio render success." << endl;
    return DH_SUCCESS;
}

void HDFAudioRenderObj::Release()
{
    cout << "[4]Release hdf audio render." << endl;
    while (runThread_.joinable()) {
        runThread_.join();
    }
    if (g_audioAdapter != nullptr) {
        g_audioAdapter->DestroyRender(g_audioAdapter, g_audioRender);
    }
    g_spkInUse = false;
    ReleaseHDFAudioDevice();
    cout << "[4]Release hdf audio render success." << endl;
}

void HDFAudioRenderObj::RunThread()
{
    if (g_audioRender == nullptr || data_ == nullptr || data_->Data() == nullptr) {
        return;
    }

    int32_t playIndex = 0;
    int32_t offset = 65535;
    uint64_t size = 0;
    int32_t dataSize = info_.sizePerFrame * sizeof(int32_t) / sizeof(int16_t);
    uint8_t *base = data_->Data();
    auto data = std::make_unique<AudioBuffer>(dataSize);
    while (playIndex < info_.frames) {
        for (int32_t i = 0; i < info_.sizePerFrame; i++) {
            ((int32_t *)data->Data())[i] = ((int16_t *)base)[i] * offset;
        }
        g_audioRender->RenderFrame(g_audioRender, data->Data(), dataSize, &size);
        base += info_.sizePerFrame;
        playIndex++;
    }
    cout << "[*]Render frame number is: " << info_.frames << endl;
}

void HDFAudioRenderObj::Start()
{
    cout << "[2]Start hdf paly thread." << endl;
    if (g_audioRender == nullptr) {
        return;
    }
    g_audioRender->control.Start((AudioHandle)g_audioRender);
    runThread_ = std::thread(&HDFAudioRenderObj::RunThread, this);
    while (!runThread_.joinable()) {
    }
    cout << "[2]Start hdf play thread success." << endl;
}

void HDFAudioRenderObj::Stop()
{
    usleep(MILLISECONDS_PER_SECOND * MICROSECONDS_PER_MILLISECOND * (info_.period + SECOND_ONE));
    if (g_audioRender != nullptr) {
        g_audioRender->control.Stop((AudioHandle)g_audioRender);
    }
    cout << "[2]Stop hdf play thread success." << endl;
}

int32_t HDFAudioRenderObj::CaptureFrame(const int32_t time)
{
    (void) time;
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS