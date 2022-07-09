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

#include "daudio_hdi_handler_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DAudioHdiHandlerTest::SetUpTestCase(void) {}

void DAudioHdiHandlerTest::TearDownTestCase(void) {}

void DAudioHdiHandlerTest::SetUp()
{
    hdiHandler_ = std::make_shared<DAudioHdiHandler>();
}

void DAudioHdiHandlerTest::TearDown()
{
    hdiHandler_ = nullptr;
}

/**
 * @tc.name: InitHdiHandler_001
 * @tc.desc: Verify the InitHdiHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(DAudioHdiHandlerTest, InitHdiHandler_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->InitHdiHandler());
}

/**
 * @tc.name: RegisterAudioDevice_001
 * @tc.desc: Verify the RegisterAudioDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(DAudioHdiHandlerTest, RegisterAudioDevice_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->InitHdiHandler());
    hdiHandler_->audioSrvHdf_ = nullptr;
    std::shared_ptr<IDAudioHdiCallback> callbackObjParam = std::make_shared<MockIDAudioHdiCallback>();
    EXPECT_NE(HDF_SUCCESS, hdiHandler_->RegisterAudioDevice(devId_, dhId_, capability_, callbackObjParam));
    EXPECT_NE(HDF_SUCCESS, hdiHandler_->UnRegisterAudioDevice(devId_, dhId_));
}

/**
 * @tc.name: RegisterAudioDevice_002
 * @tc.desc: Verify the RegisterAudioDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(DAudioHdiHandlerTest, RegisterAudioDevice_002, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->InitHdiHandler());
    std::shared_ptr<IDAudioHdiCallback> callbackObjParam = std::make_shared<MockIDAudioHdiCallback>();
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->RegisterAudioDevice(devId_, dhId_, capability_, callbackObjParam));
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->UnRegisterAudioDevice(devId_, dhId_));
}

/**
 * @tc.name: NotifyEvent_001
 * @tc.desc: Verify the NotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(DAudioHdiHandlerTest, NotifyEvent_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->InitHdiHandler());
    hdiHandler_->audioSrvHdf_ = nullptr;
    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    EXPECT_NE(HDF_SUCCESS, hdiHandler_->NotifyEvent(devId_, dhId_, audioEvent));
}

/**
 * @tc.name: NotifyEvent_002
 * @tc.desc: Verify the NotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(DAudioHdiHandlerTest, NotifyEvent_002, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->InitHdiHandler());
    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    audioEvent->type = AudioEventType::SPEAKER_OPENED;
    audioEvent->content = "";
    EXPECT_EQ(HDF_SUCCESS, hdiHandler_->NotifyEvent(devId_, dhId_, audioEvent));
}

/**
 * @tc.name: UnRegisterAudioDevice_001
 * @tc.desc: Verify the UnRegisterAudioDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(DAudioHdiHandlerTest, UnRegisterAudioDevice_001, TestSize.Level1)
{
    hdiHandler_->audioSrvHdf_ = nullptr;
    EXPECT_NE(HDF_SUCCESS, hdiHandler_->UnRegisterAudioDevice(devId_, dhId_));
}
} // DistributedHardware
} // OHOS
