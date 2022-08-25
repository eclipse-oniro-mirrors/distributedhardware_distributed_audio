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

#include "daudio_hisysevent.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DAudioHisysevent);

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioHisysevent"

void DAudioHisysevent::SysEventWriteBehavior(const std::string &eventName, const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_AUDIO,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "MSG", msg);
    if (res != DH_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res: %d.", res);
    }
}

void DAudioHisysevent::SysEventWriteBehavior(const std::string &eventName, int32_t saId, const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_AUDIO,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "SAID", saId,
        "MSG", msg);
    if (res != DH_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res: %d.", res);
    }
}

void DAudioHisysevent::SysEventWriteBehavior(const std::string &eventName, const std::string &devId,
    const std::string &dhId, const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_AUDIO,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "DEVID", GetAnonyString(devId),
        "DHID", GetAnonyString(dhId),
        "MSG", msg);
    if (res != DH_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d.", res);
    }
}

void DAudioHisysevent::SysEventWriteFault(const std::string &eventName, const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_AUDIO,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "MSG", msg);
        if (res != DH_SUCCESS) {
            DHLOGE("Write HiSysEvent error, res:%d.", res);
        }
}

void DAudioHisysevent::SysEventWriteFault(const std::string &eventName, int32_t saId, int32_t errorCode,
    const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_AUDIO,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "SAID", saId,
        "ERRCODE", errorCode,
        "MSG", msg);
        if (res != DH_SUCCESS) {
            DHLOGE("Write HiSysEvent error, res:%d.", res);
        }
}

void DAudioHisysevent::SysEventWriteFault(const std::string &eventName, int32_t errorCode, const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_AUDIO,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "ERRCODE", errorCode,
        "MSG", msg);
    if (res != DH_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d.", res);
    }
}

void DAudioHisysevent::SysEventWriteFault(const std::string &eventName, const std::string &devId,
    const std::string &dhId, int32_t errorCode, const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_AUDIO,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "DEVID", GetAnonyString(devId),
        "DHID", GetAnonyString(dhId),
        "ERRCODE", errorCode, "MSG", msg);
    if (res != DH_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d.", res);
    }
}
} // namespace DistributedHardware
} // namespace OHOS