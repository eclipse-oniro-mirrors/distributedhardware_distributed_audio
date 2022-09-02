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

#include "audio_ctrl_channel_test.h"
#include <iostream>
#include "daudio_log.h"
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void AudioCtrlChannelTest::SetUpTestCase(void) {}

void AudioCtrlChannelTest::TearDownTestCase(void) {}

void AudioCtrlChannelTest::SetUp(void)
{
    std::string peerDevId = "peerDevId";
    ctrlChannel_ = std::make_shared<AudioCtrlChannel>(peerDevId);
}

void AudioCtrlChannelTest::TearDown(void)
{
    ctrlChannel_ = nullptr;
}

/**
 * @tc.name: CreateSession_001
 * @tc.desc: Verify the CreateSession and ReleaseSession function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, CreateSession_001, TestSize.Level1)
{
    std::shared_ptr<IAudioChannelListener> listener = nullptr;
    EXPECT_NE(DH_SUCCESS, ctrlChannel_->CreateSession(listener, CTRL_SESSION_NAME));
    EXPECT_EQ(DH_SUCCESS, ctrlChannel_->ReleaseSession());
}

/**
 * @tc.name: OpenSession_002
 * @tc.desc: Verify the OpenSession function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, OpenSession_002, TestSize.Level1)
{
    EXPECT_EQ(ERR_DH_AUDIO_TRANS_ERROR, ctrlChannel_->OpenSession());
    EXPECT_EQ(DH_SUCCESS, ctrlChannel_->CloseSession());
}

/**
 * @tc.name: SendEvent_001
 * @tc.desc: Verify the SendEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, SendEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioEvent> event = nullptr;
    EXPECT_EQ(ERR_DH_AUDIO_TRANS_NULL_VALUE, ctrlChannel_->SendEvent(event));
}

/**
 * @tc.name: SendEvent_002
 * @tc.desc: Verify the SendEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, SendEvent_002, TestSize.Level1)
{
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    EXPECT_EQ(ERR_DH_AUDIO_TRANS_ERROR, ctrlChannel_->SendEvent(event));
}

/**
 * @tc.name: OnSessionOpened_001
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, OnSessionOpened_001, TestSize.Level1)
{
    std::shared_ptr<IAudioChannelListener> listener = std::make_shared<MockIAudioChannelListener>();
    ctrlChannel_->channelListener_ = listener;
    int32_t sessionId = 0;
    int32_t result = 0;

    ctrlChannel_->OnSessionOpened(sessionId, result);
}

/**
 * @tc.name: OnSessionOpened_002
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, OnSessionOpened_002, TestSize.Level1)
{
    std::shared_ptr<IAudioChannelListener> listener = std::make_shared<MockIAudioChannelListener>();
    ctrlChannel_->channelListener_ = listener;
    int32_t sessionId = -1;
    int32_t result = -1;

    ctrlChannel_->OnSessionOpened(sessionId, result);
}

/**
 * @tc.name: OnSessionOpened_003
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, OnSessionOpened_003, TestSize.Level1)
{
    std::shared_ptr<IAudioChannelListener> listener = nullptr;
    ctrlChannel_->channelListener_ = listener;
    int32_t sessionId = 0;
    int32_t result = 0;

    ctrlChannel_->OnSessionOpened(sessionId, result);
}

/**
 * @tc.name: OnSessionClosed_001
 * @tc.desc: Verify the OnSessionClosed function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, OnSessionClosed_001, TestSize.Level1)
{
    std::shared_ptr<IAudioChannelListener> listener = nullptr;
    ctrlChannel_->channelListener_ = listener;
    int32_t sessionId = 0;

    ctrlChannel_->OnSessionClosed(sessionId);
}

/**
 * @tc.name: OnBytesReceived_001
 * @tc.desc: Verify the OnBytesReceived function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioCtrlChannelTest, OnBytesReceived_001, TestSize.Level1)
{
    std::shared_ptr<IAudioChannelListener> listener = std::make_shared<MockIAudioChannelListener>();
    ctrlChannel_->channelListener_ = listener;

    int32_t sessionId = 0;
    uint8_t *data = nullptr;
    int32_t dataLen = 0;
    ctrlChannel_->OnBytesReceived(sessionId, data, dataLen);
}
} // namespace DistributedHardware
} // namespace OHOS
