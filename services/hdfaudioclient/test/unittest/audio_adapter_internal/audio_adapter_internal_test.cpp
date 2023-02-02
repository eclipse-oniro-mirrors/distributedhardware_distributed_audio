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

#include "daudio_adapter_internal.h"
#include "audio_adapter_internal_test.h"
#include "audio_adapter.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"

#include "audio_types.h"
#include <v1_0/iaudio_adapter.h>
#include <v1_0/iaudio_callback.h>
#include <v1_0/iaudio_capture.h>
#include <v1_0/iaudio_render.h>


#define HDF_LOG_TAG HDF_AUDIO_UT

using namespace std;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
class AudioAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AudioAdapterTest::SetUpTestCase()
{
}

void AudioAdapterTest::TearDownTestCase()
{
}

/**
* @tc.name: InitAllPortsInternal
* @tc.desc: Verify the abnormal branch of the InitAllPortsInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, InitAllPortsInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    int32_t ret = adapterContext.instance_.InitAllPorts(adapter);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: CreateRenderInternal
* @tc.desc: Verify the abnormal branch of the CreateRenderInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, CreateRenderInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    const struct ::AudioDeviceDescriptor *desc = nullptr;
    const struct ::AudioSampleAttributes *attrs = nullptr;
    struct AudioRender **render = nullptr;
    int32_t ret = adapterContext.instance_.CreateRender(adapter, desc, attrs, render);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: DestroyRenderInternal
* @tc.desc: Verify the abnormal branch of the DestroyRenderInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, DestroyRenderInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    int32_t ret = adapterContext.instance_.DestroyRender(adapter, render);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: CreateCaptureInternal
* @tc.desc: Verify the abnormal branch of the CreateCaptureInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, CreateCaptureInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    const struct ::AudioDeviceDescriptor *desc = nullptr;
    const struct ::AudioSampleAttributes *attrs = nullptr;
    struct AudioCapture **capture = nullptr;
    int32_t ret = adapterContext.instance_.CreateCapture(adapter, desc, attrs, capture);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: DestroyCaptureInternal
* @tc.desc: Verify the abnormal branch of the DestroyCaptureInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, DestroyCaptureInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    int32_t ret = adapterContext.instance_.DestroyCapture(adapter, capture);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: GetPassthroughModeInternal
* @tc.desc: Verify the abnormal branch of the GetPassthroughModeInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, GetPassthroughModeInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    const struct ::AudioPort *port = nullptr;
    enum ::AudioPortPassthroughMode *mode = nullptr;
    int32_t ret = adapterContext.instance_.GetPassthroughMode(adapter, port, mode);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: ReleaseAudioRouteInternal
* @tc.desc: Verify the abnormal branch of the ReleaseAudioRouteInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, ReleaseAudioRouteInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    int32_t routeHandle = 0;
    int32_t ret = adapterContext.instance_.ReleaseAudioRoute(adapter, routeHandle);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: SetPassthroughModeInternal
* @tc.desc: Verify the abnormal branch of the SetPassthroughModeInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, SetPassthroughModeInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    const struct ::AudioPort *port = nullptr;
    int32_t ret = adapterContext.instance_.SetPassthroughMode(adapter, port, PORT_PASSTHROUGH_LPCM);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: UpdateAudioRouteInternal
* @tc.desc: Verify the abnormal branch of the UpdateAudioRouteInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, UpdateAudioRouteInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext1;
    struct AudioAdapter *adapter = nullptr;
    const struct ::AudioRoute *route = nullptr;
    int32_t *routeHandle = nullptr;
    int32_t ret = adapterContext1.instance_.UpdateAudioRoute(adapter, route, routeHandle);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
    auto adapterContext = std::make_unique<AudioAdapterContext>();
    adapterContext->proxy_ = new MockIAudioAdapter();
    adapterContext->adapterName_ = "adapterName";
    struct ::AudioRoute *route1 = new ::AudioRoute;
    route1->sourcesNum = 0;
    route1->sinksNum = 0;
    int32_t a = 1;
    int32_t *routeHandle1 = &a;
    EXPECT_EQ(DH_SUCCESS, adapterContext->instance_.UpdateAudioRoute(&adapterContext->instance_, route1, routeHandle1));
    delete adapter;
    delete route;
}

/**
* @tc.name: SetExtraParamsInternal
* @tc.desc: Verify the abnormal branch of the SetExtraParamsInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, SetExtraParamsInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    const char *condition = nullptr;
    const char *value = nullptr;
    int32_t ret = adapterContext.instance_.SetExtraParams(adapter, AUDIO_EXT_PARAM_KEY_NONE, condition, value);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: GetExtraParamsInternal
* @tc.desc: Verify the abnormal branch of the GetExtraParamsInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, GetExtraParamsInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext;
    struct AudioAdapter *adapter = nullptr;
    const char *condition = nullptr;
    char *value = nullptr;
    int32_t length = 0;
    int32_t ret = adapterContext.instance_.GetExtraParams(adapter, AUDIO_EXT_PARAM_KEY_NONE, condition, value, length);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}

/**
* @tc.name: RegExtraParamObserverInternal
* @tc.desc: Verify the abnormal branch of the RegExtraParamObserverInternal, when param is null.
* @tc.type: FUNC
* @tc.require: AR000H0E6H
*/
HWTEST_F(AudioAdapterTest, RegExtraParamObserverInternal_001, TestSize.Level1)
{
    struct AudioAdapterContext adapterContext1;
    struct AudioAdapter *adapter = nullptr;
    ParamCallback callback = nullptr;
    void* cookie = nullptr;
    int32_t ret = adapterContext1.instance_.RegExtraParamObserver(adapter, callback, cookie);
    auto adapterContext = std::make_unique<AudioAdapterContext>();
    adapterContext->proxy_ = new MockIAudioAdapter();
    adapterContext->adapterName_ = "adapterName";
    ParamCallback callback1;
    cookie = &callback1;
    adapterContext->instance_.RegExtraParamObserver(&adapterContext->instance_,
        callback1, cookie);
    EXPECT_EQ(ERR_DH_AUDIO_HDI_INVALID_PARAM, ret);
}
} // DistributedHardware
} // OHOS