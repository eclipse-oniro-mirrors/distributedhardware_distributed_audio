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

#ifndef DAUDIO_EFFECT_COMPONENT_H
#define DAUDIO_EFFECT_COMPONENT_H

#include <map>
#include <memory>
#include <list>

#include "element_factory.h"
#include "ipipeline.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t ERR_PIPE_ID = -1;
constexpr int32_t INITIAL_PIPE_ID = -1;
constexpr int32_t MAX_PIPELINE_SIZE = 1000000;

class PipelineManager {
    DECLARE_SINGLE_INSTANCE_BASE(PipelineManager);
public:
    int32_t CreatePipeline(const PipeType &pipeType, const std::list<ElementType> &config,
        std::shared_ptr<IPipeline> &pipe);
    int32_t ClearPipeline(int32_t pipeId);

private:
    PipelineManager() = default;
    ~PipelineManager() = default;
    int32_t GeneratePipeId();
    std::shared_ptr<DownStreamElement> BuildPipeStream(const std::list<ElementType> &config);

private:
    std::map<int32_t, std::shared_ptr<IPipeline>> mapPipelines_;
};
} // DistributedHardware
} // OHOS
#endif // DAUDIO_EFFECT_COMPONENT_H