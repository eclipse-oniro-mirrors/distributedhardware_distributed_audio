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
#include "daudio_log.h"
#include "daudio_sink_manager.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
REGISTER_SYSTEM_ABILITY_BY_ID(DAudioSinkService, DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, true);

DAudioSinkService::DAudioSinkService(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    DHLOGI("%s: Distributed audio sink service constructed.", LOG_TAG);
}

void DAudioSinkService::OnStart()
{
    DHLOGI("%s: OnStart", LOG_TAG);
    if (!Init()) {
        DHLOGE("%s: Init failed.", LOG_TAG);
        return;
    }
    DHLOGI("%s: Start success.", LOG_TAG);
}

void DAudioSinkService::OnStop()
{
    DHLOGI("%s: OnStop", LOG_TAG);
    registerToService_ = false;
}

bool DAudioSinkService::Init()
{
    DHLOGI("%s: Start init", LOG_TAG);
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("%s: Publish service failed.", LOG_TAG);
            return false;
        }
        registerToService_ = true;
    }
    DHLOGI("%s: Init success.", LOG_TAG);
    return true;
}

int32_t DAudioSinkService::InitSink(const std::string &params)
{
    DHLOGI("%s: InitSink", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkService::ReleaseSink()
{
    DHLOGI("%s: ReleaseSink, exit process", LOG_TAG);
    exit(0);
    return DH_SUCCESS;
}

int32_t DAudioSinkService::SubscribeLocalHardware(const std::string &dhId, const std::string &param)
{
    DHLOGI("%s: SubscribeLocalHardware", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkService::UnsubscribeLocalHardware(const std::string &dhId)
{
    DHLOGI("%s: UnsubscribeLocalHardware", LOG_TAG);
    return DH_SUCCESS;
}

void DAudioSinkService::DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
    const std::string &eventContent)
{
    DHLOGI("%s: DAudioNotify devId:%s, dhId:%s, eventType:%d, eventContent:%s", LOG_TAG, GetAnonyString(devId).c_str(),
        dhId.c_str(), eventType, eventContent.c_str());
    DAudioSinkManager::GetInstance().HandleDAudioNotify(devId, dhId, eventType, eventContent);
}
} // namespace DistributedHardware
} // namespace OHOS