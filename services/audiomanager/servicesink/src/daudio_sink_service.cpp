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

#include "daudio_sink_service.h"

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
#include "daudio_sink_manager.h"
#include "daudio_util.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSinkService"

namespace OHOS {
namespace DistributedHardware {
REGISTER_SYSTEM_ABILITY_BY_ID(DAudioSinkService, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, true);

DAudioSinkService::DAudioSinkService(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    DHLOGI("Distributed audio sink service constructed.");
}

void DAudioSinkService::OnStart()
{
    DHLOGI("OnStart.");
    if (!Init()) {
        DHLOGE("Init failed.");
        return;
    }
    DHLOGI("Start success.");
}

void DAudioSinkService::OnStop()
{
    DHLOGI("OnStop.");
    isServiceStarted_ = false;
}

bool DAudioSinkService::Init()
{
    DHLOGI("Start init.");
    if (!isServiceStarted_) {
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("Publish service failed.");
            return false;
        }
        isServiceStarted_ = true;
    }
    DHLOGI("Init success.");
    return true;
}

int32_t DAudioSinkService::InitSink(const std::string &params)
{
    DHLOGI("Init distributed audio sink service.");
    DAudioSinkManager::GetInstance().Init();
    return DH_SUCCESS;
}

int32_t DAudioSinkService::ReleaseSink()
{
    DHLOGI("Release distributed audio sink service.");
    DAudioSinkManager::GetInstance().UnInit();
    return DH_SUCCESS;
}

int32_t DAudioSinkService::SubscribeLocalHardware(const std::string &dhId, const std::string &param)
{
    DHLOGI("Subscribe local hardware.");
    return DH_SUCCESS;
}

int32_t DAudioSinkService::UnsubscribeLocalHardware(const std::string &dhId)
{
    DHLOGI("Unsubscribe local hardware.");
    return DH_SUCCESS;
}

void DAudioSinkService::DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
    const std::string &eventContent)
{
    DHLOGI("DAudioNotify devId:%s, dhId:%s, eventType:%d.", GetAnonyString(devId).c_str(),
        dhId.c_str(), eventType);
    DAudioSinkManager::GetInstance().HandleDAudioNotify(devId, dhId, eventType, eventContent);
}
} // namespace DistributedHardware
} // namespace OHOS