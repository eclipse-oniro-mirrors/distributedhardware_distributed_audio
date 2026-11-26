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

#include "sinkdevtask_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_event.h"
#include "cJSON.h"
#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_sink_dev.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr size_t MIN_FUZZ_DATA_LEN = sizeof(uint8_t);
constexpr uint8_t FUZZ_MODE_COUNT = 3;
constexpr uint8_t NOTIFY_RAW_EVENT_MODE = 0;
constexpr uint8_t NOTIFY_GUIDED_EVENT_MODE = 1;
constexpr uint8_t DEV_LIFECYCLE_MODE = 2;
constexpr uint8_t GUIDED_EVENT_COUNT = 3;
constexpr uint8_t GUIDED_EVENT_SPK_IDX = 0;
constexpr uint8_t GUIDED_EVENT_MIC_IDX = 1;
constexpr uint8_t GUIDED_EVENT_ENHANCE_IDX = 2;
constexpr uint8_t CHANNEL_STATE_COUNT = 3;
constexpr uint8_t CHANNEL_STATE_SPK_IDX = 0;
constexpr uint8_t CHANNEL_STATE_MIC_IDX = 1;
constexpr int32_t MIN_EVENT_TYPE_VALUE = 0;
constexpr int32_t MAX_EVENT_TYPE_VALUE = MAX_EVENT_TYPE_NUM;
constexpr int32_t DEFAULT_SPK_DH_ID = PIN_OUT_SPEAKER;
constexpr int32_t DEFAULT_MIC_DH_ID = PIN_IN_MIC;
constexpr int32_t EVENT_TYPE_OPEN_SPEAKER = OPEN_SPEAKER;
constexpr int32_t EVENT_TYPE_OPEN_MIC = OPEN_MIC;
constexpr int32_t EVENT_TYPE_ENHANCE_PARAM = ENHANCE_PARAM_CHANGE;
const std::string FUZZ_DEV_ID = "fuzzsinkdev001";
const std::string EMPTY_ARGS = "";
const std::string ENHANCE_PARAM_KEY = "audio_effect";
const std::string ENHANCE_PARAM_HD_VALUE = "high-definition-record";

std::shared_ptr<DAudioSinkDev> CreateAwakenedSinkDev()
{
    sptr<IDAudioSinkIpcCallback> nullCallback;
    std::shared_ptr<DAudioSinkDev> dev = std::make_shared<DAudioSinkDev>(FUZZ_DEV_ID, nullCallback);
    (void)dev->AwakeAudioDev();
    return dev;
}

std::shared_ptr<DAudioSinkDev> GetFuzzSinkDev()
{
    static std::shared_ptr<DAudioSinkDev> sinkDev = CreateAwakenedSinkDev();
    return sinkDev;
}
} // namespace

std::string BuildGuidedAudioParamArgs(FuzzedDataProvider &fdp, int32_t dhId)
{
    cJSON *jParam = cJSON_CreateObject();
    cJSON *audioParamJson = cJSON_CreateObject();
    if (jParam == nullptr || audioParamJson == nullptr) {
        cJSON_Delete(jParam);
        cJSON_Delete(audioParamJson);
        return EMPTY_ARGS;
    }
    cJSON_AddStringToObject(jParam, KEY_DH_ID, std::to_string(dhId).c_str());
    cJSON_AddNumberToObject(audioParamJson, KEY_SAMPLING_RATE, fdp.ConsumeIntegral<int32_t>());
    cJSON_AddNumberToObject(audioParamJson, KEY_CHANNELS, fdp.ConsumeIntegral<int32_t>());
    cJSON_AddNumberToObject(audioParamJson, KEY_FORMAT, fdp.ConsumeIntegral<int32_t>());
    cJSON_AddNumberToObject(audioParamJson, KEY_SOURCE_TYPE, fdp.ConsumeIntegral<int32_t>());
    cJSON_AddNumberToObject(audioParamJson, KEY_CONTENT_TYPE, fdp.ConsumeIntegral<int32_t>());
    cJSON_AddNumberToObject(audioParamJson, KEY_STREAM_USAGE, fdp.ConsumeIntegral<int32_t>());
    cJSON_AddItemToObject(jParam, KEY_AUDIO_PARAM, audioParamJson);
    char *jsonData = cJSON_PrintUnformatted(jParam);
    std::string args = (jsonData != nullptr) ? std::string(jsonData) : EMPTY_ARGS;
    cJSON_free(jsonData);
    cJSON_Delete(jParam);
    return args;
}

std::string BuildGuidedEnhanceParamArgs()
{
    cJSON *jParam = cJSON_CreateObject();
    if (jParam == nullptr) {
        return EMPTY_ARGS;
    }
    cJSON_AddStringToObject(jParam, ENHANCE_PARAM_KEY.c_str(), ENHANCE_PARAM_HD_VALUE.c_str());
    cJSON_AddStringToObject(jParam, KEY_DH_ID, std::to_string(DEFAULT_MIC_DH_ID).c_str());
    char *jsonData = cJSON_PrintUnformatted(jParam);
    std::string args = (jsonData != nullptr) ? std::string(jsonData) : EMPTY_ARGS;
    cJSON_free(jsonData);
    cJSON_Delete(jParam);
    return args;
}

void FuzzNotifyEventRaw(const std::shared_ptr<DAudioSinkDev> &sinkDev, FuzzedDataProvider &fdp)
{
    int32_t eventType = fdp.ConsumeIntegralInRange<int32_t>(MIN_EVENT_TYPE_VALUE, MAX_EVENT_TYPE_VALUE);
    std::string eventContent = fdp.ConsumeRemainingBytesAsString();
    sinkDev->NotifyEvent(AudioEvent(eventType, eventContent));
    sinkDev->SleepAudioDev();
}

void FuzzNotifyEventGuided(const std::shared_ptr<DAudioSinkDev> &sinkDev, FuzzedDataProvider &fdp)
{
    uint8_t guidedIndex = fdp.ConsumeIntegral<uint8_t>() % GUIDED_EVENT_COUNT;
    int32_t eventType = EVENT_TYPE_OPEN_SPEAKER;
    std::string eventContent = BuildGuidedAudioParamArgs(fdp, DEFAULT_SPK_DH_ID);
    if (guidedIndex == GUIDED_EVENT_MIC_IDX) {
        eventType = EVENT_TYPE_OPEN_MIC;
        eventContent = BuildGuidedAudioParamArgs(fdp, DEFAULT_MIC_DH_ID);
    } else if (guidedIndex == GUIDED_EVENT_ENHANCE_IDX) {
        eventType = EVENT_TYPE_ENHANCE_PARAM;
        eventContent = BuildGuidedEnhanceParamArgs();
    } else if (guidedIndex != GUIDED_EVENT_SPK_IDX) {
        return;
    }
    sinkDev->SetUserId(fdp.ConsumeIntegral<int32_t>());
    sinkDev->SetTokenId(fdp.ConsumeIntegral<int32_t>());
    sinkDev->SetAccountId(fdp.ConsumeRandomLengthString());
    sinkDev->SetDevLevelStatus(fdp.ConsumeBool());
    sinkDev->NotifyEvent(AudioEvent(eventType, eventContent));
    sinkDev->SleepAudioDev();
}

ChannelState MapFuzzChannelState(uint8_t channelIndex)
{
    switch (channelIndex) {
        case CHANNEL_STATE_SPK_IDX:
            return ChannelState::SPK_CONTROL_OPENED;
        case CHANNEL_STATE_MIC_IDX:
            return ChannelState::MIC_CONTROL_OPENED;
        default:
            return ChannelState::UNKNOWN;
    }
}

void FuzzDevLifecycle(const std::shared_ptr<DAudioSinkDev> &sinkDev, FuzzedDataProvider &fdp)
{
    uint8_t channelIndex = fdp.ConsumeIntegral<uint8_t>() % CHANNEL_STATE_COUNT;
    std::shared_ptr<IAVEngineProvider> provider = std::make_shared<IAVEngineProvider>();
    IAVEngineProvider *providerPtr = fdp.ConsumeBool() ? nullptr : provider.get();
    (void)sinkDev->InitAVTransEngines(MapFuzzChannelState(channelIndex), providerPtr);
    sinkDev->SetSinkTokenId(fdp.ConsumeIntegral<uint64_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();
    (void)sinkDev->PauseDistributedHardware(networkId);
    (void)sinkDev->ResumeDistributedHardware(networkId);
    (void)sinkDev->StopDistributedHardware(networkId);
    sinkDev->JudgeDeviceStatus();
    (void)sinkDev->CheckAclRight();
    (void)sinkDev->IsHardwareAccessGranted();
    (void)sinkDev->IsIdenticalAccount(networkId);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    uint32_t enableTokenId = fdp.ConsumeIntegral<uint32_t>();
    (void)sinkDev->ResolveEnableUser(userId, enableTokenId);
}

void SinkDevTaskFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < MIN_FUZZ_DATA_LEN) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DAudioSinkDev> sinkDev = GetFuzzSinkDev();
    uint8_t mode = fdp.ConsumeIntegral<uint8_t>() % FUZZ_MODE_COUNT;
    switch (mode) {
        case NOTIFY_RAW_EVENT_MODE:
            FuzzNotifyEventRaw(sinkDev, fdp);
            break;
        case NOTIFY_GUIDED_EVENT_MODE:
            FuzzNotifyEventGuided(sinkDev, fdp);
            break;
        case DEV_LIFECYCLE_MODE:
            FuzzDevLifecycle(sinkDev, fdp);
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
    OHOS::DistributedHardware::SinkDevTaskFuzzTest(data, size);
    return OHOS::DistributedHardware::FUZZER_EXIT_CODE;
}
