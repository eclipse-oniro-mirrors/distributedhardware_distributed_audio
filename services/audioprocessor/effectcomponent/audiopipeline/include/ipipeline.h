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

#ifndef IPIPELINE_H
#define IPIPELINE_H

#include <memory>
#include <string>

#include "audio_data.h"
#include "life_cycle.h"

namespace OHOS {
namespace DistributedHardware {
enum class PipeType {
    RENDER_PIPE,
    CAPTURE_PIPE,
    UNKNOWN,
};

class IPipeline : public ILifeCycle {
public:
    virtual ~IPipeline() = default;
    virtual int32_t FeedData(std::shared_ptr<AudioData> &inData, std::shared_ptr<AudioData> &outData) = 0;
};
} // DistributedHardware
} // OHOS
#endif // IPIPELINE_H