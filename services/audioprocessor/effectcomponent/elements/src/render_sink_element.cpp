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

#include "render_sink_element.h"

#include "daudio_errorcode.h"
#include "daudio_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "RenderSinkElement"

using namespace std;
namespace OHOS {
namespace DistributedHardware {
int32_t RenderSinkElement::Init(const std::string config)
{
    DHLOGI("[init procedure] : %s", GetType().c_str());
    for (auto &&obj : GetDownStreams()) {
        if (obj == nullptr) {
            DHLOGE("Next node is null, type: %d.", GetType().c_str());
            return ERR_DH_AUDIO_NULLPTR;
        }
        int32_t ret = obj->Init(config);
        if (ret != DH_SUCCESS) {
            DHLOGE("Current node init failed.");
            return ERR_DH_AUDIO_FAILED;
        }
    }
    return DH_SUCCESS;
}

int32_t RenderSinkElement::StartUp()
{
    DHLOGI("[startup procedure] : %s", GetType().c_str());
    for (auto &&obj : GetDownStreams()) {
        if (obj == nullptr) {
            DHLOGE("Next node is null, type: %d.", GetType().c_str());
            return ERR_DH_AUDIO_NULLPTR;
        }
        int32_t ret = obj->StartUp();
        if (ret != DH_SUCCESS) {
            DHLOGE("Current node start up failed.");
            return ERR_DH_AUDIO_FAILED;
        }
    }
    return DH_SUCCESS;
}

int32_t RenderSinkElement::ShutDown()
{
    DHLOGI("[shutdown procedure] : %s", GetType().c_str());
    for (auto &&obj : GetDownStreams()) {
        if (obj == nullptr) {
            DHLOGE("Next node is null, type: %d.", GetType().c_str());
            return ERR_DH_AUDIO_NULLPTR;
        }
        int32_t ret = obj->ShutDown();
        if (ret != DH_SUCCESS) {
            DHLOGE("Current node shut down failed.");
            return ERR_DH_AUDIO_FAILED;
        }
    }
    return DH_SUCCESS;
}

int32_t RenderSinkElement::Release()
{
    DHLOGI("[release procedure] : %s", GetType().c_str());
    for (auto &&obj : GetDownStreams()) {
        if (obj == nullptr) {
            DHLOGE("Next node is null, type: %d.", GetType().c_str());
            return ERR_DH_AUDIO_NULLPTR;
        }
        int32_t ret = obj->Release();
        if (ret != DH_SUCCESS) {
            DHLOGE("Current node release failed.");
            return ERR_DH_AUDIO_FAILED;
        }
    }
    return DH_SUCCESS;
}

std::string RenderSinkElement::GetType()
{
    return elemType_;
}

int32_t RenderSinkElement::PushData(std::shared_ptr<AudioData> &inData, std::shared_ptr<AudioData> &outData)
{
    std::shared_ptr<AudioData> tmpData;
    int32_t ret = DoProcessData(inData, tmpData);
    if (ret != DH_SUCCESS) {
        DHLOGI("Current element process data failed.");
        return ERR_DH_AUDIO_FAILED;
    }
    for (auto &&obj : GetDownStreams()) {
        if (obj == nullptr) {
            DHLOGE("Next node is null, type: %d.", GetType().c_str());
            return ERR_DH_AUDIO_NULLPTR;
        }
        int32_t ret = obj->PushData(tmpData, outData);
        if (ret != DH_SUCCESS) {
            DHLOGE("Current node release failed.");
            return ERR_DH_AUDIO_FAILED;
        }
    }
    return DH_SUCCESS;
}

int32_t RenderSinkElement::DoProcessData(std::shared_ptr<AudioData> &inData, std::shared_ptr<AudioData> &outData)
{
    DHLOGI("Element of type: %s is processing data.", GetType().c_str());
    (void)inData;
    (void)outData;
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS