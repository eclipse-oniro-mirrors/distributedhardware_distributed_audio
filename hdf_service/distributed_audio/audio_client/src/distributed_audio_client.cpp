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

#include "distributed_audio_client.h"

#include <securec.h>
#include <v1_0/types.h>

#include "audio_types.h"
#include "daudio_errcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioAdapter;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioAdapterDescriptorHAL;

static const char * const AUDIO_LOG = "DAudioAudioClient";

static int32_t InitAudioAdapterDescriptor(AudioManagerContext *context,
    std::vector<AudioAdapterDescriptorHAL> &descriptors)
{
    DHLOGI("%s: Init audio adapters descriptor, size is: %zd.", AUDIO_LOG, descriptors.size());
    for (auto desc : descriptors) {
        AudioPort *audioPorts = (AudioPort *)malloc(desc.ports.size() * sizeof(AudioPort));
        if (audioPorts == nullptr) {
            DHLOGE("%s: GetAllAdaptersInternal audioPorts is nullptr.", AUDIO_LOG);
            return ERR_DH_AUDIO_HDF_FAILURE;
        }
        char* adapterName = (char*)calloc(desc.adapterName.length() + 1, sizeof(char));
        if (strcpy_s(adapterName, desc.adapterName.length() + 1, desc.adapterName.c_str()) != EOK) {
            DHLOGE("%s:strcpy_s adapterName failed.", AUDIO_LOG);
            continue;
        }
        AudioAdapterDescriptor descInternal = {
            .adapterName = adapterName,
            .portNum = desc.ports.size(),
            .ports = audioPorts,
        };
        for (auto port : desc.ports) {
            char* portName = (char*)calloc(port.portName.length() + 1, sizeof(char));
            if (strcpy_s(portName, port.portName.length() + 1, port.portName.c_str()) != EOK) {
                DHLOGE("%s:strcpy_s portName failed.", AUDIO_LOG);
                continue;
            }
            audioPorts->dir = static_cast<AudioPortDirection>(port.dir);
            audioPorts->portId = port.portId;
            audioPorts->portName = portName;
            audioPorts++;
        }
        context->descriptors_.push_back(descInternal);
    }
    return DH_SUCCESS;
}

static int32_t GetAllAdaptersInternal(struct AudioManager *manager, struct AudioAdapterDescriptor **descs,
    int32_t *size)
{
    DHLOGI("%s: GetAllAdaptersInternal enter.", AUDIO_LOG);
    if (manager == nullptr || descs == nullptr || size == nullptr) {
        DHLOGE("%s: GetAllAdaptersInternal param is nullptr.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioManagerContext *context = reinterpret_cast<AudioManagerContext *>(manager);
    std::lock_guard<std::mutex> lock(context->mtx_);

    std::vector<AudioAdapterDescriptorHAL> descriptors;
    int32_t ret = context->proxy_->GetAllAdapters(descriptors);
    if (ret != DH_SUCCESS) {
        *descs = nullptr;
        *size = 0;
        DHLOGE("%s: GetAllAdaptersInternal getAllAdapters failed.", AUDIO_LOG);
        return ret;
    }
    context->ClearDescriptors();
    ret = InitAudioAdapterDescriptor(context, descriptors);
    if (ret != DH_SUCCESS) {
        return ret;
    }
    *descs = context->descriptors_.data();
    *size = context->descriptors_.size();
    return DH_SUCCESS;
}

static int32_t LoadAdapterInternal(struct AudioManager *manager, const struct AudioAdapterDescriptor *desc,
    struct AudioAdapter **adapter)
{
    DHLOGI("%s: LoadAdapterInternal enter.", AUDIO_LOG);
    if (manager == nullptr || desc == nullptr || desc->adapterName == nullptr || adapter == nullptr) {
        DHLOGE("%s: LoadAdapterInternal param is nullptr.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioManagerContext *context = reinterpret_cast<AudioManagerContext *>(manager);
    AudioAdapterDescriptorHAL descriptor = {
        .adapterName = desc->adapterName,
    };
    sptr<IAudioAdapter> adapterProxy = nullptr;
    int32_t ret = context->proxy_->LoadAdapter(descriptor, adapterProxy);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: LoadAdapterInternal loadAdapter failed.", AUDIO_LOG);
        *adapter = nullptr;
        return ret;
    }

    std::unique_ptr<AudioAdapterContext> adapterContext  = std::make_unique<AudioAdapterContext>();
    adapterContext->proxy_ = adapterProxy;
    *adapter = &adapterContext->instance_;
    adapterContext->adapterName_ = descriptor.adapterName;
    {
        std::lock_guard<std::mutex> lock(context->mtx_);
        context->adapters_.push_back(std::move(adapterContext));
    }
    return DH_SUCCESS;
}

static void UnloadAdapterInternal(struct AudioManager *manager, struct AudioAdapter *adapter)
{
    DHLOGI("%s: UnloadAdapterInternal enter.", AUDIO_LOG);
    if (manager == nullptr || adapter == nullptr) {
        DHLOGE("%s: UnloadAdapterInternal param is nullptr.", AUDIO_LOG);
        return;
    }

    AudioManagerContext *context = reinterpret_cast<AudioManagerContext *>(manager);
    AudioAdapterContext *adapterContext = reinterpret_cast<AudioAdapterContext *>(adapter);

    std::lock_guard<std::mutex> lock(context->mtx_);
    for (auto it = context->adapters_.begin(); it != context->adapters_.end(); ++it) {
        if ((*it).get() == adapterContext) {
            int32_t ret = context->proxy_->UnloadAdapter(adapterContext->adapterName_);
            if (ret != DH_SUCCESS) {
                DHLOGE("%s: UnloadAdapterInternal unloadAdapter failed.", AUDIO_LOG);
                return;
            }
            context->adapters_.erase(it);
            break;
        }
    }
}

void AudioManagerContext::ClearDescriptors()
{
    for (auto &desc : descriptors_) {
        if (desc.adapterName != nullptr) {
            free(const_cast<char *>(desc.adapterName));
        }
        for (uint32_t i = 0; i < desc.portNum; i++) {
            if (desc.ports[i].portName != nullptr) {
                free(const_cast<char *>(desc.ports[i].portName));
            }
        }
        free(desc.ports);
    }
    descriptors_.clear();
}

AudioManagerContext::AudioManagerContext()
{
    instance_.GetAllAdapters = GetAllAdaptersInternal;
    instance_.LoadAdapter = LoadAdapterInternal;
    instance_.UnloadAdapter = UnloadAdapterInternal;
}

AudioManagerContext::~AudioManagerContext()
{
    adapters_.clear();
    ClearDescriptors();
}

AudioManagerContext g_AudioManagerContext;

static bool AudioManagerInit()
{
    std::lock_guard<std::mutex> lock(g_AudioManagerContext.mtx_);

    if (g_AudioManagerContext.initFlag_ == true) {
        return true;
    }

    sptr<IAudioManager> audioMgr = IAudioManager::Get("daudio_primary_service", false);
    if (audioMgr == nullptr) {
        return false;
    }
    g_AudioManagerContext.proxy_ = audioMgr;
    g_AudioManagerContext.initFlag_ = true;
    return true;
}
} // DistributedHardware
} // OHOS

#ifdef __cplusplus
extern "C" {
#endif

struct AudioManager *GetAudioManagerFuncs(void)
{
    if (OHOS::DistributedHardware::AudioManagerInit() == true) {
        return &OHOS::DistributedHardware::g_AudioManagerContext.instance_;
    } else {
        return nullptr;
    }
}

#ifdef __cplusplus
}
#endif
