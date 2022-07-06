/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "audio_manager.h"
#include "daudio_errcode.h"
#include "daudio_log.h"

#define HDF_LOG_TAG HDF_AUDIO_UT

using namespace std;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
class AudioManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AudioManagerTest::SetUpTestCase()
{
}

void AudioManagerTest::TearDownTestCase()
{
}

/**
* @tc.name: GetAllAdapters_001
* @tc.desc: Verify the abnormal branch of the GetAllAdapters.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioManagerTest, GetAllAdapters_001, TestSize.Level1)
{
    AudioManager *manager = GetAudioManagerFuncs();
    int32_t size = 0;
    AudioAdapterDescriptor *descs = nullptr;

    int32_t ret = manager->GetAllAdapters(nullptr, &descs, &size);
    EXPECT_NE(DH_SUCCESS, ret);
    ret = manager->GetAllAdapters(manager, nullptr, &size);
    EXPECT_NE(DH_SUCCESS, ret);
    ret = manager->GetAllAdapters(manager, &descs, nullptr);
    EXPECT_NE(DH_SUCCESS, ret);
}

/**
* @tc.name: LoadAdapter_001
* @tc.desc: Verify the abnormal branch of the LoadAdapter.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioManagerTest, LoadAdapter_001, TestSize.Level1)
{
    AudioManager *manager = GetAudioManagerFuncs();
    AudioAdapterDescriptor desc = {};
    AudioAdapter *adapter = nullptr;

    int32_t ret = manager->LoadAdapter(nullptr, &desc, &adapter);
    EXPECT_NE(DH_SUCCESS, ret);
    ret = manager->LoadAdapter(manager, nullptr, &adapter);
    EXPECT_NE(DH_SUCCESS, ret);
    ret = manager->LoadAdapter(manager, &desc, nullptr);
    EXPECT_NE(DH_SUCCESS, ret);
}

/**
* @tc.name: LoadAdapter_002
* @tc.desc: Verify the abnormal branch of the LoadAdapter,when adapterName is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioManagerTest, LoadAdapter_002, TestSize.Level1)
{
    AudioManager *manager = GetAudioManagerFuncs();
    AudioAdapterDescriptor desc1 = {
        .adapterName = nullptr,
    };
    AudioAdapter *adapter = nullptr;

    int32_t ret = manager->LoadAdapter(manager, &desc1, &adapter);
    EXPECT_NE(DH_SUCCESS, ret);
}

/**
* @tc.name: GetAllAdapters_002
* @tc.desc: Verify the normal branch of the GetAllAdapters.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioManagerTest, GetAllAdapters_002, TestSize.Level1)
{
    AudioManager *manager = GetAudioManagerFuncs();
    ASSERT_NE(manager, nullptr);

    AudioAdapterDescriptor *descs = nullptr;
    int32_t size = 0;
    int32_t ret = manager->GetAllAdapters(manager, &descs, &size);
    EXPECT_EQ(ret, DH_SUCCESS);
    EXPECT_GT(size, 0);
    EXPECT_NE(descs, nullptr);
}

/**
* @tc.name: LoadAdapter_003
* @tc.desc: Verify the abnormal branch of the LoadAdapter,when param is Nullptr.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioManagerTest, LoadAdapter_003, TestSize.Level1)
{
    AudioManager *manager = GetAudioManagerFuncs();
    ASSERT_NE(manager, nullptr);

    AudioAdapterDescriptor *descs = nullptr;
    int32_t size = 0;
    int32_t ret = manager->GetAllAdapters(manager, &descs, &size);
    EXPECT_EQ(ret, DH_SUCCESS);
    EXPECT_GT(size, 0);
    ASSERT_NE(descs, nullptr);

    for (int32_t index = 0; index < size; index++) {
        // adapterName is invalid
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor desc1 = {
            .adapterName = "8$%^90sdad1!#",
            .portNum = descs[index].portNum,
            .ports = descs[index].ports,
        };
        ret = manager->LoadAdapter(manager, &desc1, &adapter);
        EXPECT_NE(DH_SUCCESS, ret);
    }
}

/**
* @tc.name: AudioManagerLoadAdapter
* @tc.desc: Verify the normal branch of the LoadAdapter and UnloadAdapter.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioManagerTest, AudioManagerLoadAdapter, TestSize.Level1)
{
    AudioManager *manager = GetAudioManagerFuncs();
    ASSERT_NE(manager, nullptr);

    AudioAdapterDescriptor *descs = nullptr;
    int32_t size = 0;
    int32_t ret = manager->GetAllAdapters(manager, &descs, &size);
    EXPECT_EQ(ret, DH_SUCCESS);
    EXPECT_GT(size, 0);
    ASSERT_NE(descs, nullptr);

    for (int32_t index = 0; index < size; index++) {
        AudioAdapter *adapter = nullptr;
        AudioAdapterDescriptor &desc = descs[index];
        if (strcmp(desc.adapterName, "daudio_primary_service") != 0) {
            continue;
        }
        ret = manager->LoadAdapter(manager, &desc, &adapter);
        ASSERT_EQ(ret, DH_SUCCESS);
        manager->UnloadAdapter(manager, adapter);
    }
}
} // DistributedHardware
} // OHOS