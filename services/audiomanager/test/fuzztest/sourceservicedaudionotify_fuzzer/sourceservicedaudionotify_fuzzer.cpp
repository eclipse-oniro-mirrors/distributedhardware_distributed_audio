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

#include "sourceservicedaudionotify_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "daudio_source_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
void SourceServiceDAudioNotifyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < (sizeof(int32_t)))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    
    std::string dhId = fdp.ConsumeRandomLengthString();
    int32_t saId = fdp.ConsumeIntegral<int32_t>();
    bool runOnCreate = fdp.ConsumeBool();
    int32_t eventType = fdp.ConsumeIntegral<int32_t>();
    std::string eventContent = fdp.ConsumeRandomLengthString();
    std::string devId = fdp.ConsumeRandomLengthString();

    auto dAudioSourceService = std::make_shared<DAudioSourceService>(saId, runOnCreate);

    dAudioSourceService->DAudioNotify(devId, dhId, eventType, eventContent);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SourceServiceDAudioNotifyFuzzTest(data, size);
    return 0;
}

