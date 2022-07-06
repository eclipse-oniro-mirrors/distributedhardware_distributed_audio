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

#include "audio_proxy_manager.h"
#include "daudio_errcode.h"
#include "daudio_log.h"

#define HDF_LOG_TAG HDF_AUDIO_UT

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class AudioAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();

    AudioManager *managerAdapter_ = nullptr;
    AudioAdapterDescriptor *descsAdapter_ = nullptr;
    int32_t descsSizeAdapter_;
};

void AudioAdapterTest::SetUpTestCase()
{
}

void AudioAdapterTest::TearDownTestCase()
{
}

void AudioAdapterTest::SetUp()
{
    managerAdapter_ = GetAudioManagerFuncs();
    ASSERT_NE(managerAdapter_, nullptr);

    int32_t ret = managerAdapter_->GetAllAdapters(managerAdapter_, &descsAdapter_, &descsSizeAdapter_);
    EXPECT_EQ(ret, DH_SUCCESS);
    EXPECT_GT(descsSizeAdapter_, 0);
    ASSERT_NE(descsAdapter_, nullptr);
}

/**
* @tc.name: InitAllPorts_001
* @tc.desc: Verify the abnormal branch of the InitAllPorts, when param is Nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, InitAllPorts_001, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        ret = adapter->InitAllPorts(nullptr);
        EXPECT_EQ(ret, ERR_DH_AUDIO_HDF_INVALID_PARAM);

        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: CreateRender_001
* @tc.desc: Verify the abnormal branch of the CreateRender,when param is Nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, CreateRender_001, TestSize.Level1)
{
    AudioDeviceDescriptor devDesc = {};
    AudioSampleAttributes attrs = {};
    AudioRender *render = nullptr;
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        ret = adapter->CreateRender(nullptr, &devDesc, &attrs, &render);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->CreateRender(adapter, nullptr, &attrs, &render);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->CreateRender(adapter, &devDesc, nullptr, &render);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->CreateRender(adapter, &devDesc, &attrs, nullptr);
        EXPECT_NE(DH_SUCCESS, ret);
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: DestroyRender_001
* @tc.desc: Verify the abnormal branch of the DestroyRender,when param is Nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, DestroyRender_001, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        ret = adapter->DestroyRender(nullptr, nullptr);
        EXPECT_NE(DH_SUCCESS, ret);
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: CreateCapture_001
* @tc.desc: Verify the abnormal branch of the CreateCapture,when param is Nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, CreateCapture_001, TestSize.Level1)
{
    AudioDeviceDescriptor devDesc = {};
    AudioSampleAttributes attrs = {};
    AudioCapture *capture;
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        ret = adapter->CreateCapture(nullptr, &devDesc, &attrs, &capture);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->CreateCapture(adapter, nullptr, &attrs, &capture);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->CreateCapture(adapter, &devDesc, nullptr, &capture);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->CreateCapture(adapter, &devDesc, &attrs, nullptr);
        EXPECT_NE(DH_SUCCESS, ret);
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: DestroyCapture_001
* @tc.desc: Verify the abnormal branch of the DestroyCapture,when param is Nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, DestroyCapture_001, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        ret = adapter->DestroyCapture(nullptr, nullptr);
        EXPECT_NE(DH_SUCCESS, ret);
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: GetPortCapability_001
* @tc.desc: Verify the normal branch of the GetPortCapability,when param is Nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, GetPortCapability_001, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        ret = adapter->InitAllPorts(adapter);
        EXPECT_EQ(DH_SUCCESS, ret);

        for (int i = 0; i < (int)desc.portNum; i++) {
            AudioPortCapability capability = {};
            ret = adapter->GetPortCapability(adapter, &desc.ports[i], &capability);
            EXPECT_EQ(ret, DH_SUCCESS);
        }
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: GetPortCapability_002
* @tc.desc: Verify the abnormal branch of the GetPortCapability,when param is nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, GetPortCapability_002, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        ret = adapter->InitAllPorts(adapter);
        EXPECT_EQ(DH_SUCCESS, ret);

        for (int i = 0; i < (int)desc.portNum; i++) {
            AudioPortCapability capability = {};
            ret = adapter->GetPortCapability(nullptr, &desc.ports[i], &capability);
            EXPECT_NE(DH_SUCCESS, ret);
            ret = adapter->GetPortCapability(adapter, nullptr, &capability);
            EXPECT_NE(DH_SUCCESS, ret);
            ret = adapter->GetPortCapability(adapter, &desc.ports[i], nullptr);
            EXPECT_NE(DH_SUCCESS, ret);
        }
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: SetPassthroughMode_001
* @tc.desc: Verify the normal branch of the SetPassthroughMode.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, SetPassthroughMode_001, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        for (int i = 0; i < (int)desc.portNum; i++) {
            AudioPortPassthroughMode mode = PORT_PASSTHROUGH_AUTO;
            ret = adapter->SetPassthroughMode(adapter, &desc.ports[i], mode);
            EXPECT_EQ(DH_SUCCESS, ret);
        }
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: SetPassthroughMode_002
* @tc.desc: Verify the abnormal branch of the SetPassthroughMode,when param is nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, SetPassthroughMode_002, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        for (int i = 0; i < (int)desc.portNum; i++) {
            AudioPortPassthroughMode mode = PORT_PASSTHROUGH_AUTO;
            ret = adapter->SetPassthroughMode(nullptr, &desc.ports[i], mode);
            EXPECT_NE(DH_SUCCESS, ret);
            ret = adapter->SetPassthroughMode(adapter, nullptr, mode);
            EXPECT_NE(DH_SUCCESS, ret);
        }
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: GetPassthroughMode_001
* @tc.desc: Verify the normal branch of the GetPassthroughMode.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, GetPassthroughMode_001, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        for (int i = 0; i < (int)desc.portNum; i++) {
            AudioPortPassthroughMode mode = PORT_PASSTHROUGH_AUTO;
            ret = adapter->GetPassthroughMode(adapter, &desc.ports[i], &mode);
            EXPECT_EQ(DH_SUCCESS, ret);
        }
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: GetPassthroughMode_002
* @tc.desc: Verify the abnormal branch of the GetPassthroughMode,when param is nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, GetPassthroughMode_002, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        for (int i = 0; i < (int)desc.portNum; i++) {
            AudioPortPassthroughMode mode = PORT_PASSTHROUGH_AUTO;
            ret = adapter->GetPassthroughMode(nullptr, &desc.ports[i], &mode);
            EXPECT_NE(DH_SUCCESS, ret);
            ret = adapter->GetPassthroughMode(adapter, nullptr, &mode);
            EXPECT_NE(DH_SUCCESS, ret);
        }
        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}

/**
* @tc.name: UpdateAudioRoute_001
* @tc.desc: Verify the abnormal branch of the UpdateAudioRoute,when param is nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, UpdateAudioRoute_001, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        AudioRoute route = {0};
        int32_t routeHandle = -1;
        ret = adapter->UpdateAudioRoute(nullptr, &route, &routeHandle);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->UpdateAudioRoute(adapter, nullptr, &routeHandle);
        EXPECT_NE(DH_SUCCESS, ret);
        ret = adapter->UpdateAudioRoute(adapter, &route, nullptr);
        EXPECT_NE(DH_SUCCESS, ret);
    }
}

/**
* @tc.name: UpdateAudioRoute_002
* @tc.desc: Verify the normal branch of the UpdateAudioRoute.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, UpdateAudioRoute_002, TestSize.Level1)
{
    for (int32_t index = 0; index < descsSizeAdapter_; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descsAdapter_[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        int32_t ret = managerAdapter_->LoadAdapter(managerAdapter_, &desc, &adapter);
        EXPECT_EQ(ret, DH_SUCCESS);
        ASSERT_NE(adapter, nullptr);

        AudioRouteNode source = {
            .portId = index,
            .role = AUDIO_PORT_SOURCE_ROLE,
            .type = AUDIO_PORT_DEVICE_TYPE,
            .ext.device.moduleId = 0,
            .ext.device.type = PIN_IN_MIC,
            .ext.device.desc = "pin_in_mic",
        };

        AudioRouteNode sink = {
            .portId = index,
            .role = AUDIO_PORT_SINK_ROLE,
            .type = AUDIO_PORT_DEVICE_TYPE,
            .ext.device.moduleId = 0,
            .ext.device.type = PIN_OUT_SPEAKER,
            .ext.device.desc = "pin_out_speaker",
        };

        AudioRoute route = {
            .sourcesNum = 1,
            .sources = &source,
            .sinksNum = 1,
            .sinks = &sink,
        };

        int32_t routeHandle = -1;
        ret = adapter->UpdateAudioRoute(adapter, &route, &routeHandle);
        EXPECT_EQ(DH_SUCCESS, ret);
        if (ret == DH_SUCCESS) {
            ret = adapter->ReleaseAudioRoute(adapter, routeHandle);
            EXPECT_EQ(DH_SUCCESS, ret);
        }

        managerAdapter_->UnloadAdapter(managerAdapter_, adapter);
    }
}
} // DistributedHardware
} // OHOS
