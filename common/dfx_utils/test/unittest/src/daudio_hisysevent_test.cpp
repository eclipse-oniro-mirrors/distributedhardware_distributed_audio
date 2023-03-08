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

#include "daudio_hisysevent_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DAudioHisyseventTest::SetUpTestCase(void) {}

void DAudioHisyseventTest::TearDownTestCase(void) {}

void DAudioHisyseventTest::SetUp()
{
    hisysevent_ = std::make_shared<DAudioHisysevent>();
}

void DAudioHisyseventTest::TearDown()
{
    hisysevent_ = nullptr;
}

/**
 * @tc.name: OnStateChange_001
 * @tc.desc: Verify the OnStateChange function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DAudioHisyseventTest, Dump_001, TestSize.Level1)
{
    std::string str1 = "str1";
    std::string str2 = "str2";
    std::string str3 = "str3";
    std::string str4 = "str4";
    int32_t saId = 4805;
    int32_t errNo = 1;
    hisysevent_->SysEventWriteBehavior(str1, str2);
    hisysevent_->SysEventWriteBehavior(str1, saId, str2);
    hisysevent_->SysEventWriteBehavior(str1, str2, str3, str4);

    hisysevent_->SysEventWriteFault(str1, str2);
    hisysevent_->SysEventWriteFault(str1, saId, errNo, str2);
    hisysevent_->SysEventWriteFault(str1, errNo, str2);
    hisysevent_->SysEventWriteFault(str1, str2, str3, errNo, str4);
}
} // DistributedHardware
} // OHOS
