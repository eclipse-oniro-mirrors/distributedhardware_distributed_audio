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

#include "daudio_source_dev_ctrl_manager_test.h"

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DAudioSourceDevCtrlMgrTest::SetUpTestCase(void) {}

void DAudioSourceDevCtrlMgrTest::TearDownTestCase(void) {}

void DAudioSourceDevCtrlMgrTest::SetUp(void)
{
    uint64_t tokenId;
    const char* perms[2] = {"ohos.permisson.DISTRIBUTED_DATASYNC", "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER"};
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[1] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "daudio_source_dev_ctrl_manager_test",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);

    std::string networkId = "devId";
    std::shared_ptr<IAudioEventCallback> audioEventCallback = nullptr;
    sourceDevCtrl_ = std::make_shared<DAudioSourceDevCtrlMgr>(networkId, audioEventCallback);
}

void DAudioSourceDevCtrlMgrTest::TearDown(void)
{
    sourceDevCtrl_ = nullptr;
}

/**
 * @tc.name: SetUp_001
 * @tc.desc: Verify the SetUp function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, SetUp_001, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->SetUp());
}

/**
 * @tc.name: Start_001
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, Start_001, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->Start());
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Verify the Stop function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, Stop_001, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->Stop());
}

/**
 * @tc.name: Release_001
 * @tc.desc: Verify the Release function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, Release_001, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->Release());
}

/**
 * @tc.name: IsOpened_001
 * @tc.desc: Verify the IsOpened function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, IsOpened_001, TestSize.Level1)
{
    EXPECT_EQ(true, sourceDevCtrl_->IsOpened());
}

/**
 * @tc.name: SendAudioEvent_001
 * @tc.desc: Verify the SendAudioEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, SendAudioEvent_001, TestSize.Level1)
{
    AudioEvent event;
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->SendAudioEvent(event));
}
} // namespace DistributedHardware
} // namespace OHOS
