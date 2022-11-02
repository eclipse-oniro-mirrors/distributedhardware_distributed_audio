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

#include "audio_transport_stop_status_test.h"

#include "audio_transport_context.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"
#include "mock_audio_data_channel.h"
#include "mock_audio_processor.h"
#include "securec.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

void AudioTransportStopStatusTest::SetUpTestCase(void)
{
}

void AudioTransportStopStatusTest::TearDownTestCase(void)
{
}

void AudioTransportStopStatusTest::SetUp(void)
{
    audioChannel_ = std::shared_ptr<MockAudioDataChannel>();
    processor_ = std::shared_ptr<MockIAudioProcessor>();
    stateContext_ = std::shared_ptr<AudioTransportContext>();
    audioStatus_ = std::make_shared<AudioTransportStopStatus>(stateContext_);
}

void AudioTransportStopStatusTest::TearDown(void)
{
    audioChannel_ = nullptr;
    processor_ = nullptr;
    stateContext_ = nullptr;
    audioStatus_ = nullptr;
}

/**
 * @tc.name: transport_stop_test_001
 * @tc.desc: Verify start action when status is stop.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioTransportStopStatusTest, transport_stop_test_001, TestSize.Level1)
{
    EXPECT_EQ(ERR_DH_AUDIO_NULLPTR, audioStatus_->Start(audioChannel_, processor_));
}

/**
 * @tc.name: transport_stop_test_002
 * @tc.desc: Verify stop action when status is stop.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioTransportStopStatusTest, transport_stop_test_002, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, audioStatus_->Stop(audioChannel_, processor_));
}

/**
 * @tc.name: transport_stop_test_003
 * @tc.desc: Verify pause action when status is stop.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioTransportStopStatusTest, transport_stop_test_003, TestSize.Level1)
{
    EXPECT_EQ(ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION, audioStatus_->Pause(processor_));
}

/**
 * @tc.name: transport_stop_test_004
 * @tc.desc: Verify reStart action when status is stop.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(AudioTransportStopStatusTest, transport_stop_test_004, TestSize.Level1)
{
    AudioParam testLocalParaEnc = {
        {
            SAMPLE_RATE_48000,
            STEREO,
            SAMPLE_S16LE,
            AUDIO_CODEC_FLAC
        },
        {
            SOURCE_TYPE_INVALID,
            0
        },
        {
            CONTENT_TYPE_UNKNOWN,
            STREAM_USAGE_UNKNOWN,
            0
        }
    };
    AudioParam testRemoteParaEnc = {
        {
            SAMPLE_RATE_48000,
            STEREO,
            SAMPLE_S16LE,
            AUDIO_CODEC_FLAC
        },
        {
            SOURCE_TYPE_INVALID,
            0
        },
        {
            CONTENT_TYPE_UNKNOWN,
            STREAM_USAGE_UNKNOWN,
            0
        }
    };
    EXPECT_EQ(ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION,
        audioStatus_->Restart(testLocalParaEnc, testRemoteParaEnc, processor_));
}
} // namespace DistributedHardware
} // namespace OHOS
