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

#include "daudio_sink_load_callback.h"

#include "daudio_log.h"
#include "daudio_sink_handler.h"

namespace OHOS {
namespace DistributedHardware {
DAudioSinkLoadCallback::DAudioSinkLoadCallback(const std::string &params) : params_(params) {}

void DAudioSinkLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    DHLOGI("%s: Load audio SA success, systemAbilityId: %d, remoteObject result: %s.",
        LOG_TAG, systemAbilityId, (remoteObject != nullptr) ? "true" : "false");
    if (remoteObject == nullptr) {
        DHLOGE("%s: RemoteObject is nullptr.", LOG_TAG);
        return;
    }

    DAudioSinkHandler::GetInstance().FinishStartSA(params_, remoteObject);
}

void DAudioSinkLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    DHLOGE("%s: Load audio SA failed, systemAbilityId: %d.", LOG_TAG, systemAbilityId);
}
} // namespace DistributedHardware
} // namespace OHOS