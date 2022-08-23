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

#ifndef OHOS_DMIC_CLIENT_TEST_H
#define OHOS_DMIC_CLIENT_TEST_H

#include <gtest/gtest.h>

#include "daudio_test_utils.h"
#include "iaudio_event_callback.h"
#define private public
#include "dmic_client.h"
#undef private

namespace OHOS {
namespace DistributedHardware {
class DMicClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<IAudioEventCallback> clientCallback_ = nullptr;
    std::shared_ptr<DMicClient> micClient_ = nullptr;
    AudioParam audioParam_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DMIC_CLIENT_TEST_H