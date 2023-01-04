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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>

#include "daudio_capture_internal.h"
#include "audio_capture.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"

#define HDF_LOG_TAG HDF_AUDIO_UT

using namespace std;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
class AudioCaptureTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AudioCaptureTest::SetUpTestCase()
{
}

void AudioCaptureTest::TearDownTestCase()
{
}

/**
* @tc.name: GetCapturePositionInternal
* @tc.desc: Verify the abnormal branch of the GetCapturePositionInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioCaptureTest, GetCapturePositionInternal_001, TestSize.Level1)
{
    struct AudioCaptureContext captureContext;
    struct AudioCapture *capture = nullptr;
    uint64_t *frames = nullptr;
    struct ::AudioTimeStamp *time = nullptr;
    int32_t ret = captureContext.instance_.GetCapturePosition(capture, frames, time);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: GetCapturePositionInternal
* @tc.desc: Verify the abnormal branch of the GetCapturePositionInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioCaptureTest, GetCapturePositionInternal_002, TestSize.Level1)
{
    struct AudioCaptureContext captureContext;
    struct AudioCapture *capture = new AudioCapture;
    uint64_t *frames = new uint64_t;
    struct ::AudioTimeStamp *time = new ::AudioTimeStamp;
    int32_t ret = captureContext.instance_.GetCapturePosition(capture, frames, time);
    delete capture;
    delete time;
    EXPECT_EQ(ERR_DH_AUDIO_HDI_NULLPTR, ret);
}

/**
* @tc.name: CaptureFrameInternal
* @tc.desc: Verify the abnormal branch of the CaptureFrameInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioCaptureTest, CaptureFrameInternal_001, TestSize.Level1)
{
    struct AudioCaptureContext captureContext;
    struct AudioCapture *capture = nullptr;
    void *frame = nullptr;
    uint64_t requestBytes = 0;
    uint64_t *replyBytes = nullptr;
    int32_t ret = captureContext.instance_.CaptureFrame(capture, frame, requestBytes, replyBytes);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: CaptureFrameInternal
* @tc.desc: Verify the abnormal branch of the CaptureFrameInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioCaptureTest, CaptureFrameInternal_002, TestSize.Level1)
{
    struct AudioCaptureContext captureContext;
    struct AudioCapture *capture = new AudioCapture;
    char p = 'c';
    void *frame = &p;
    uint64_t requestBytes = 1;
    uint64_t *replyBytes = new uint64_t;
    int32_t ret = captureContext.instance_.CaptureFrame(capture, frame, requestBytes, replyBytes);
    delete capture;
    delete replyBytes;
    EXPECT_EQ(ERR_DH_AUDIO_HDI_NULLPTR, ret);
}
} // DistributedHardware
} // OHOS