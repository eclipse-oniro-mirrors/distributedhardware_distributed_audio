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
#include "audiocontrol_test_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DAudioSourceDevCtrlMgrTest::SetUpTestCase(void) {}

void DAudioSourceDevCtrlMgrTest::TearDownTestCase(void) {}

void DAudioSourceDevCtrlMgrTest::SetUp(void)
{
    std::string networkId = "networkId";
    std::shared_ptr<IAudioEventCallback> audioEventCallback = nullptr;
    sourceDevCtrl_ = std::make_shared<DAudioSourceDevCtrlMgr>(networkId, audioEventCallback);
}

void DAudioSourceDevCtrlMgrTest::TearDown(void)
{
    sourceDevCtrl_ = nullptr;
}

/**
 * @tc.name: Init_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, Init_001, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->Init());
}

/**
 * @tc.name: UnInit_001
 * @tc.desc: Verify the UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, UnInit_001, TestSize.Level1)
{
    sourceDevCtrl_->audioCtrlTrans_ = nullptr;
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->UnInit());
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
    sourceDevCtrl_->audioCtrlTrans_ = nullptr;
    EXPECT_EQ(ERR_DH_AUDIO_SA_CTRL_TRANS_NULL, sourceDevCtrl_->Start());
}

/**
 * @tc.name: Start_002
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, Start_002, TestSize.Level1)
{
    std::string devId = "devId";
    sourceDevCtrl_->isOpened_ = true;
    sourceDevCtrl_->audioCtrlTrans_ = std::make_shared<MockIAudioCtrlTransport>(devId);
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
    sourceDevCtrl_->audioCtrlTrans_ = nullptr;
    EXPECT_EQ(ERR_DH_AUDIO_SA_CTRL_TRANS_NULL, sourceDevCtrl_->Stop());
}

/**
 * @tc.name: Stop_002
 * @tc.desc: Verify the Stop function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, Stop_002, TestSize.Level1)
{
    std::string devId = "devId";
    sourceDevCtrl_->audioCtrlTrans_ = std::make_shared<MockIAudioCtrlTransport>(devId);
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
    sourceDevCtrl_->audioCtrlTrans_ = nullptr;
    EXPECT_EQ(ERR_DH_AUDIO_SA_CTRL_TRANS_NULL, sourceDevCtrl_->Release());
}

/**
 * @tc.name: Release_002
 * @tc.desc: Verify the Release function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, Release_002, TestSize.Level1)
{
    std::string devId = "devId";
    sourceDevCtrl_->audioCtrlTrans_ = std::make_shared<MockIAudioCtrlTransport>(devId);
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
    sourceDevCtrl_->isOpened_ = true;
    bool actual = sourceDevCtrl_->IsOpened();
    EXPECT_EQ(true, actual);
}

/**
 * @tc.name: SendAudioEvent_001
 * @tc.desc: Verify the SendAudioEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, SendAudioEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioEvent> event = nullptr;
    sourceDevCtrl_->audioCtrlTrans_ = nullptr;
    EXPECT_EQ(ERR_DH_AUDIO_SA_CTRL_TRANS_NULL, sourceDevCtrl_->SendAudioEvent(event));
}

/**
 * @tc.name: SendAudioEvent_002
 * @tc.desc: Verify the SendAudioEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, SendAudioEvent_002, TestSize.Level1)
{
    std::string devId = "devId";
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    sourceDevCtrl_->audioCtrlTrans_ = std::make_shared<MockIAudioCtrlTransport>(devId);
    EXPECT_EQ(DH_SUCCESS, sourceDevCtrl_->SendAudioEvent(event));
}

/**
 * @tc.name: OnStateChange_001
 * @tc.desc: Verify the OnStateChange function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, OnStateChange_001, TestSize.Level1)
{
    int32_t type = static_cast<int32_t>(AudioEventType::CTRL_OPENED);
    sourceDevCtrl_->OnStateChange(type);
}

/**
 * @tc.name: OnEventReceived_001
 * @tc.desc: Verify the OnEventReceived function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevCtrlMgrTest, OnEventReceived_001, TestSize.Level1)
{
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    sourceDevCtrl_->audioEventCallback_ = std::make_shared<MockIAudioEventCallback>();
    sourceDevCtrl_->OnEventReceived(event);
}
} // namespace DistributedHardware
} // namespace OHOS