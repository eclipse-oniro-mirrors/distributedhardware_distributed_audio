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

#include "setaudioparameters_fuzzer.h"

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
void SetAudioParametersFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < (sizeof(int32_t)))) {
        return;
    }

    AudioAdapterDescriptorHAL desc;
    auto audioAdapter = std::make_shared<AudioAdapterInterfaceImpl>(desc);
    AudioExtParamKeyHAL key = *(reinterpret_cast<const AudioExtParamKeyHAL*>(data));
    std::string condition(reinterpret_cast<const char*>(data), size);
    std::string value(reinterpret_cast<const char*>(data), size);

    audioAdapter->SetAudioParameters(key, condition, value);
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
    OHOS::HDI::DistributedAudio::Audio::V1_0::SetAudioParametersFuzzTest(data, size);
    return 0;
}

