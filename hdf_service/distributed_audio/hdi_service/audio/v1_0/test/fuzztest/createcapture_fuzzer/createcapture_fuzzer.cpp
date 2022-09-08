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

#include "createcapture_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "audio_adapter_interface_impl.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
void CreateCaptureFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < (sizeof(int32_t)))) {
        return;
    }

    AudioAdapterDescriptorHAL desc;
    auto audioAdapter = std::make_shared<AudioAdapterInterfaceImpl>(desc);

    uint32_t portId = *(reinterpret_cast<const uint32_t*>(data));
    uint32_t pins = *(reinterpret_cast<const uint32_t*>(data));
    std::string tdesc(reinterpret_cast<const char*>(data), size);
    AudioDeviceDescriptorHAL deviceDes;
    deviceDes.portId = portId;
    deviceDes.pins = pins;
    deviceDes.desc = tdesc;

    AudioSampleAttributesHAL sampleAttr;
    sampleAttr.type = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.interleaved = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.format = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.sampleRate = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.channelCount = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.period = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.frameSize = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.isBigEndian = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.isSignedData = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.startThreshold = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.stopThreshold = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.silenceThreshold = *(reinterpret_cast<const uint32_t*>(data));
    sampleAttr.streamId = *(reinterpret_cast<const uint32_t*>(data));

    sptr<IAudioCapture> capture;
    audioAdapter->CreateCapture(deviceDes, sampleAttr, capture);
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::HDI::DistributedAudio::Audio::V1_0::CreateCaptureFuzzTest(data, size);
    return 0;
}

