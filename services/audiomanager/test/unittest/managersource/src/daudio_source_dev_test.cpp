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

#include "daudio_source_dev_test.h"
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DAudioSourceDevTest::SetUpTestCase(void) {}

void DAudioSourceDevTest::TearDownTestCase(void) {}

void DAudioSourceDevTest::SetUp(void)
{
    std::string devId = "devId";
    auto daudioMgrCallback = std::make_shared<DAudioSourceMgrCallback>();
    sourceDev_ = std::make_shared<DAudioSourceDev>(devId, daudioMgrCallback);
}

void DAudioSourceDevTest::TearDown(void)
{
    sourceDev_ = nullptr;
}

/**
 * @tc.name: IsSpeakerEvent_001
 * @tc.desc: Verify the IsSpeakerEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, IsSpeakerEvent_001, TestSize.Level1)
{
    AudioEvent event;
    EXPECT_EQ(DH_SUCCESS, sourceDev_->IsSpeakerEvent(event));
}

/**
 * @tc.name: IsMicEvent_001
 * @tc.desc: Verify the IsMicEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, IsMicEvent_001, TestSize.Level1)
{
    AudioEvent event;
    EXPECT_EQ(DH_SUCCESS, sourceDev_->IsMicEvent(event));
}

/**
 * @tc.name: HandleDMicClosed_001
 * @tc.desc: Verify the HandleDMicClosed function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, HandleDMicClosed_001, TestSize.Level1)
{
    AudioEvent event;
    EXPECT_EQ(DH_SUCCESS, sourceDev_->HandleDMicClosed(event));
}

/**
 * @tc.name: HandleDMicClosed_002
 * @tc.desc: Verify the HandleDMicClosed function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, HandleDMicClosed_002, TestSize.Level1)
{
    AudioEvent event;
    std::string devId = "devId";
    sourceDev_->mic_ = std::make_shared<DMicDev>(devId, sourceDev_);
    EXPECT_EQ(DH_SUCCESS, sourceDev_->HandleDMicClosed(event));
}

/**
 * @tc.name: HandleNotifyRPC_001
 * @tc.desc: Verify the HandleNotifyRPC function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, HandleNotifyRPC_001, TestSize.Level1)
{
    AudioEvent event(NOTIFY_OPEN_SPEAKER_RESULT, "result");
    EXPECT_NE(DH_SUCCESS, sourceDev_->HandleNotifyRPC(event));
}

/**
 * @tc.name: WaitForRPC_001
 * @tc.desc: Verify the WaitForRPC function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, WaitForRPC_001, TestSize.Level1)
{
    EXPECT_NE(DH_SUCCESS, sourceDev_->WaitForRPC(NOTIFY_OPEN_CTRL_RESULT));
}

/**
 * @tc.name: TaskEnableDAudio_001
 * @tc.desc: Verify the TaskEnableDAudio function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskEnableDAudio_001, TestSize.Level1)
{
    std::string args;
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskEnableDAudio(args));
}

/**
 * @tc.name: TaskDisableDAudio_001
 * @tc.desc: Verify the TaskDisableDAudio function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskDisableDAudio_001, TestSize.Level1)
{
    std::string args;
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskDisableDAudio(args));
}

/**
 * @tc.name: EnableDSpeaker_001
 * @tc.desc: Verify the EnableDSpeaker function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, EnableDSpeaker_001, TestSize.Level1)
{
    int32_t dhId = 0;
    std::string attrs = "attrs";
    EXPECT_NE(DH_SUCCESS, sourceDev_->EnableDSpeaker(dhId, attrs));
}

/**
 * @tc.name: EnableDMic_001
 * @tc.desc: Verify the EnableDMic function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, EnableDMic_001, TestSize.Level1)
{
    int32_t dhId = 0;
    std::string attrs = "attrs";
    EXPECT_NE(DH_SUCCESS, sourceDev_->EnableDMic(dhId, attrs));
}

/**
 * @tc.name: DisableDSpeaker_001
 * @tc.desc: Verify the DisableDSpeaker function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, DisableDSpeaker_001, TestSize.Level1)
{
    int32_t dhId = 0;
    EXPECT_EQ(ERR_DH_AUDIO_NULLPTR, sourceDev_->DisableDSpeaker(dhId));
}

/**
 * @tc.name: DisableDSpeaker_002
 * @tc.desc: Verify the DisableDSpeaker function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, DisableDSpeaker_002, TestSize.Level1)
{
    int32_t dhId = 0;
    std::string devId = "devId";
    sourceDev_->speaker_ = std::make_shared<DSpeakerDev>(devId, sourceDev_);
    EXPECT_NE(DH_SUCCESS, sourceDev_->DisableDSpeaker(dhId));
}

/**
 * @tc.name: DisableDMic_001
 * @tc.desc: Verify the DisableDMic function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, DisableDMic_001, TestSize.Level1)
{
    int32_t dhId = 0;
    EXPECT_EQ(ERR_DH_AUDIO_NULLPTR, sourceDev_->DisableDMic(dhId));
}

/**
 * @tc.name: DisableDMic_002
 * @tc.desc: Verify the DisableDMic function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, DisableDMic_002, TestSize.Level1)
{
    int32_t dhId = 0;
    std::string devId = "devId";
    sourceDev_->mic_ = std::make_shared<DMicDev>(devId, sourceDev_);
    EXPECT_NE(DH_SUCCESS, sourceDev_->DisableDMic(dhId));
}

/**
 * @tc.name: TaskOpenDSpeaker_001
 * @tc.desc: Verify the TaskOpenDSpeaker function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskOpenDSpeaker_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_EQ(ERR_DH_AUDIO_SA_SPEAKER_DEVICE_NOT_INIT, sourceDev_->TaskOpenDSpeaker(args));
}

/**
 * @tc.name: TaskOpenDSpeaker_002
 * @tc.desc: Verify the TaskOpenDSpeaker function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskOpenDSpeaker_002, TestSize.Level1)
{
    std::string args = "args";
    std::string devId = "devId";
    sourceDev_->speaker_ = std::make_shared<DSpeakerDev>(devId, sourceDev_);
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskOpenDSpeaker(args));
}

/**
 * @tc.name: TaskCloseDSpeaker_001
 * @tc.desc: Verify the TaskCloseDSpeaker function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskCloseDSpeaker_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_EQ(DH_SUCCESS, sourceDev_->TaskCloseDSpeaker(args));
}

/**
 * @tc.name: TaskCloseDSpeaker_002
 * @tc.desc: Verify the TaskCloseDSpeaker function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskCloseDSpeaker_002, TestSize.Level1)
{
    std::string args = "args";
    std::string devId = "devId";
    sourceDev_->speaker_ = std::make_shared<DSpeakerDev>(devId, sourceDev_);
    EXPECT_EQ(ERR_DH_AUDIO_FAILED, sourceDev_->TaskCloseDSpeaker(args));
}

/**
 * @tc.name: TaskOpenDMic_001
 * @tc.desc: Verify the TaskOpenDMic function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskOpenDMic_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_EQ(ERR_DH_AUDIO_SA_MIC_DEVICE_NOT_INIT, sourceDev_->TaskOpenDMic(args));
}

/**
 * @tc.name: TaskOpenDMic_002
 * @tc.desc: Verify the TaskOpenDMic function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskOpenDMic_002, TestSize.Level1)
{
    std::string args = "args";
    std::string devId = "devId";
    sourceDev_->mic_ = std::make_shared<DMicDev>(devId, sourceDev_);
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskOpenDMic(args));
}

/**
 * @tc.name: TaskCloseDMic_001
 * @tc.desc: Verify the TaskCloseDMic function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskCloseDMic_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_EQ(DH_SUCCESS, sourceDev_->TaskCloseDMic(args));
}

/**
 * @tc.name: TaskCloseDMic_002
 * @tc.desc: Verify the TaskCloseDMic function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskCloseDMic_002, TestSize.Level1)
{
    std::string args = "args";
    std::string devId = "devId";
    sourceDev_->mic_ = std::make_shared<DMicDev>(devId, sourceDev_);
    EXPECT_EQ(ERR_DH_AUDIO_FAILED, sourceDev_->TaskCloseDMic(args));
}

/**
 * @tc.name: TaskOpenCtrlChannel_001
 * @tc.desc: Verify the TaskOpenCtrlChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskOpenCtrlChannel_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskOpenCtrlChannel(args));
}

/**
 * @tc.name: TaskOpenCtrlChannel_002
 * @tc.desc: Verify the TaskOpenCtrlChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskOpenCtrlChannel_002, TestSize.Level1)
{
    std::string args = "args";
    std::string devId = "devId";
    sourceDev_->audioCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId, sourceDev_);
    EXPECT_EQ(ERR_DH_AUDIO_FAILED, sourceDev_->TaskOpenCtrlChannel(args));
}

/**
 * @tc.name: TaskCloseCtrlChannel_001
 * @tc.desc: Verify the TaskCloseCtrlChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskCloseCtrlChannel_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_EQ(DH_SUCCESS, sourceDev_->TaskCloseCtrlChannel(args));
}

/**
 * @tc.name: TaskCloseCtrlChannel_002
 * @tc.desc: Verify the TaskCloseCtrlChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskCloseCtrlChannel_002, TestSize.Level1)
{
    std::string args = "args";
    std::string devId = "devId";
    sourceDev_->audioCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId, sourceDev_);
    EXPECT_EQ(DH_SUCCESS, sourceDev_->TaskCloseCtrlChannel(args));
}

/**
 * @tc.name: TaskSetVolume_001
 * @tc.desc: Verify the TaskSetVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskSetVolume_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_EQ(ERR_DH_AUDIO_NULLPTR, sourceDev_->TaskSetVolume(args));
}

/**
 * @tc.name: TaskSetVolume_002
 * @tc.desc: Verify the TaskSetVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskSetVolume_002, TestSize.Level1)
{
    std::string args = "args";
    std::string devId = "devId";
    sourceDev_->audioCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId, sourceDev_);
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskSetVolume(args));
}

/**
 * @tc.name: TaskChangeVolume_001
 * @tc.desc: Verify the TaskChangeVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskChangeVolume_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskChangeVolume(args));
}

/**
 * @tc.name: TaskChangeFocus_001
 * @tc.desc: Verify the TaskChangeFocus function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskChangeFocus_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskChangeFocus(args));
}

/**
 * @tc.name: TaskChangeRenderState_001
 * @tc.desc: Verify the TaskChangeRenderState function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, TaskChangeRenderState_001, TestSize.Level1)
{
    std::string args = "args";
    EXPECT_NE(DH_SUCCESS, sourceDev_->TaskChangeRenderState(args));
}

/**
 * @tc.name: NotifyHDF_001
 * @tc.desc: Verify the NotifyHDF function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, NotifyHDF_001, TestSize.Level1)
{
    AudioEventType type = NOTIFY_CLOSE_MIC_RESULT;
    std::string result = "result";
    EXPECT_NE(DH_SUCCESS, sourceDev_->NotifyHDF(type, result));
}

/**
 * @tc.name: NotifyHDF_002
 * @tc.desc: Verify the NotifyHDF function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSourceDevTest, NotifyHDF_002, TestSize.Level1)
{
    AudioEventType type = NOTIFY_CLOSE_MIC_RESULT;
    std::string result = "result";
    std::string devId = "devId";
    sourceDev_->mic_ = std::make_shared<DMicDev>(devId, sourceDev_);
    EXPECT_EQ(DH_SUCCESS, sourceDev_->NotifyHDF(type, result));
}
} // namespace DistributedHardware
} // namespace OHOS
