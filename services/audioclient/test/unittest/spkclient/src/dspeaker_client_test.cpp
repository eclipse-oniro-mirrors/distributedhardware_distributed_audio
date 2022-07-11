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

#include "dspeaker_client_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DSpeakerClientTest::SetUpTestCase(void) {}

void DSpeakerClientTest::TearDownTestCase(void) {}

void DSpeakerClientTest::SetUp()
{
    std::string devId = "hello";
    std::shared_ptr<IAudioEventCallback> callback = std::make_shared<MockIAudioEventCallback>();
    speakerClient_ = std::make_shared<DSpeakerClient>(devId, callback);
    speakerClient_->speakerTrans_ = std::make_shared<MockIAudioDataTransport>();

    audioParam_.comParam.codecType = AudioCodecType::AUDIO_CODEC_AAC;
    audioParam_.comParam.sampleRate = AudioSampleRate::SAMPLE_RATE_48000;
    audioParam_.comParam.bitFormat = AudioSampleFormat::SAMPLE_S16LE;
    audioParam_.comParam.channelMask = AudioChannel::STEREO;
    audioParam_.renderOpts.contentType = ContentType::CONTENT_TYPE_MUSIC;
    audioParam_.renderOpts.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
}

void DSpeakerClientTest::TearDown()
{
    speakerClient_ = nullptr;
}

/**
 * @tc.name: OnStateChange_001
 * @tc.desc: Verify the OnStateChange function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, OnStateChange_001, TestSize.Level1)
{
    int32_t type = 15;
    EXPECT_NE(DH_SUCCESS, speakerClient_->OnStateChange(type));
}

/**
 * @tc.name: OnStateChange_002
 * @tc.desc: Verify the OnStateChange function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, OnStateChange_002, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, speakerClient_->OnStateChange(AudioEventType::DATA_CLOSED));
}

/**
 * @tc.name: SetUp_001
 * @tc.desc: Verify the SetUp function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, SetUp_001, TestSize.Level1)
{
    AudioParam audioParam;
    EXPECT_NE(DH_SUCCESS, speakerClient_->SetUp(audioParam));
}

/**
 * @tc.name: SetUp_002
 * @tc.desc: Verify the SetUp function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, SetUp_002, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, speakerClient_->SetUp(audioParam_));
}

/**
 * @tc.name: StartRender_001
 * @tc.desc: Verify the StartRender and StopRender function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, StartRender001, TestSize.Level1)
{
    EXPECT_NE(DH_SUCCESS, speakerClient_->StartRender());
    EXPECT_NE(DH_SUCCESS, speakerClient_->StopRender());
}

/**
 * @tc.name: StartRender002
 * @tc.desc: Verify the StartRender function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, StartRender002, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, speakerClient_->SetUp(audioParam_));
    EXPECT_EQ(DH_SUCCESS, speakerClient_->StartRender());
    EXPECT_EQ(DH_SUCCESS, speakerClient_->StopRender());
}

/**
 * @tc.name: StopRender_001
 * @tc.desc: Verify the StopRender function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, StopRender001, TestSize.Level1)
{
    EXPECT_NE(DH_SUCCESS, speakerClient_->StopRender());
}

/**
 * @tc.name: SetAudioParameters_001
 * @tc.desc: Verify the SetAudioParameters function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, SetAudioParameters001, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, speakerClient_->SetUp(audioParam_));
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::VOLUME_GET;
    event->content = "EVENT_TYPE=1;VOLUME_GROUP_ID=1;AUDIO_VOLUME_TYPE=1;VOLUME_LEVEL=6;";
    EXPECT_NE(DH_SUCCESS, speakerClient_->SetAudioParameters(event));
}

/**
 * @tc.name: SetAudioParameters_002
 * @tc.desc: Verify the SetAudioParameters function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6G
 */
HWTEST_F(DSpeakerClientTest, SetAudioParameters002, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, speakerClient_->SetUp(audioParam_));
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::VOLUME_MUTE_SET;
    event->content = "EVENT_TYPE=1;VOLUME_GROUP_ID=1;AUDIO_VOLUME_TYPE=1;VOLUME_LEVEL=2;";
    EXPECT_EQ(DH_SUCCESS, speakerClient_->SetAudioParameters(event));
}
} // DistributedHardware
} // OHOS
