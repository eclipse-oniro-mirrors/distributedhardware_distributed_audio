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

#include "element_factory.h"

#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "render_sink_element.h"
#include "source_element.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "ElementFactory"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<DownStreamElement> ElementFactory::CreateElement(ElementType elementType)
{
    switch (elementType) {
        case ElementType::SOURCE:
            DHLOGI("Factory: creating source.");
            return std::make_shared<SourceElement>();
        case ElementType::REF_SINK:
            DHLOGI("Factory: creating sink.");
            return std::make_shared<RenderSinkElement>();
        case ElementType::MIC_SINK:
        case ElementType::BUFFER_QUEUE:
        case ElementType::RESAMPLER:
        case ElementType::ECHO_CANSELLER:
        default:
            DHLOGE("This element type is not exist.");
            break;
    }
    return nullptr;
}
} // DistributedHardware
} // OHOS