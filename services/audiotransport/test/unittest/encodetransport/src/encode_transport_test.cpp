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

#include "encode_transport_test.h"

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
const std::string RMT_DEV_ID_TEST = "RemoteTest";
const std::string ROLE_TEST = "speaker";

void EncodeTransportTest::SetUpTestCase(void)
{
}

void EncodeTransportTest::TearDownTestCase(void)
{
}

void EncodeTransportTest::SetUp(void)
{
    transCallback_ = std::make_shared<MockAudioTransportCallback>();
    encodeTrans_ = std::make_shared<AudioEncodeTransport>(RMT_DEV_ID_TEST);
}

void EncodeTransportTest::TearDown(void)
{
    transCallback_ = nullptr;
    encodeTrans_ = nullptr;
}

/**
 * @tc.name: encode_transport_test_002
 * @tc.desc: Verify the configure transport function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(EncodeTransportTest, encode_transport_test_002, TestSize.Level1)
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
    EXPECT_NE(DH_SUCCESS, encodeTrans_->SetUp(testLocalParaEnc, testRemoteParaEnc, transCallback_, ROLE_TEST));
    EXPECT_EQ(DH_SUCCESS, encodeTrans_->Release());
}

/**
 * @tc.name: encode_transport_test_004
 * @tc.desc: Verify the start transport without configure transport function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5U
 */
HWTEST_F(EncodeTransportTest, encode_transport_test_004, TestSize.Level1)
{
    EXPECT_NE(DH_SUCCESS, encodeTrans_->Start());
    EXPECT_NE(DH_SUCCESS, encodeTrans_->Stop());
    EXPECT_EQ(DH_SUCCESS, encodeTrans_->Release());
}
} // namespace DistributedHardware
} // namespace OHOS
