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

#include "decode_transport_test.h"

#include <memory>

#include "audio_data.h"
#include "audio_event.h"
#include "audio_param.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"
#include "mock_audio_data_channel.h"
#include "mock_audio_processor.h"
#include "mock_audio_transport_callback.h"
#include "securec.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
const std::string RMT_DEV_ID_TEST = "RemoteTest";
const std::string ROLE_TEST = "speaker";

void DecodeTransportTest::SetUpTestCase(void)
{
}

void DecodeTransportTest::TearDownTestCase(void)
{
}

void DecodeTransportTest::SetUp(void)
{
    transCallback_ = std::make_shared<MockAudioTransportCallback>();
    decodeTrans_ = std::make_shared<AudioDecodeTransport>(RMT_DEV_ID_TEST);
}

void DecodeTransportTest::TearDown(void)
{
    transCallback_ = nullptr;
    decodeTrans_ = nullptr;
}

/**
 * @tc.name: decode_transport_test_001
 * @tc.desc: Verify the configure processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_001, TestSize.Level1)
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
    EXPECT_NE(DH_SUCCESS, decodeTrans_->SetUp(testLocalParaEnc, testRemoteParaEnc, transCallback_, ROLE_TEST));
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_002
 * @tc.desc: Verify the start processor without configure processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_002, TestSize.Level1)
{
    EXPECT_NE(DH_SUCCESS, decodeTrans_->Start());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_003
 * @tc.desc: Verify the pause and  processor without configure processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_003, TestSize.Level1)
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
    EXPECT_EQ(ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION, decodeTrans_->Pause());
    EXPECT_EQ(ERR_DH_AUDIO_TRANS_ILLEGAL_OPERATION, decodeTrans_->Restart(testLocalParaEnc, testRemoteParaEnc));
}
} // namespace DistributedHardware
} // namespace OHOS
