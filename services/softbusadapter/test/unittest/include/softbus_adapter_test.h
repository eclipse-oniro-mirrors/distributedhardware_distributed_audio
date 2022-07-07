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

#ifndef OHOS_SOFTBUS_ADAPTER_TEST_H
#define OHOS_SOFTBUS_ADAPTER_TEST_H

#include <gtest/gtest.h>

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "softbus_adapter_test_utils.h"

#define private public
#include "softbus_adapter.h"
#undef private

namespace OHOS {
namespace DistributedHardware {
class SoftbusAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    SoftbusAdapter softbusAdapter;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_SOFTBUS_ADAPTER_TEST_H
