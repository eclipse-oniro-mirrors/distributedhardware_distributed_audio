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

#include "task_queue_test.h"

#include "audio_event.h"
#include "daudio_errorcode.h"
#include "mock_task_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void TaskQueueTest::SetUpTestCase(void) {}

void TaskQueueTest::TearDownTestCase(void) {}

void TaskQueueTest::SetUp() {}

void TaskQueueTest::TearDown() {}

/**
 * @tc.name: Produce_001
 * @tc.desc: Verify the Produce function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(TaskQueueTest, Produce_001, TestSize.Level1)
{
    uint32_t maxSize = 2;
    TaskQueue taskQueueClass_(maxSize);
    std::shared_ptr<TaskImplInterface> task = nullptr;

    EXPECT_EQ(ERR_DH_AUDIO_NULLPTR, taskQueueClass_.Produce(task));
    taskQueueClass_.maxSize_ = 0;
    EXPECT_EQ(ERR_DH_AUDIO_NULLPTR, taskQueueClass_.Produce(task));
}

/**
 * @tc.name: Produce_002
 * @tc.desc: Verify the Produce function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(TaskQueueTest, Produce_002, TestSize.Level1)
{
    uint32_t maxSize = 2;
    TaskQueue taskQueueClass_(maxSize);
    std::shared_ptr<TaskImplInterface> task = std::make_shared<MockTaskImplInterface>();
    taskQueueClass_.Start();
    EXPECT_EQ(DH_SUCCESS, taskQueueClass_.Produce(task));
    taskQueueClass_.Stop();
    std::shared_ptr<TaskImplInterface> task2 = std::make_shared<MockTaskImplInterface>();
    EXPECT_EQ(DH_SUCCESS, taskQueueClass_.Produce(task));
}
} // DistributedHardware
} // OHOS
