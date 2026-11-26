/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "sinkmanagerdev_fuzzer.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

#include <fuzzer/FuzzedDataProvider.h>

#include "av_trans_types.h"
#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_sink_manager.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr size_t MIN_FUZZ_DATA_LEN = sizeof(uint8_t);
constexpr uint8_t FUZZ_MODE_COUNT = 5;
constexpr uint8_t HANDLE_NOTIFY_MODE = 0;
constexpr uint8_t CHANNEL_DEVICE_MODE = 1;
constexpr uint8_t TRANS_CONTROL_MODE = 2;
constexpr uint8_t PROVIDER_LISTENER_MODE = 3;
constexpr uint8_t DEVICE_QUERY_MODE = 4;
constexpr uint8_t AV_EVENT_TYPE_COUNT = 3;
constexpr uint8_t AV_EVENT_CHANNEL_OPENED_IDX = 0;
constexpr uint8_t AV_EVENT_CHANNEL_CLOSED_IDX = 1;
constexpr int32_t MIN_EVENT_TYPE_VALUE = 0;
constexpr int32_t MAX_EVENT_TYPE_VALUE = MAX_EVENT_TYPE_NUM;
const std::string FUZZ_SINK_DEV_ID = "fuzzsinkdev001";
} // namespace

void FuzzManagerHandleNotify(FuzzedDataProvider &fdp)
{
    auto &manager = DAudioSinkManager::GetInstance();
    /* With unknown channel state the dev stays in the map after creation. */
    (void)manager.CreateAudioDevice(FUZZ_SINK_DEV_ID);
    bool useKnownDev = fdp.ConsumeBool();
    std::string devId = useKnownDev ? FUZZ_SINK_DEV_ID : fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    int32_t eventType = fdp.ConsumeIntegralInRange<int32_t>(MIN_EVENT_TYPE_VALUE, MAX_EVENT_TYPE_VALUE);
    std::string eventContent = fdp.ConsumeRemainingBytesAsString();
    (void)manager.HandleDAudioNotify(devId, dhId, eventType, eventContent);
}

void FuzzManagerChannelDevice(FuzzedDataProvider &fdp)
{
    auto &manager = DAudioSinkManager::GetInstance();
    std::string channelContent = fdp.ConsumeRandomLengthString();
    if (fdp.ConsumeBool()) {
        channelContent = OWNER_NAME_D_SPEAKER;
    }
    manager.SetChannelState(channelContent);
    std::string devId = fdp.ConsumeRandomLengthString();
    (void)manager.CreateAudioDevice(devId);
    (void)manager.ClearAudioDev(devId);
    manager.OnSinkDevReleased(FUZZ_SINK_DEV_ID);
}

void FuzzManagerTransControl(FuzzedDataProvider &fdp)
{
    auto &manager = DAudioSinkManager::GetInstance();
    (void)manager.CreateAudioDevice(FUZZ_SINK_DEV_ID);
    std::string networkId = fdp.ConsumeBool() ? FUZZ_SINK_DEV_ID : fdp.ConsumeRandomLengthString();
    (void)manager.PauseDistributedHardware(networkId);
    (void)manager.ResumeDistributedHardware(networkId);
    (void)manager.StopDistributedHardware(networkId);
    int32_t eventType = fdp.ConsumeIntegralInRange<int32_t>(MIN_EVENT_TYPE_VALUE, MAX_EVENT_TYPE_VALUE);
    manager.NotifyEvent(networkId, eventType, fdp.ConsumeRandomLengthString());
}

EventType MapFuzzAvEventType(uint8_t eventIndex)
{
    switch (eventIndex) {
        case AV_EVENT_CHANNEL_OPENED_IDX:
            return EventType::EVENT_CHANNEL_OPENED;
        case AV_EVENT_CHANNEL_CLOSED_IDX:
            return EventType::EVENT_CHANNEL_CLOSED;
        default:
            return EventType::EVENT_ENGINE_ERROR;
    }
}

void FuzzManagerProviderListener(FuzzedDataProvider &fdp)
{
    EngineProviderListener providerListener;
    CtrlChannelListener ctrlListener;
    DeviceInitCallback deviceInitCallback;
    DmInitCallback &dmInitCallback = deviceInitCallback;
    AVTransEvent event;
    event.type = MapFuzzAvEventType(fdp.ConsumeIntegral<uint8_t>() % AV_EVENT_TYPE_COUNT);
    event.content = fdp.ConsumeRandomLengthString();
    if (fdp.ConsumeBool()) {
        event.content = OWNER_NAME_D_SPEAKER;
    }
    event.peerDevId = fdp.ConsumeBool() ? FUZZ_SINK_DEV_ID : fdp.ConsumeRandomLengthString();
    (void)providerListener.OnProviderEvent(event);
    ctrlListener.OnCtrlChannelEvent(event);
    dmInitCallback.OnRemoteDied();
}

void FuzzManagerDeviceQuery(FuzzedDataProvider &fdp)
{
    auto &manager = DAudioSinkManager::GetInstance();
    bool isInvalid = fdp.ConsumeBool();
    std::string networkId = fdp.ConsumeBool() ? FUZZ_SINK_DEV_ID : fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    int32_t eventType = fdp.ConsumeIntegralInRange<int32_t>(MIN_EVENT_TYPE_VALUE, MAX_EVENT_TYPE_VALUE);
    std::string eventContent = fdp.ConsumeRandomLengthString();
    (void)manager.DAudioNotify(networkId, dhId, eventType, eventContent);
    (void)manager.CheckOsType(networkId, isInvalid);
    (void)manager.GetUdidByNetworkId(networkId);
    (void)manager.GetDeviceSecurityLevel(fdp.ConsumeRandomLengthString());
    manager.SetCallerTokenId(fdp.ConsumeIntegral<uint64_t>());
    manager.SetEnableFirstTokenId(fdp.ConsumeIntegral<uint32_t>());
    (void)manager.GetEnableFirstTokenId();
}

void SinkManagerDevFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < MIN_FUZZ_DATA_LEN) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    uint8_t mode = fdp.ConsumeIntegral<uint8_t>() % FUZZ_MODE_COUNT;
    switch (mode) {
        case HANDLE_NOTIFY_MODE:
            FuzzManagerHandleNotify(fdp);
            break;
        case CHANNEL_DEVICE_MODE:
            FuzzManagerChannelDevice(fdp);
            break;
        case TRANS_CONTROL_MODE:
            FuzzManagerTransControl(fdp);
            break;
        case PROVIDER_LISTENER_MODE:
            FuzzManagerProviderListener(fdp);
            break;
        case DEVICE_QUERY_MODE:
            FuzzManagerDeviceQuery(fdp);
            break;
        default:
            break;
    }
}
} // namespace DistributedHardware
} // namespace OHOS

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t FUZZER_EXIT_CODE = 0;
} // namespace
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SinkManagerDevFuzzTest(data, size);
    return OHOS::DistributedHardware::FUZZER_EXIT_CODE;
}
