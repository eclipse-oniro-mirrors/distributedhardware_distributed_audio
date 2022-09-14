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

#include "audio_manager_interface_impl_test.h"

#include <hdf_base.h>
#include "hdf_device_object.h"
#include <sstream>

#include "daudio_constants.h"
#include "daudio_errcode.h"
#include "daudio_events.h"
#include "daudio_log.h"
#include "daudio_utils.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
void AudioManagerInterfaceImplTest::SetUpTestCase(void) {}

void AudioManagerInterfaceImplTest::TearDownTestCase(void) {}

void AudioManagerInterfaceImplTest::SetUp(void)
{
    audioManagerInterfaceImpl_ = std::make_shared<AudioManagerInterfaceImpl>();
}

void AudioManagerInterfaceImplTest::TearDown(void)
{
    audioManagerInterfaceImpl_ = nullptr;
}

/**
 * @tc.name: GetAllAdapters_001
 * @tc.desc: Verify the GetAllAdapters function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, GetAllAdapters_001, TestSize.Level1)
{
    std::vector<AudioAdapterDescriptorHAL> descriptors;
    EXPECT_EQ(HDF_SUCCESS, audioManagerInterfaceImpl_->GetAllAdapters(descriptors));
}

/**
 * @tc.name: LoadAdapter_001
 * @tc.desc: Verify the LoadAdapter and UnloadAdapter function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, LoadAdapter_001, TestSize.Level1)
{
    std::string adpName = "adpName";
    AudioAdapterDescriptorHAL descriptor;
    descriptor.adapterName = adpName;
    AudioPortHAL audioPortHal_ = {0, 0, "world"};
    descriptor.ports.push_back(audioPortHal_);
    sptr<IAudioAdapter> adapter = nullptr;
    EXPECT_EQ(HDF_FAILURE, audioManagerInterfaceImpl_->LoadAdapter(descriptor, adapter));
    EXPECT_EQ(HDF_SUCCESS, audioManagerInterfaceImpl_->UnloadAdapter(adpName));
}

/**
 * @tc.name: LoadAdapter_002
 * @tc.desc: Verify the LoadAdapter and UnloadAdapter function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, LoadAdapter_002, TestSize.Level1)
{
    std::string adpName = "adpName";
    AudioAdapterDescriptorHAL descriptor;
    descriptor.adapterName = adpName;
    AudioPortHAL audioPortHal_ = {0, 0, "world"};
    descriptor.ports.push_back(audioPortHal_);
    sptr<IAudioAdapter> adapter = nullptr;
    AudioAdapterDescriptorHAL desc;
    sptr<AudioAdapterInterfaceImpl> AudioAdapter = new AudioAdapterInterfaceImpl(desc);
    audioManagerInterfaceImpl_->mapAudioAdapter_.insert(std::make_pair(adpName, AudioAdapter));
    EXPECT_EQ(HDF_SUCCESS, audioManagerInterfaceImpl_->LoadAdapter(descriptor, adapter));
    EXPECT_EQ(HDF_SUCCESS, audioManagerInterfaceImpl_->UnloadAdapter(adpName));
}

/**
 * @tc.name: AddAudioDevice_001
 * @tc.desc: Verify the AddAudioDevice and RemoveAudioDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, AddAudioDevice_001, TestSize.Level1)
{
    std::string adpName;
    uint32_t devId = 0;
    std::string caps;
    sptr<IDAudioCallback> callback = nullptr;
    EXPECT_EQ(ERR_DH_AUDIO_HDF_FAIL, audioManagerInterfaceImpl_->AddAudioDevice(adpName, devId, caps, callback));
    EXPECT_EQ(ERR_DH_AUDIO_HDF_INVALID_OPERATION, audioManagerInterfaceImpl_->RemoveAudioDevice(adpName, devId));
}

/**
 * @tc.name: AddAudioDevice_002
 * @tc.desc: Verify the AddAudioDevice and RemoveAudioDevice function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, AddAudioDevice_002, TestSize.Level1)
{
    std::string adpName = "adpName";
    uint32_t devId = 1;
    std::string caps = "world";
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();
    AudioAdapterDescriptorHAL desc;
    sptr<AudioAdapterInterfaceImpl> AudioAdapter = new AudioAdapterInterfaceImpl(desc);
    audioManagerInterfaceImpl_->mapAudioAdapter_.insert(std::make_pair(adpName, AudioAdapter));
    EXPECT_EQ(ERR_DH_AUDIO_HDF_FAIL, audioManagerInterfaceImpl_->AddAudioDevice(adpName, devId, caps, callback));
    EXPECT_EQ(DH_SUCCESS, audioManagerInterfaceImpl_->RemoveAudioDevice(adpName, devId));
}

/**
 * @tc.name: Notify_001
 * @tc.desc: Verify the Notify function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, Notify_001, TestSize.Level1)
{
    std::string adpName;
    uint32_t devId = 0;
    AudioEvent event;
    EXPECT_EQ(ERR_DH_AUDIO_HDF_INVALID_OPERATION, audioManagerInterfaceImpl_->Notify(adpName, devId, event));
}

/**
 * @tc.name: Notify_002
 * @tc.desc: Verify the Notify function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, Notify_002, TestSize.Level1)
{
    std::string adpName = "adpName";
    uint32_t devId = 0;
    AudioEvent event;
    AudioAdapterDescriptorHAL desc;
    sptr<AudioAdapterInterfaceImpl> AudioAdapter = new AudioAdapterInterfaceImpl(desc);
    audioManagerInterfaceImpl_->mapAudioAdapter_.insert(std::make_pair(adpName, AudioAdapter));
    EXPECT_EQ(ERR_DH_AUDIO_HDF_FAIL, audioManagerInterfaceImpl_->Notify(adpName, devId, event));
}

/**
 * @tc.name: NotifyFwk_001
 * @tc.desc: Verify the NotifyFwk function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, NotifyFwk_001, TestSize.Level1)
{
    DAudioDevEvent event;
    EXPECT_EQ(ERR_DH_AUDIO_HDF_FAIL, audioManagerInterfaceImpl_->NotifyFwk(event));
}

/**
 * @tc.name: CreateAdapter_001
 * @tc.desc: Verify the CreateAdapter function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, CreateAdapter_001, TestSize.Level1)
{
    std::string adpName;
    uint32_t devId = 0;
    sptr<IDAudioCallback> callback = nullptr;
    EXPECT_EQ(ERR_DH_AUDIO_HDF_NULLPTR, audioManagerInterfaceImpl_->CreateAdapter(adpName, devId, callback));
}

/**
 * @tc.name: CreateAdapter_002
 * @tc.desc: Verify the CreateAdapter function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioManagerInterfaceImplTest, CreateAdapter_002, TestSize.Level1)
{
    std::string adpName = "adpName";
    uint32_t devId = static_cast<uint32_t>(PIN_OUT_DAUDIO_DEFAULT);
    sptr<IDAudioCallback> callback = new MockIDAudioCallback();
    EXPECT_EQ(DH_SUCCESS, audioManagerInterfaceImpl_->CreateAdapter(adpName, devId, callback));
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS