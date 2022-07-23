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

#include "task_queue.h"

#include "daudio_errorcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
void TaskQueue::Start()
{
    taskQueueReady_ = true;
    isQuitTaskQueue_ = false;
    mainThreadLoop_ = std::thread(&TaskQueue::Run, this);
    while (!mainThreadLoop_.joinable()) {
    }
}

void TaskQueue::Stop()
{
    DHLOGI("%s: Stop task queue.", LOG_TAG);
    isQuitTaskQueue_ = true;
    if (mainThreadLoop_.joinable()) {
        mainThreadLoop_.join();
    }
    DHLOGI("%s: Stop task queue success.", LOG_TAG);
}

void TaskQueue::Run()
{
    DHLOGI("%s: Task queue running.", LOG_TAG);
    while (taskQueueReady_) {
        if (isQuitTaskQueue_ && taskQueue_.empty()) {
            DHLOGI("%s: Task queue quit.", LOG_TAG);
            break;
        }
        std::shared_ptr<TaskImplInterface> task = nullptr;
        {
            std::unique_lock<std::mutex> lck(taskQueueMutex_);
            taskQueueCond_.wait_for(lck, std::chrono::seconds(TASK_WAIT_SECONDS),
                [this]() { return !taskQueue_.empty(); });
            if (taskQueue_.empty()) {
                continue;
            }
            Consume(task);
        }
        if (task == nullptr) {
            continue;
        }
        task->Run();
    }
}

void TaskQueue::Consume(std::shared_ptr<TaskImplInterface> &task)
{
    task = taskQueue_.front();
    taskQueue_.pop();
}

int32_t TaskQueue::Produce(std::shared_ptr<TaskImplInterface> &task)
{
    std::lock_guard<std::mutex> lck(taskQueueMutex_);
    if (taskQueue_.size() >= maxSize_) {
        DHLOGI("%s: task queue is full, size: %zu", LOG_TAG, taskQueue_.size());
        return ERR_DH_AUDIO_SA_TASKQUEUE_FULL;
    }
    taskQueue_.push(task);
    taskQueueCond_.notify_one();
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS
