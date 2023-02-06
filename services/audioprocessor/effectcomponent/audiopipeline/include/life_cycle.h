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

#ifndef LIFE_CYCLE_H
#define LIFE_CYCLE_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
class ILifeCycle {
public:
    virtual ~ILifeCycle() = default;
    virtual int32_t Init(const std::string config) = 0;
    virtual int32_t StartUp() = 0;
    virtual int32_t ShutDown() = 0;
    virtual int32_t Release() = 0;
};
} // OHOS
} // DistributedHardware
#endif // LIFE_CYCLE_H