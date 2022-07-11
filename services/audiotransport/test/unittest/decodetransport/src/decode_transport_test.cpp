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
#include "mock_audio_transport_callback.h"
#include "securec.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
const AudioParam LOC_PARA_ENC_TEST = {
    {
        SAMPLE_RATE_48000,
        STEREO,
        SAMPLE_S16LE,
        AUDIO_CODEC_AAC
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
const AudioParam RMT_PARA_ENC_TEST = {
    {
        SAMPLE_RATE_48000,
        STEREO,
        SAMPLE_S16LE,
        AUDIO_CODEC_AAC
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
 * @tc.desc: Verify the configure and release processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_001, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_002
 * @tc.desc: Verify the configure processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_002, TestSize.Level1)
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
 * @tc.name: decode_transport_test_003
 * @tc.desc: Verify the start and stop processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_003, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_004
 * @tc.desc: Verify the start processor without configure processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_004, TestSize.Level1)
{
    EXPECT_NE(DH_SUCCESS, decodeTrans_->Start());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_005
 * @tc.desc: Verify the stop processor without start processor function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_005, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_NE(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_006
 * @tc.desc: Verify the start and stop processor again function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_006, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());

    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_007
 * @tc.desc: Verify the start processor again function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_007, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());
    EXPECT_NE(DH_SUCCESS, decodeTrans_->Start());

    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_008
 * @tc.desc: Verify the stop processor again function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_008, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());

    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_NE(DH_SUCCESS, decodeTrans_->Stop());

    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_009
 * @tc.desc: Verify feed audio transport function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_009, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());

    std::shared_ptr<AudioData> inputData = nullptr;
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->FeedAudioData(inputData));

    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_010
 * @tc.desc: Verify request audio transport function, when decoded audio data queue is empty.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_010, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());

    std::shared_ptr<AudioData> outputData = nullptr;
    EXPECT_NE(DH_SUCCESS, decodeTrans_->RequestAudioData(outputData));
    EXPECT_EQ(outputData, nullptr);

    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}

/**
 * @tc.name: decode_transport_test_011
 * @tc.desc: Verify request audio transport function, when decoded audio data queue is not empty.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(DecodeTransportTest, decode_transport_test_011, TestSize.Level1)
{
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->SetUp(LOC_PARA_ENC_TEST, RMT_PARA_ENC_TEST, transCallback_, ROLE_TEST));
    EXPECT_NE(decodeTrans_->audioChannel_, nullptr);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->audioChannel_->ReleaseSession());
    decodeTrans_->audioChannel_ = std::make_shared<MockAudioDataChannel>(RMT_DEV_ID_TEST);
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Start());

    size_t bufLen = 305;
    std::shared_ptr<AudioData> decodedData = std::make_shared<AudioData>(bufLen);
    decodeTrans_->dataQueue_.push(decodedData);
    std::shared_ptr<AudioData> outputData = nullptr;
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->RequestAudioData(outputData));
    EXPECT_NE(outputData, nullptr);
    EXPECT_EQ(outputData->Size(), decodedData->Size());

    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, decodeTrans_->Release());
}
} // namespace DistributedHardware
} // namespace OHOS
