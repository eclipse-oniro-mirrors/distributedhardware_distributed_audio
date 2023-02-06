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

#ifndef ELEMENT_FACTORY_H
#define ELEMENT_FACTORY_H

#include <memory>

#include "down_stream_element.h"

namespace OHOS {
namespace DistributedHardware {
enum class ElementType {
    UNKNOWN,
    SOURCE,
    REF_SINK,
    MIC_SINK,
    BUFFER_QUEUE,
    RESAMPLER,
    ECHO_CANSELLER,
};

class ElementFactory {
public:
    ElementFactory() = default;
    ~ElementFactory() = default;
    std::shared_ptr<DownStreamElement> CreateElement(enum ElementType elementType);
};
} // DistributedHardware
} // OHOS
#endif // ELEMENT_FACTORY_H