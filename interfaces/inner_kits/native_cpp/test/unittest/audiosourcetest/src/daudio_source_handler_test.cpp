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

#define private public
#include "daudio_source_handler_test.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DAudioSourceHandlerTest::SetUpTestCase(void)
{
    DAudioSourceHandler::GetInstance().InitSource("DAudioSourceHandlerTest");
}

void DAudioSourceHandlerTest::TearDownTestCase(void)
{
    DAudioSourceHandler::GetInstance().ReleaseSource();
}

void DAudioSourceHandlerTest::SetUp(void) {}

void DAudioSourceHandlerTest::TearDown(void) {}

/**
 * @tc.name: RegisterDistributedHardware_001
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceHandlerTest, RegisterDistributedHardware_001, TestSize.Level1)
{
    const std::string devId = "devId";
    const std::string dhId = "dhId";
    EnableParam param;
    param.version = "1";
    param.attrs = "attrs";
    std::shared_ptr<RegisterCallback> callback = std::make_shared<RegisterCallbackTest>();
    int32_t ret = DAudioSourceHandler::GetInstance().RegisterDistributedHardware(devId, dhId, param, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: RegisterDistributedHardware_002
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceHandlerTest, RegisterDistributedHardware_002, TestSize.Level1)
{
    const std::string devId = "devId";
    const std::string dhId = "dhId";
    EnableParam param;
    param.version = "1";
    param.attrs = "attrs";
    std::shared_ptr<RegisterCallback> callback = std::make_shared<RegisterCallbackTest>();
    DAudioSourceHandler::GetInstance().dAudioIpcCallback_ = nullptr;
    int32_t ret = DAudioSourceHandler::GetInstance().RegisterDistributedHardware(devId, dhId, param, callback);
    EXPECT_EQ(ERR_DH_AUDIO_SA_IPCCALLBACK_NOT_INIT, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
