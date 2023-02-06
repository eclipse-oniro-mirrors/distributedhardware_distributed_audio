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

#include "pipeline_impl.h"

#include "daudio_errorcode.h"
#include "daudio_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "PipelineImpl"

namespace OHOS {
namespace DistributedHardware {
PipelineImpl::PipelineImpl(PipeType pipeType, std::shared_ptr<IElement> sourceNode)
    : pipeType_(pipeType), sourceNode_(sourceNode)
{
    DHLOGI("Pipeline is construct, type: %d.", pipeType_);
}

int32_t PipelineImpl::Init(const std::string config)
{
    DHLOGI("Init pipeline.");
    if (sourceNode_ == nullptr) {
        DHLOGE("Source node is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    sourceNode_->Init(config);
    return DH_SUCCESS;
}

int32_t PipelineImpl::StartUp()
{
    if (sourceNode_ == nullptr) {
        DHLOGE("Source node is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    sourceNode_->StartUp();
    return DH_SUCCESS;
}

int32_t PipelineImpl::ShutDown()
{
    if (sourceNode_ == nullptr) {
        DHLOGE("Source node is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    sourceNode_->ShutDown();
    return DH_SUCCESS;
}

int32_t PipelineImpl::Release()
{
    if (sourceNode_ == nullptr) {
        DHLOGE("Source node is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    sourceNode_->Release();

    return DH_SUCCESS;
}

int32_t PipelineImpl::FeedData(std::shared_ptr<AudioData> &inData, std::shared_ptr<AudioData> &outData)
{
    if (sourceNode_ == nullptr) {
        DHLOGE("Source node is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    return sourceNode_->PushData(inData, outData);
}
} // DistributedHardware
} // OHOS