/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_TASK_QUEUE_H
#define OHOS_TASK_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "task_impl_interface.h"

namespace OHOS {
namespace DistributedHardware {
class TaskQueue {
public:
    explicit TaskQueue(uint32_t maxSize) : maxSize_(maxSize) {};
    ~TaskQueue() = default;
    int32_t Produce(std::shared_ptr<TaskImplInterface> &task);
    void Start();
    void Stop();

private:
    void Consume(std::shared_ptr<TaskImplInterface> &task);
    void Run();

private:
    static constexpr uint8_t TASK_WAIT_TIME = 20;
    static constexpr const char* MAIN_THREAD_LOOP = "mainThreadLoop";

    std::mutex taskQueueMutex_;
    std::condition_variable taskQueueCond_;
    std::queue<std::shared_ptr<TaskImplInterface>> taskQueue_;
    bool taskQueueReady_ = false;
    bool isQuitTaskQueue_ = false;
    std::thread mainThreadLoop_;
    uint32_t maxSize_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_TASK_QUEUE_H
