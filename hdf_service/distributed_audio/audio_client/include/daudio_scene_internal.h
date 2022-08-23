/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DAUDIO_SCRENE_INTERNAL_H
#define DAUDIO_SCRENE_INTERNAL_H

#include "audio_types.h"
#include "daudio_errcode.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;
template<typename T>
class AudioSceneInternal final {
public:
    static int32_t CheckSceneCapability(AudioHandle handle, const struct AudioSceneDescriptor *scene, bool *supported);
    static int32_t SelectScene(AudioHandle handle, const struct AudioSceneDescriptor *scene);

public:
    static const char *AUDIO_LOG;
};
template<typename T>
const char *AudioSceneInternal<T>::AUDIO_LOG = "AudioSceneInternal";

template<typename T>
int32_t AudioSceneInternal<T>::SelectScene(AudioHandle handle, const struct AudioSceneDescriptor *scene)
{
    if (handle == nullptr || scene == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    AudioSceneDescriptorHAL sceneHAL = {
        .id = scene->scene.id,
        .desc.portId = scene->desc.portId,
        .desc.pins = scene->desc.pins,
    };
    if (scene->desc.desc == nullptr) {
        sceneHAL.desc.desc = "";
    } else {
        sceneHAL.desc.desc = scene->desc.desc;
    }
    return (context == nullptr || context->proxy_ == nullptr) ?
        ERR_DH_AUDIO_HDF_INVALID_PARAM : context->proxy_->SelectScene(sceneHAL);
}

template<typename T>
int32_t AudioSceneInternal<T>::CheckSceneCapability(AudioHandle handle, const struct AudioSceneDescriptor *scene,
    bool *supported)
{
    if (handle == nullptr || scene == nullptr || supported == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    AudioSceneDescriptorHAL sceneHAL = {
        .id = scene->scene.id,
        .desc.portId = scene->desc.portId,
        .desc.pins = scene->desc.pins,
    };

    if (scene->desc.desc == nullptr) {
        sceneHAL.desc.desc = "";
    } else {
        sceneHAL.desc.desc = scene->desc.desc;
    }

    return (context == nullptr || context->proxy_ == nullptr) ?
        ERR_DH_AUDIO_HDF_INVALID_PARAM : context->proxy_->CheckSceneCapability(sceneHAL, *supported);
}
} // namespace DistributedHardware
} // namespace OHOS
#endif // DAUDIO_SCRENE_INTERNAL_H
