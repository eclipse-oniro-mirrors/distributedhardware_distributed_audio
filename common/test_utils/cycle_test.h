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

#ifndef OHOS_CYCLE_TEST_H
#define OHOS_CYCLE_TEST_H

#include <memory>
#include "audio_base.h"
#include "audio_buffer.h"

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    MODE_INVALID = 0,
    MODE_LR_LC = 1,
    MODE_HR_HC = 2,
} CYCLE_MODE;
class CycleTest {
public:
    CycleTest() = default;
    ~CycleTest() = default;

    int32_t Init();
    int32_t Process();
    void Release();
private:
    void GetMode();

private:
    CYCLE_MODE mode_ = MODE_INVALID;
    std::string spkFile_;
    std::string micFile_;
    std::unique_ptr<AudioObject> render_;
    std::unique_ptr<AudioObject> capture_;
    AudioBufferInfo info_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_CYCLE_TEST_H