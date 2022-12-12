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

#define private public
#include "daudio_sink_handler_test.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DAudioSinkHandlerTest::SetUpTestCase(void)
{
    DAudioSinkHandler::GetInstance().InitSink("DAudioSinkHandlerTest");
}

void DAudioSinkHandlerTest::TearDownTestCase(void)
{
    DAudioSinkHandler::GetInstance().ReleaseSink();
}

void DAudioSinkHandlerTest::SetUp(void) {}

void DAudioSinkHandlerTest::TearDown(void) {}

/**
 * @tc.name: LocalHardware_001
 * @tc.desc: Verify the SubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSinkHandlerTest, LocalHardware_001, TestSize.Level1)
{
    const std::string dhId = "DAudioSinkHandlerTest";
    const std::string param = "DAudioSinkHandlerTest";
    int32_t ret = DAudioSinkHandler::GetInstance().SubscribeLocalHardware(dhId, param);
    EXPECT_EQ(DH_SUCCESS, ret);
    ret = DAudioSinkHandler::GetInstance().UnsubscribeLocalHardware(dhId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: LocalHardware_002
 * @tc.desc: Verify the SubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSinkHandlerTest, LocalHardware_002, TestSize.Level1)
{
    size_t DAUDIO_MAX_DEVICE_ID_LEN = 101;
    std::string dhId;
    dhId.resize(DAUDIO_MAX_DEVICE_ID_LEN);
    const std::string param = "DAudioSinkHandlerTest";
    int32_t ret = DAudioSinkHandler::GetInstance().SubscribeLocalHardware(dhId, param);
    EXPECT_EQ(ERR_DH_AUDIO_SA_DEVID_ILLEGAL, ret);
    ret = DAudioSinkHandler::GetInstance().UnsubscribeLocalHardware(dhId);
    EXPECT_EQ(ERR_DH_AUDIO_SA_DEVID_ILLEGAL, ret);
}

/**
 * @tc.name: LocalHardware_003
 * @tc.desc: Verify the SubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E5F
 */
HWTEST_F(DAudioSinkHandlerTest, LocalHardware_003, TestSize.Level1)
{
    const std::string dhId = "DAudioSinkHandlerTest";
    const std::string param = "DAudioSinkHandlerTest";
    DAudioSinkHandler::GetInstance().dAudioSinkProxy_ = nullptr;
    int32_t ret = DAudioSinkHandler::GetInstance().SubscribeLocalHardware(dhId, param);
    EXPECT_EQ(ERR_DH_AUDIO_SA_PROXY_NOT_INIT, ret);
    ret = DAudioSinkHandler::GetInstance().UnsubscribeLocalHardware(dhId);
    EXPECT_EQ(ERR_DH_AUDIO_SA_PROXY_NOT_INIT, ret);
    wptr<IRemoteObject> remote = nullptr;
    DAudioSinkHandler::GetInstance().OnRemoteSinkSvrDied(remote);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID);
    wptr<IRemoteObject> remoteobject (remoteObject);
    DAudioSinkHandler::GetInstance().OnRemoteSinkSvrDied(remoteobject);
}
} // namespace DistributedHardware
} // namespace OHOS
