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

#ifndef OHOS_AUDIO_MANAGER_INTERFACE_IMPL_H
#define OHOS_AUDIO_MANAGER_INTERFACE_IMPL_H

#include <map>
#include <mutex>
#include <string>

#include "hdf_device_desc.h"

#include "audio_adapter_interface_impl.h"
#include "v1_0/iaudio_manager.h"
#include "v1_0/id_audio_manager.h"

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
using OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioEvent;
using OHOS::HDI::DistributedAudio::Audioext::V1_0::IDAudioCallback;

typedef struct {
    std::string adapterName;
    uint32_t devId;
    uint32_t eventType;
    uint32_t deviceType;
    uint32_t volGroupId;
    uint32_t iptGroupId;
} DAudioDevEvent;

class AudioManagerInterfaceImpl : public IAudioManager {
public:
    static AudioManagerInterfaceImpl *GetAudioManager()
    {
        if (mgr == nullptr) {
            std::unique_lock<std::mutex> mgr_mutex(mutex_mgr);
            if (mgr == nullptr) {
                mgr = new AudioManagerInterfaceImpl();
            }
        }
        return mgr;
    }

    virtual ~AudioManagerInterfaceImpl();
    int32_t GetAllAdapters(std::vector<AudioAdapterDescriptorHAL> &descriptors) override;
    int32_t LoadAdapter(const AudioAdapterDescriptorHAL &descriptor, sptr<IAudioAdapter> &adapter) override;
    int32_t UnloadAdapter(const std::string &adpName) override;
    int32_t AddAudioDevice(const std::string &adpName, const uint32_t devId, const std::string &caps,
        const sptr<IDAudioCallback> &callback);
    int32_t RemoveAudioDevice(const std::string &adpName, const uint32_t devId);
    int32_t Notify(const std::string &adpName, const uint32_t devId, const AudioEvent &event);
    void SetDeviceObject(struct HdfDeviceObject *deviceObject);

private:
    AudioManagerInterfaceImpl();
    AudioManagerInterfaceImpl(const AudioManagerInterfaceImpl &);
    AudioManagerInterfaceImpl &operator = (const AudioManagerInterfaceImpl &);
    int32_t NotifyFwk(const DAudioDevEvent &event);
    int32_t CreateAdapter(const std::string &adpName, const uint32_t devId, const sptr<IDAudioCallback> &callback);

private:
    class Deletor {
    public:
        ~Deletor()
        {
            if (AudioManagerInterfaceImpl::mgr != nullptr) {
                delete AudioManagerInterfaceImpl::mgr;
            }
        };
    };
    static Deletor deletor;

private:
    const char *AUDIO_LOG = "AudioManagerInterfaceImpl";

    static AudioManagerInterfaceImpl *mgr;
    static std::mutex mutex_mgr;
    struct HdfDeviceObject *deviceObject_ = nullptr;
    std::mutex adapterMapMtx_;
    std::map<std::string, sptr<AudioAdapterInterfaceImpl>> mapAudioAdapter_;
};
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
#endif // OHOS_AUDIO_MANAGER_INTERFACE_IMPL_H
