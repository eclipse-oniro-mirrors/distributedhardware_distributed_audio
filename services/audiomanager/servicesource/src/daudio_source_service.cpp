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

#include "daudio_source_service.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hisysevent.h"
#include "daudio_log.h"
#include "daudio_source_manager.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
REGISTER_SYSTEM_ABILITY_BY_ID(DAudioSourceService, DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID, true);
void DAudioSourceService::OnStart()
{
    DHLOGI("%s: Distributed audio service on start.", LOG_TAG);
    if (!Init()) {
        DHLOGE("%s: Init service failed.", LOG_TAG);
        return;
    }
    DHLOGI("%s: Start distributed audio service success.", LOG_TAG);
}

void DAudioSourceService::OnStop()
{
    DHLOGI("%s: Distributed audio service on stop.", LOG_TAG);
    isServiceStarted_ = false;
}

bool DAudioSourceService::Init()
{
    if (!isServiceStarted_) {
        DHLOGI("%s: Publish distributed audio service.", LOG_TAG);
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("%s: Publish service failed.", LOG_TAG);
            return false;
        }
        isServiceStarted_ = true;
    }
    return true;
}

int32_t DAudioSourceService::InitSource(const std::string &params, const sptr<IDAudioIpcCallback> &callback)
{
    DHLOGI("%s: Init source service.", LOG_TAG);
    (void)params;
    int32_t ret = DAudioSourceManager::GetInstance().Init(callback);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: DAudioSourceManager init failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceService::ReleaseSource()
{
    DHLOGI("%s: Release source service.", LOG_TAG);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_EXIT, "daudio source sa exit success.");
    DAudioSourceManager::GetInstance().UnInit();
    DHLOGI("%s: Audio source service process exit.", LOG_TAG);
    exit(0);
    return DH_SUCCESS;
}

int32_t DAudioSourceService::RegisterDistributedHardware(const std::string &devId, const std::string &dhId,
    const EnableParam &param, const std::string &reqId)
{
    DHLOGI("%s: Register distributed audio device, devId: %s, dhId: %s.", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str());
    std::string version = param.version;
    std::string attrs = param.attrs;
    return DAudioSourceManager::GetInstance().EnableDAudio(devId, dhId, version, attrs, reqId);
}

int32_t DAudioSourceService::UnregisterDistributedHardware(const std::string &devId, const std::string &dhId,
    const std::string &reqId)
{
    DHLOGI("%s: Unregister distributed audio device, devId: %s, dhId: %s.", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str());
    return DAudioSourceManager::GetInstance().DisableDAudio(devId, dhId, reqId);
}

int32_t DAudioSourceService::ConfigDistributedHardware(const std::string &devId, const std::string &dhId,
    const std::string &key, const std::string &value)
{
    DHLOGI("%s: Config distributed audio device, devId: %s, dhId: %s.", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str());
    return ERR_DH_AUDIO_SA_FUNCTION_NOT_IMPLEMENT;
}

void DAudioSourceService::DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
    const std::string &eventContent)
{
    DHLOGI("%s: Notify distributed audio device, devId: %s, dhId: %s.", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str());
    DAudioSourceManager::GetInstance().HandleDAudioNotify(devId, dhId, eventType, eventContent);
}

int DAudioSourceService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("%s: DAudioSourceService Dump.", LOG_TAG);
    std::string result;
    std::vector<std::string> argsStr;
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    if (!DaudioHidumper::GetInstance().Dump(argsStr, result)) {
        DHLOGE("%s: Hidump error", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        DHLOGE("%s: dprintf error", LOG_TAG);
        return ERR_DH_AUDIO_BAD_VALUE;
    }

    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS