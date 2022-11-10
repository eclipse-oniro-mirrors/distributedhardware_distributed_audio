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

#ifndef OHOS_TASK_IMPL_H
#define OHOS_TASK_IMPL_H

#include <functional>

#include "task_impl_interface.h"

namespace OHOS {
namespace DistributedHardware {
class TaskImpl : public TaskImplInterface {
public:
    TaskImpl(std::function<int32_t()> taskFunc, std::function<void(int32_t)> taskCallback);
    ~TaskImpl() override = default;
    void Run() override;

private:
    std::function<int32_t()> taskFunc_ = nullptr;
    std::function<void(int32_t)> taskCallback_ = nullptr;
};

template <class T> using TaskFunc = int32_t (T::*)(const std::string &);
template <class T> using TaskCallback = void (T::*)(int32_t, const std::string &, const std::string &);

template <typename T>
std::shared_ptr<TaskImplInterface> GenerateTask(
    T *object, TaskFunc<T> taskFunc, const std::string &args, const std::string &funcName,
    TaskCallback<T> taskCallback = [](int32_t ret, const std::string &result, const std::string &funcName) {
        (void)ret;
        (void)result;
        (void)funcName;
    })
{
    auto task = std::make_shared<TaskImpl>(std::bind(taskFunc, object, args),
        std::bind(taskCallback, object, std::placeholders::_1, args, funcName));
    return task;
}
} // DistributedHardware
} // OHOS
#endif // OHOS_TASK_IMPL_H
