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

#ifndef OHOS_AUDIO_IPC_COMMON_H
#define OHOS_AUDIO_IPC_COMMON_H

#include <string>

#include "daudio_errorcode.h"
#include "daudio_ipc_callback.h"
#include "daudio_ipc_callback_stub.h"
#include "daudio_source_handler.h"
#include "daudio_source_proxy.h"
#include "idaudio_ipc_callback.h"

namespace OHOS {
namespace DistributedHardware {
class RegisterCallbackTest : public RegisterCallback {
public:
    RegisterCallbackTest() = default;
    virtual ~RegisterCallbackTest() = default;

    int32_t OnRegisterResult(const std::string &uuid, const std::string &dhId, int32_t status,
        const std::string &data)
    {
        return DH_SUCCESS;
    }
};

class UnregisterCallbackTest : public UnregisterCallback {
public:
    UnregisterCallbackTest() = default;
    virtual ~UnregisterCallbackTest() = default;

    int32_t OnUnregisterResult(const std::string &uuid, const std::string &dhId, int32_t status,
        const std::string &data)
    {
        return DH_SUCCESS;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AUDIO_IPC_COMMON_H