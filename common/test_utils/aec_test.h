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

#ifndef OHOS_AEC_TEST_H
#define OHOS_AEC_TEST_H

#include "local_audio.h"

namespace OHOS {
namespace DistributedHardware {
class AecTest {
public:
    AecTest() = default;
    ~AecTest() = default;

    int32_t Init();
    int32_t Process();
    void Release();
private:
    AudioRenderObj render_;
    AudioCaptureObj capture_;
    AudioBufferInfo info_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AEC_TEST_H