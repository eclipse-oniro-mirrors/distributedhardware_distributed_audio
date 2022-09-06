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

#include "daudio_ipc_callback.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioIpcCallback"

namespace OHOS {
namespace DistributedHardware {
int32_t DAudioIpcCallback::OnNotifyRegResult(const std::string &devId, const std::string &dhId,
    const std::string &reqId, int32_t status, const std::string &resultData)
{
    DHLOGI("On notify the registration result, devId: %s, dhId: %s, status: %d, resultData: %s, reqId: %s",
        GetAnonyString(devId).c_str(), dhId.c_str(), status, resultData.c_str(), reqId.c_str());

    if (devId.length() > DAUDIO_MAX_DEVICE_ID_LEN || dhId.length() > DAUDIO_MAX_DEVICE_ID_LEN ||
        reqId.length() > DAUDIO_MAX_DEVICE_ID_LEN) {
        return ERR_DH_AUDIO_SA_DEVID_ILLEGAL;
    }
    auto iter = registerCallbackMap_.find(reqId);
    if (iter != registerCallbackMap_.end()) {
        iter->second->OnRegisterResult(devId, dhId, status, resultData);
        registerCallbackMap_.erase(reqId);
        return DH_SUCCESS;
    }

    return ERR_DH_AUDIO_SA_REGISTERCALLBACK_NOT_FOUND;
}

int32_t DAudioIpcCallback::OnNotifyUnregResult(const std::string &devId, const std::string &dhId,
    const std::string &reqId, int32_t status, const std::string &resultData)
{
    DHLOGI("On notify the unregistration result, devId: %s, dhId: %s, status: %d, resultData: %s, reqId: %s",
        GetAnonyString(devId).c_str(), dhId.c_str(), status, resultData.c_str(), reqId.c_str());

    if (devId.length() > DAUDIO_MAX_DEVICE_ID_LEN || dhId.length() > DAUDIO_MAX_DEVICE_ID_LEN ||
        reqId.length() > DAUDIO_MAX_DEVICE_ID_LEN) {
        return ERR_DH_AUDIO_SA_DEVID_ILLEGAL;
    }
    auto iter = unregisterCallbackMap_.find(reqId);
    if (iter != unregisterCallbackMap_.end()) {
        iter->second->OnUnregisterResult(devId, dhId, status, resultData);
        unregisterCallbackMap_.erase(reqId);
        return DH_SUCCESS;
    }
    return ERR_DH_AUDIO_SA_UNREGISTERCALLBACK_NOT_FOUND;
}

void DAudioIpcCallback::PushRegisterCallback(const std::string &reqId,
    const std::shared_ptr<RegisterCallback> &callback)
{
    DHLOGI("Push register callback, reqId: %s", reqId.c_str());
    registerCallbackMap_.emplace(reqId, callback);
}

void DAudioIpcCallback::PopRegisterCallback(const std::string &reqId)
{
    DHLOGI("Pop register callback, reqId: %s", reqId.c_str());
    registerCallbackMap_.erase(reqId);
}

void DAudioIpcCallback::PushUnregisterCallback(const std::string &reqId,
    const std::shared_ptr<UnregisterCallback> &callback)
{
    DHLOGI("Push unregister callback, reqId: %s", reqId.c_str());
    unregisterCallbackMap_.emplace(reqId, callback);
}

void DAudioIpcCallback::PopUnregisterCallback(const std::string &reqId)
{
    DHLOGI("Pop unregister callback, reqId: %s", reqId.c_str());
    unregisterCallbackMap_.erase(reqId);
}
} // DistributedHardware
} // OHOS