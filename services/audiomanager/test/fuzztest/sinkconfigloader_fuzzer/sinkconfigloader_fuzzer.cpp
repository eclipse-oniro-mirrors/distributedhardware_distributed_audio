/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "sinkconfigloader_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "daudio_sink_config_loader.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr size_t MIN_FUZZ_DATA_LEN = sizeof(uint8_t);
constexpr int32_t FUZZER_EXIT_CODE = 0;
} // namespace

void SinkConfigLoaderFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < MIN_FUZZ_DATA_LEN) {
        return;
    }
    auto &loader = DAudioSinkConfigLoader::GetInstance();
    /* First call loads and parses the config file, later calls hit the initialized branch. */
    (void)loader.Init();
    (void)loader.Init();
    (void)loader.IsMicSinkEnabled();
    (void)loader.IsSpeakerSinkEnabled();
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SinkConfigLoaderFuzzTest(data, size);
    return OHOS::DistributedHardware::FUZZER_EXIT_CODE;
}
