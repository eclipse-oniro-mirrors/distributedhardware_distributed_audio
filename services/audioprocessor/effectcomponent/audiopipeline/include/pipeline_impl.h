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

#ifndef PIPELINE_IMPL_H
#define PIPELINE_IMPL_H

#include <string>
#include <list>

#include "audio_data.h"
#include "element_factory.h"
#include "ielement.h"
#include "ipipeline.h"

namespace OHOS {
namespace DistributedHardware {
class PipelineImpl : public IPipeline {
public:
    PipelineImpl(PipeType pipeType);
    ~PipelineImpl() = default;
    virtual int32_t Init(const std::string config) override;
    virtual int32_t StartUp() override;
    virtual int32_t ShutDown() override;
    virtual int32_t Release() override;
    virtual int32_t FeedData(std::shared_ptr<AudioData> &inData, std::shared_ptr<AudioData> &outData) override;
    int32_t BuildPipeStream(const std::list<ElementType> &config);

private:
    PipeType pipeType_;
    std::shared_ptr<IElement> sourceNode_ = nullptr;
};
} // DistributedHardware
} // OHOS
#endif // PIPELINE_IMPL_H