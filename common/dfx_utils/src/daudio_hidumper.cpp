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

#include "daudio_hidumper.h"

#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DaudioHidumper"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DaudioHidumper);

namespace {
const std::string ARGS_HELP = "-h";
const std::string ARGS_SOURCE_DEVID = "--sourceDevId";
const std::string ARGS_SINK_INFO = "--sinkInfo";
const std::string ARGS_ABILITY = "--ability";

const std::map<std::string, HidumpFlag> ARGS_MAP = {
    { ARGS_HELP, HidumpFlag::GET_HELP },
    { ARGS_SOURCE_DEVID, HidumpFlag::GET_SOURCE_DEVID },
    { ARGS_SINK_INFO, HidumpFlag::GET_SINK_INFO },
    { ARGS_ABILITY, HidumpFlag::GET_ABILITY },
};
}

DaudioHidumper::DaudioHidumper()
{
    DHLOGI("DaudioHidumper constructed.");
}

DaudioHidumper::~DaudioHidumper()
{
    DHLOGI("DaudioHidumper destructed.");
}

bool DaudioHidumper::Dump(const std::vector<std::string> &args, std::string &result)
{
    DHLOGI("DaudioHidumper Dump args.size():%d.", args.size());
    result.clear();
    int32_t argsSize = static_cast<int32_t>(args.size());
    for (int32_t i = 0; i < argsSize; i++) {
        DHLOGI("DaudioHidumper Dump args[%d]: %s.", i, args.at(i).c_str());
    }

    if (args.empty()) {
        ShowHelp(result);
        return true;
    } else if (args.size() > 1) {
        ShowIllegalInfomation(result);
        return true;
    }

    if (ProcessDump(args[0], result) != DH_SUCCESS) {
        return false;
    }
    return true;
}

int32_t DaudioHidumper::ProcessDump(const std::string &args, std::string &result)
{
    DHLOGI("ProcessDump Dump.");
    HidumpFlag hf = HidumpFlag::UNKNOWN;
    auto operatorIter = ARGS_MAP.find(args);
    if (operatorIter != ARGS_MAP.end()) {
        hf = operatorIter->second;
    }

    if (hf == HidumpFlag::GET_HELP) {
        ShowHelp(result);
        return DH_SUCCESS;
    }
    result.clear();
    int32_t ret = ERR_DH_AUDIO_BAD_VALUE;
    switch (hf) {
        case HidumpFlag::GET_SOURCE_DEVID: {
            ret = GetSourceDevId(result);
            break;
        }
        case HidumpFlag::GET_SINK_INFO: {
            ret = GetSinkInfo(result);
            break;
        }
        case HidumpFlag::GET_ABILITY: {
            ret = GetAbilityInfo(result);
            break;
        }
        default: {
            ret = ShowIllegalInfomation(result);
            break;
        }
    }
    return ret;
}

int32_t DaudioHidumper::GetSourceDevId(std::string &result)
{
    DHLOGI("Get source devId dump.");
    int32_t ret = GetLocalDeviceNetworkId(g_sourceDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("Get local network id failed.");
        result.append("sourceDevId: ").append("");
        return ret;
    }
    result.append("sourceDevId: ").append(GetAnonyString(g_sourceDevId_));
    return DH_SUCCESS;
}

int32_t DaudioHidumper::GetSinkInfo(std::string &result)
{
    DHLOGI("Get sink info dump.");
    g_manager = GetAudioManagerFuncs();
    if (g_manager == nullptr) {
        return ERR_DH_AUDIO_NULLPTR;
    }
    int32_t ret = g_manager->GetAllAdapters(g_manager, &g_devices, &g_deviceNum);
    if (ret != DH_SUCCESS) {
        DHLOGE("Get all adapters failed.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    for (int32_t index = 0; index < g_deviceNum; index++) {
        AudioAdapterDescriptor &desc = g_devices[index];
        result.append("sinkDevId: ").append(GetAnonyString(desc.adapterName)).append("    portId: ");
        for (uint32_t i = 0; i < desc.portNum; i++) {
            result.append(std::to_string(desc.ports[i].portId)).append(" ");
        }
    }

    return DH_SUCCESS;
}

int32_t DaudioHidumper::GetAbilityInfo(std::string &result)
{
    DHLOGI("GetAbilityInfo Dump.");
    std::vector<DHItem> abilityInfo = DAudioHandler::GetInstance().ablityForDump();
    for (DHItem dhItem : abilityInfo) {
        if (dhItem.dhId == spkDefault) {
            result.append("spkAbilityInfo:").append(dhItem.attrs).append("      ");
        } else if (dhItem.dhId == micDefault) {
            result.append("micAbilityInfo:").append(dhItem.attrs).append("      ");
        } else {
            continue;
        }
    }
    return DH_SUCCESS;
}

void DaudioHidumper::ShowHelp(std::string &result)
{
    DHLOGI("ShowHelp Dump.");
    result.append("Usage:dump  <command> [options]\n")
        .append("Description:\n")
        .append("-h            ")
        .append(": show help\n")
        .append("--sourceDevId ")
        .append(": dump audio sourceDevId in the system\n")
        .append("--sinkInfo    ")
        .append(": dump sink info in the system\n")
        .append("--ability     ")
        .append(": dump current ability of the audio in the system\n");
}

int32_t DaudioHidumper::ShowIllegalInfomation(std::string &result)
{
    DHLOGI("ShowIllegalInfomation Dump.");
    result.append("unknown command, -h for help.");
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS