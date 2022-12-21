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

#ifndef DAUDIO_SOURCE_SERVICE_TEST_H
#define DAUDIO_SOURCE_SERVICE_TEST_H

#include <gtest/gtest.h>

#include "daudio_errorcode.h"
#include "idaudio_ipc_callback.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"

#define private public
#define protected public
#include "daudio_source_service.h"
#undef protected
#undef private

namespace OHOS {
namespace DistributedHardware {
class DAudioSourceServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DAudioSourceService> sourceSrv_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DAUDIO_SINK_DEV_TEST_H