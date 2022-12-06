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
#include "daudio_constants.h"

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
    AudioAdapterDescriptor adaDesc;
    AdapterTest_ = std::make_shared<AudioAdapterInterfaceImpl>(adaDesc);
}

void AudioAdapterInterfaceImpTest::TearDown(void)
{
    AdapterTest_ = nullptr;
}

/**
 * @tc.name: SetSpeakerCallback_001
 * @tc.desc: Verify the SetSpeakerCallback function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetSpeakerCallback_001, TestSize.Level1)
{
    sptr<IDAudioCallback> speakerCallback = nullptr;
    AdapterTest_->SetSpeakerCallback(speakerCallback);
}

/**
 * @tc.name: SetMicCallback_001
 * @tc.desc: Verify the SetMicCallback function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetMicCallback_001, TestSize.Level1)
{
    sptr<IDAudioCallback> micCallback = nullptr;
    AdapterTest_->SetMicCallback(micCallback);
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
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attrs;
    sptr<IAudioRender> render = nullptr;
    AdapterTest_->extSpeakerCallback_ = new MockIDAudioCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->CreateRender(devDesc, attrs, render));
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->DestroyRender(devDesc));
}

/**
 * @tc.name: CreateCapture_001
 * @tc.desc: Verify the CreateCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, CreateCapture_001, TestSize.Level1)
{
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attrs;
    sptr<IAudioCapture> capture = nullptr;
    AdapterTest_->extMicCallback_ = new MockIDAudioCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->CreateCapture(devDesc, attrs, capture));
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->DestroyCapture(devDesc));
}

/**
 * @tc.name: GetPortCapability_001
 * @tc.desc: Verify the GetPortCapability function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetPortCapability_001, TestSize.Level1)
{
    AudioPort port;
    AudioPortCapability capability;
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
    AudioPort port;
    AudioPortPassthroughMode mode = AudioPortPassthroughMode::PORT_PASSTHROUGH_LPCM;
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
    AudioPort port;
    AudioPortPassthroughMode mode = AudioPortPassthroughMode::PORT_PASSTHROUGH_LPCM;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetPassthroughMode(port, mode));
}

/**
 * @tc.name: SetMicMute_001
 * @tc.desc: Verify the SetMicMute function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetMicMute_001, TestSize.Level1)
{
    bool muteTmp = true;
    bool muteGetted;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->SetMicMute(muteTmp));
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetMicMute(muteGetted));
}

/**
 * @tc.name: SetVoiceVolume_001
 * @tc.desc: Verify the SetVoiceVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetVoiceVolume_001, TestSize.Level1)
{
    float vol = 1.0f;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->SetVoiceVolume(vol));
}

/**
 * @tc.name: UpdateAudioRoute_001
 * @tc.desc: Verify the UpdateAudioRoute function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, UpdateAudioRoute_001, TestSize.Level1)
{
    AudioRoute route;
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
 * @tc.name: SetExtraParams_001
 * @tc.desc: Verify the SetExtraParams function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, SetExtraParams_001, TestSize.Level1)
{
    AudioExtParamKey key = AudioExtParamKey::AUDIO_EXT_PARAM_KEY_NONE;
    std::string condition = "hello";
    std::string value = "world";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->SetExtraParams(key, condition, value));
}

/**
 * @tc.name: GetExtraParams_001
 * @tc.desc: Verify the GetExtraParams function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetExtraParams_001, TestSize.Level1)
{
    AudioExtParamKey key = AudioExtParamKey::AUDIO_EXT_PARAM_KEY_NONE;
    std::string condition = "hello";
    std::string value = "world";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetExtraParams(key, condition, value));
}

/**
 * @tc.name: GetExtraParams_002
 * @tc.desc: Verify the GetExtraParams function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetExtraParams_002, TestSize.Level1)
{
    AudioExtParamKey key = AudioExtParamKey::AUDIO_EXT_PARAM_KEY_VOLUME;
    std::string condition = "hello";
    std::string value = "1";
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->GetExtraParams(key, condition, value));
}

/**
 * @tc.name: GetExtraParams_003
 * @tc.desc: Verify the GetExtraParams function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetExtraParams_003, TestSize.Level1)
{
    AudioExtParamKey key = AudioExtParamKey::AUDIO_EXT_PARAM_KEY_VOLUME;
    std::string condition = "EVENT_TYPE=1;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    std::string value = "1";
    EXPECT_EQ(HDF_FAILURE, AdapterTest_->GetExtraParams(key, condition, value));
}

/**
 * @tc.name: GetExtraParams_004
 * @tc.desc: Verify the GetExtraParams function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetExtraParams_004, TestSize.Level1)
{
    AudioExtParamKey key = AudioExtParamKey::AUDIO_EXT_PARAM_KEY_STATUS;
    std::string condition = "hello";
    std::string value = "world";
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, AdapterTest_->GetExtraParams(key, condition, value));
}

/**
 * @tc.name: RegExtraParamObserver_001
 * @tc.desc: Verify the RegExtraParamObserver function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, RegExtraParamObserver_001, TestSize.Level1)
{
    sptr<IAudioCallback> cbObj = nullptr;
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->RegExtraParamObserver(cbObj, 0));
}

/**
 * @tc.name: RegExtraParamObserver_002
 * @tc.desc: Verify the RegExtraParamObserver function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, RegExtraParamObserver_002, TestSize.Level1)
{
    AdapterTest_->paramCallback_ = new MockIAudioParamCallback();
    sptr<IAudioCallback> cbObj = new MockIAudioParamCallback();
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->RegExtraParamObserver(cbObj, 0));
}

/**
 * @tc.name: GetDeviceCapabilitys_001
 * @tc.desc: Verify the GetDeviceCapabilitys function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetDeviceCapabilitys_001, TestSize.Level1)
{
    uint32_t devId = 88;
    std::string caps = "worldcup";
    AdapterTest_->AddAudioDevice(devId, caps);

    EXPECT_EQ(caps, AdapterTest_->GetDeviceCapabilitys(devId));
}

/**
 * @tc.name: GetDeviceCapabilitys_002
 * @tc.desc: Verify the GetDeviceCapabilitys function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetDeviceCapabilitys_002, TestSize.Level1)
{
    uint32_t devId = 88;
    std::string caps = "worldcup";
    AdapterTest_->RemoveAudioDevice(devId);

    EXPECT_EQ("", AdapterTest_->GetDeviceCapabilitys(devId));
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
    DAudioEvent event;
    event.type = 3;
    event.content = "VOLUME_LEVEL";
    uint32_t devId = 64;
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->Notify(devId, event));
}

/**
 * @tc.name: Notify_002
 * @tc.desc: Verify the Notify function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, Notify_002, TestSize.Level1)
{
    DAudioEvent event;
    event.type = 10;
    event.content = "FOCUS_CHANGE";
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
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->AddAudioDevice(devId, caps));
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
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attrs;
    AdapterTest_->extSpeakerCallback_ = new MockIDAudioCallback;
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->OpenRenderDevice(devDesc, attrs));
}

/**
 * @tc.name: OpenCaptureDevice_001
 * @tc.desc: Verify the OpenCaptureDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, OpenCaptureDevice_001, TestSize.Level1)
{
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attrs;
    AdapterTest_->extMicCallback_ = new MockIDAudioCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->OpenCaptureDevice(devDesc, attrs));
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

/**
 * @tc.name: GetAudioVolume_001
 * @tc.desc: Verify the GetAudioVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetAudioVolume_001, TestSize.Level1)
{
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);

    std::string condition = "EVENT_TYPE=1;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    std::string param = "1";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetAudioVolume(condition, param));
}

/**
 * @tc.name: GetAudioVolume_002
 * @tc.desc: Verify the GetAudioVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetAudioVolume_002, TestSize.Level1)
{
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);

    std::string condition = "EVENT_TYPE=3;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    std::string param = "1";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetAudioVolume(condition, param));
}

/**
 * @tc.name: GetAudioVolume_003
 * @tc.desc: Verify the GetAudioVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetAudioVolume_003, TestSize.Level1)
{
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);

    std::string condition = "EVENT_TYPE=2;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    std::string param = "1";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetAudioVolume(condition, param));
}

/**
 * @tc.name: GetAudioVolume_004
 * @tc.desc: Verify the GetAudioVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetAudioVolume_004, TestSize.Level1)
{
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);

    std::string condition = "EVENT_TYPE=4;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    std::string param = "1";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetAudioVolume(condition, param));
}


/**
 * @tc.name: GetAudioVolume_005
 * @tc.desc: Verify the GetAudioVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetAudioVolume_005, TestSize.Level1)
{
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);

    std::string condition = "EVENT_TYPE=66;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    std::string param = "1";
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->GetAudioVolume(condition, param));
    EXPECT_EQ("0", param);
}

/**
 * @tc.name: GetAudioVolume_006
 * @tc.desc: Verify the GetAudioVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, GetAudioVolume_006, TestSize.Level1)
{
    AdapterTest_->audioRender_ = nullptr;

    std::string condition = "EVENT_TYPE=1;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    std::string param = "1";
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->GetAudioVolume(condition, param));
    EXPECT_EQ("1", param);
}

/**
 * @tc.name: getEventTypeFromCondition_001
 * @tc.desc: Verify the getEventTypeFromCondition function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, getEventTypeFromCondition_001, TestSize.Level1)
{
    std::string condition = "EVENT_TYPE=1;VOLUME_GROUP_ID=2;AUDIO_VOLUME_TYPE=1;";
    auto actualValue = AdapterTest_->getEventTypeFromCondition(condition);
    EXPECT_EQ(1, actualValue);
}

/**
 * @tc.name: HandleVolumeChangeEvent_001
 * @tc.desc: Verify the HandleVolumeChangeEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleVolumeChangeEvent_001, TestSize.Level1)
{
    DAudioEvent event = {DistributedHardware::HDF_AUDIO_EVENT_VOLUME_CHANGE,
        "VOLUME_CHANAGE;AUDIO_STREAM_TYPE=1;VOLUME_LEVEL=1;IS_UPDATEUI=1;VOLUME_GROUP_ID=1;"};
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = nullptr;
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleVolumeChangeEvent(event));

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleVolumeChangeEvent(event));
    AdapterTest_->paramCallback_ = new MockIAudioParamCallback();
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleVolumeChangeEvent(event));
}

/**
 * @tc.name: HandleVolumeChangeEvent_002
 * @tc.desc: Verify the HandleVolumeChangeEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleVolumeChangeEvent_002, TestSize.Level1)
{
    DAudioEvent event = {DistributedHardware::HDF_AUDIO_EVENT_VOLUME_CHANGE,
        "V"};
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleVolumeChangeEvent(event));
}

/**
 * @tc.name: HandleVolumeChangeEvent_003
 * @tc.desc: Verify the HandleVolumeChangeEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleVolumeChangeEvent_003, TestSize.Level1)
{
    DAudioEvent event = {DistributedHardware::HDF_AUDIO_EVENT_VOLUME_CHANGE,
        "V"};
    std::string adpterName = "adbcef";
    AudioDeviceDescriptor desc;
    AudioSampleAttributes attrs;
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();

    AdapterTest_->audioRender_ = new AudioRenderInterfaceImpl(adpterName, desc, attrs, callback);
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleVolumeChangeEvent(event));
}

/**
 * @tc.name: HandleFocusChangeEvent_001
 * @tc.desc: Verify the HandleFocusChangeEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleFocusChangeEvent_001, TestSize.Level1)
{
    DAudioEvent event = {DistributedHardware::HDF_AUDIO_EVENT_FOCUS_CHANGE,
        "INTERRUPT_EVENT;EVENT_TYPE=1;VOLUME_LEVEL=1;FORCE_TYPE=1;HINT_TYPE=1;"};

    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleFocusChangeEvent(event));
    AdapterTest_->paramCallback_ = new MockIAudioParamCallback();
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleFocusChangeEvent(event));
}

/**
 * @tc.name: HandleFocusChangeEvent_002
 * @tc.desc: Verify the HandleFocusChangeEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleFocusChangeEvent_002, TestSize.Level1)
{
    DAudioEvent event = {DistributedHardware::HDF_AUDIO_EVENT_FOCUS_CHANGE,
        "INTERRUPT_EVENT;EVENT_TYPE=1;VOLUME_LEVEL=1;FORCE_TYPE=1;HINT_TYPE=1;"};

    AdapterTest_->paramCallback_ = new MockRevertIAudioParamCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleFocusChangeEvent(event));
}

/**
 * @tc.name: HandleRenderStateChangeEvent_001
 * @tc.desc: Verify the HandleRenderStateChangeEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleRenderStateChangeEvent_001, TestSize.Level1)
{
    DAudioEvent event = {DistributedHardware::HDF_AUDIO_EVENT_RENDER_STATE_CHANGE,
        "RENDER_STATE_CHANGE_EVENT;STATE=0;"};

    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleRenderStateChangeEvent(event));
    AdapterTest_->paramCallback_ = new MockIAudioParamCallback();
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleRenderStateChangeEvent(event));
}

/**
 * @tc.name: HandleRenderStateChangeEvent_002
 * @tc.desc: Verify the HandleRenderStateChangeEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleRenderStateChangeEvent_002, TestSize.Level1)
{
    DAudioEvent event = {DistributedHardware::HDF_AUDIO_EVENT_RENDER_STATE_CHANGE,
        "RENDER_STATE_CHANGE_EVENT;STATE=0;"};

    AdapterTest_->paramCallback_ = new MockRevertIAudioParamCallback();
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleRenderStateChangeEvent(event));
}

/**
 * @tc.name: HandleSANotifyEvent_001
 * @tc.desc: Verify the HandleSANotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleSANotifyEvent_001, TestSize.Level1)
{
    using namespace DistributedHardware;
    DAudioEvent event = {HDF_AUDIO_EVENT_OPEN_SPK_RESULT, "RENDER_STATE_CHANGE_EVENT"};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event));
}

/**
 * @tc.name: HandleSANotifyEvent_002
 * @tc.desc: Verify the HandleSANotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleSANotifyEvent_002, TestSize.Level1)
{
    using namespace DistributedHardware;
    DAudioEvent event = {HDF_AUDIO_EVENT_OPEN_SPK_RESULT, HDF_EVENT_RESULT_SUCCESS};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event));
}

/**
 * @tc.name: HandleSANotifyEvent_003
 * @tc.desc: Verify the HandleSANotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleSANotifyEvent_003, TestSize.Level1)
{
    using namespace DistributedHardware;
    DAudioEvent event = {HDF_AUDIO_EVENT_CLOSE_SPK_RESULT,
        HDF_EVENT_RESULT_SUCCESS};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event));
    DAudioEvent event1 = {HDF_AUDIO_EVENT_CLOSE_SPK_RESULT, "RENDER_STATE_CHANGE"};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event1));
}

/**
 * @tc.name: HandleSANotifyEvent_004
 * @tc.desc: Verify the HandleSANotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleSANotifyEvent_004, TestSize.Level1)
{
    using namespace DistributedHardware;
    DAudioEvent event = {HDF_AUDIO_EVENT_OPEN_MIC_RESULT,
        HDF_EVENT_RESULT_SUCCESS};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event));
    DAudioEvent event1 = {HDF_AUDIO_EVENT_OPEN_MIC_RESULT, "RENDER_STATE_CHANGE"};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event1));
}

/**
 * @tc.name: HandleSANotifyEvent_005
 * @tc.desc: Verify the HandleSANotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleSANotifyEvent_005, TestSize.Level1)
{
    using namespace DistributedHardware;
    DAudioEvent event = {HDF_AUDIO_EVENT_CLOSE_MIC_RESULT, HDF_EVENT_RESULT_SUCCESS};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event));
    DAudioEvent event1 = {HDF_AUDIO_EVENT_CLOSE_MIC_RESULT, "RENDER_STATE_CHANGE"};
    EXPECT_EQ(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event1));
}

/**
 * @tc.name: HandleSANotifyEvent_006
 * @tc.desc: Verify the HandleSANotifyEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioAdapterInterfaceImpTest, HandleSANotifyEvent_006, TestSize.Level1)
{
    using namespace DistributedHardware;
    DAudioEvent event = {-1, "ddd"};
    EXPECT_NE(HDF_SUCCESS, AdapterTest_->HandleSANotifyEvent(event));
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
