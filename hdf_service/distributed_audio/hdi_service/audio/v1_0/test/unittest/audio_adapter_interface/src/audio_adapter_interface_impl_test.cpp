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

#include "audio_adapter_interface_impl_test.h"

using namespace testing::ext;

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
void AudioAdapterInterfaceImpTest::SetUpTestCase(void) {}

void AudioAdapterInterfaceImpTest::TearDownTestCase(void) {}

void AudioAdapterInterfaceImpTest::SetUp(void)
{
    AudioAdapterDescriptorHAL adaDesc;
    AdapterTest_ = std::make_shared<AudioAdapterInterfaceImpl>(adaDesc);
}

void AudioAdapterInterfaceImpTest::TearDown(void)
{
    AdapterTest_ = nullptr;
}

/**
 * @tc.name: InitAllPorts_001
 * @tc.desc: Verify the InitAllPorts function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, InitAllPorts_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->InitAllPorts());
}

/**
 * @tc.name: CreateRender_001
 * @tc.desc: Verify the CreateRender function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, CreateRender_001, TestSize.Level1)
{
    AudioDeviceDescriptorHAL devDesc;
    AudioSampleAttributesHAL attrs;
    sptr<IAudioRender> render = nullptr;
    AdapterTest_->extSpeakerCallback_ = new MockIDAudioCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->CreateRender(devDesc, attrs, render));
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->DestoryRender(devDesc));
}

/**
 * @tc.name: CreateCapture_001
 * @tc.desc: Verify the CreateCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, CreateCapture_001, TestSize.Level1)
{
    AudioDeviceDescriptorHAL devDesc;
    AudioSampleAttributesHAL attrs;
    sptr<IAudioCapture> capture = nullptr;
    AdapterTest_->extMicCallback_ = new MockIDAudioCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->CreateCapture(devDesc, attrs, capture));
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->DestoryCapture(devDesc));
}

/**
 * @tc.name: GetPortCapability_001
 * @tc.desc: Verify the GetPortCapability function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetPortCapability_001, TestSize.Level1)
{
    AudioPortHAL port;
    AudioPortCapabilityHAl capability;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetPortCapability(port, capability));
}

/**
 * @tc.name: SetPassthroughMode_001
 * @tc.desc: Verify the SetPassthroughMode function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetPassthroughMode_001, TestSize.Level1)
{
    AudioPortHAL port;
    AudioPortPassthroughModeHAL mode = AudioPortPassthroughModeHAL::PORT_PASSTHROUGH_LPCM;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->SetPassthroughMode(port, mode));
}

/**
 * @tc.name: GetPassthroughMode_001
 * @tc.desc: Verify the GetPassthroughMode function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetPassthroughMode_001, TestSize.Level1)
{
    AudioPortHAL port;
    AudioPortPassthroughModeHAL mode = AudioPortPassthroughModeHAL::PORT_PASSTHROUGH_LPCM;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetPassthroughMode(port, mode));
}

/**
 * @tc.name: UpdateAudioRoute_001
 * @tc.desc: Verify the UpdateAudioRoute function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, UpdateAudioRoute_001, TestSize.Level1)
{
    AudioRouteHAL route;
    int32_t handle = 0;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->UpdateAudioRoute(route, handle));
}

/**
 * @tc.name: ReleaseAudioRoute_001
 * @tc.desc: Verify the ReleaseAudioRoute function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, ReleaseAudioRoute_001, TestSize.Level1)
{
    int32_t handle = 0;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->ReleaseAudioRoute(handle));
}

/**
 * @tc.name: SetAudioParameters_001
 * @tc.desc: Verify the SetAudioParameters function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetAudioParameters_001, TestSize.Level1)
{
    AudioExtParamKeyHAL key = AudioExtParamKeyHAL::AUDIO_EXT_PARAM_KEY_NONE;
    std::string condition = "hello";
    std::string value = "world";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->SetAudioParameters(key, condition, value));
}

/**
 * @tc.name: GetAudioParameters_001
 * @tc.desc: Verify the GetAudioParameters function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetAudioParameters_001, TestSize.Level1)
{
    AudioExtParamKeyHAL key = AudioExtParamKeyHAL::AUDIO_EXT_PARAM_KEY_NONE;
    std::string condition = "hello";
    std::string value = "world";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetAudioParameters(key, condition, value));
}

/**
 * @tc.name: RegAudioParamObserver_001
 * @tc.desc: Verify the RegAudioParamObserver function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, RegAudioParamObserver_001, TestSize.Level1)
{
    sptr<IAudioParamCallback> cbObj = nullptr;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->RegAudioParamObserver(cbObj));
}

/**
 * @tc.name: RegAudioParamObserver_002
 * @tc.desc: Verify the RegAudioParamObserver function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, RegAudioParamObserver_002, TestSize.Level1)
{
    AdapterTest_->paramCallback_ = new MockIAudioParamCallback();
    sptr<IAudioParamCallback> cbObj = new MockIAudioParamCallback();
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->RegAudioParamObserver(cbObj));
}

/**
 * @tc.name: AdapterLoad_001
 * @tc.desc: Verify the AdapterLoad function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, AdapterLoad_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->AdapterLoad());
}

/**
 * @tc.name: AdapterUnload_001
 * @tc.desc: Verify the AdapterUnload function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, AdapterUnload_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->AdapterUnload());
}

/**
 * @tc.name: Notify_001
 * @tc.desc: Verify the Notify function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, Notify_001, TestSize.Level1)
{
    AudioEvent event;
    event.type = 3;
    event.content = "VOLUME_LEVEL";
    uint32_t devId = 64;
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->Notify(devId, event));
}

/**
 * @tc.name: AddAudioDevice_001
 * @tc.desc: Verify the AddAudioDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, AddAudioDevice_001, TestSize.Level1)
{
    uint32_t devId = 64;
    std::string caps;
    AdapterTest_->mapAudioDevice_.insert(std::make_pair(64, "hello"));
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->AddAudioDevice(devId, caps));
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->RemoveAudioDevice(devId));
}

/**
 * @tc.name: AddAudioDevice_002
 * @tc.desc: Verify the AddAudioDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, AddAudioDevice_002, TestSize.Level1)
{
    uint32_t devId = 64;
    std::string caps = "hello";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->AddAudioDevice(devId, caps));
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->RemoveAudioDevice(devId));
}

/**
 * @tc.name: OpenRenderDevice_001
 * @tc.desc: Verify the OpenRenderDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, OpenRenderDevice_001, TestSize.Level1)
{
    AudioDeviceDescriptorHAL devDesc;
    AudioSampleAttributesHAL attrs;
    AdapterTest_->extSpeakerCallback_ = new MockIDAudioCallback;
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->OpenRenderDevice(devDesc, attrs));
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->CloseRenderDevice(devDesc));
}

/**
 * @tc.name: OpenCaptureDevice_001
 * @tc.desc: Verify the OpenCaptureDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, OpenCaptureDevice_001, TestSize.Level1)
{
    AudioDeviceDescriptorHAL devDesc;
    AudioSampleAttributesHAL attrs;
    AdapterTest_->extMicCallback_ = new MockIDAudioCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->OpenCaptureDevice(devDesc, attrs));
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->CloseCaptureDevice(devDesc));
}

/**
 * @tc.name: SetAudioVolume_001
 * @tc.desc: Verify the SetAudioVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetAudioVolume_001, TestSize.Level1)
{
    std::string condition;
    std::string param;
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->SetAudioVolume(condition, param));
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
