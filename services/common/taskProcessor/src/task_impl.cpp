/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "task_impl.h"

#include "daudio_errorcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
TaskImpl::TaskImpl(std::function<int32_t()> taskFunc, std::function<void(int32_t)> taskCallback)
{
    taskFunc_ = taskFunc;
    taskCallback_ = taskCallback;
}

void TaskImpl::Run()
{
    DHLOGI("%s: task run.", LOG_TAG);
    if (taskFunc_ == nullptr) {
        DHLOGE("%s: task function is null", LOG_TAG);
        return;
    }
    int32_t ret = taskFunc_();
    if (taskCallback_ != nullptr) {
        taskCallback_(ret);
    }
}
} // DistributedHardware
} // OHOS
