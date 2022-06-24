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

#ifndef OHOS_DAUDIO_SOURCE_DEV_H
#define OHOS_DAUDIO_SOURCE_DEV_H

#include <mutex>
#include "nlohmann/json.hpp"

#include "audio_event.h"
#include "iaudio_event_callback.h"
#include "iaudio_datatrans_callback.h"
#include "iaudio_data_transport.h"
#include "idaudio_ipc_callback.h"
#include "idaudio_hdi_callback.h"
#include "daudio_source_dev_ctrl_manager.h"
#include "daudio_source_mgr_callback.h"
#include "dmic_dev.h"
#include "dspeaker_dev.h"
#include "task_queue.h"

using json = nlohmann::json;
namespace OHOS {
namespace DistributedHardware {
class DAudioSourceDev : public IAudioEventCallback, public std::enable_shared_from_this<DAudioSourceDev> {
public:
    DAudioSourceDev(const std::string &devId, const std::shared_ptr<DAudioSourceMgrCallback> &callback)
        : devId_(devId), mgrCallback_(callback) {};
    ~DAudioSourceDev() = default;

    int32_t AwakeAudioDev();
    void SleepAudioDev();

    int32_t EnableDAudio(const std::string &dhId, const std::string &attrs);
    int32_t DisableDAudio(const std::string &dhId);
    void NotifyEvent(const std::shared_ptr<AudioEvent> &event) override;

private:
    int32_t EnableDSpeaker(const int32_t dhId, const std::string &attrs);
    int32_t EnableDMic(const int32_t dhId, const std::string &attrs);
    int32_t DisableDSpeaker(const int32_t dhId);
    int32_t DisableDMic(const int32_t dhId);

    int32_t TaskEnableDAudio(const std::string &args);
    int32_t TaskDisableDAudio(const std::string &args);
    int32_t TaskOpenCtrlChannel(const std::string &args);
    int32_t TaskCloseCtrlChannel(const std::string &args);
    int32_t TaskOpenDSpeaker(const std::string &args);
    int32_t TaskCloseDSpeaker(const std::string &args);
    int32_t TaskOpenDMic(const std::string &args);
    int32_t TaskCloseDMic(const std::string &args);

    void OnDisableTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName);
    void OnEnableTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName);
    void OnTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName);

    int32_t HandleOpenDSpeaker(const std::shared_ptr<AudioEvent> &event);
    int32_t HandleCloseDSpeaker(const std::shared_ptr<AudioEvent> &event);
    int32_t HandleOpenDMic(const std::shared_ptr<AudioEvent> &event);
    int32_t HandleCloseDMic(const std::shared_ptr<AudioEvent> &event);
    int32_t HandleOpenCtrlTrans(const std::shared_ptr<AudioEvent> &event);
    int32_t HandleCloseCtrlTrans();
    int32_t HandleNotifyRPC(const std::shared_ptr<AudioEvent> &event);
    int32_t WaitForRPC(const AudioEventType type);

private:
    static const constexpr char *LOG_TAG = "DAudioSourceDev";
    static constexpr uint8_t RPC_WAIT_SECONDS = 2;
    static constexpr uint8_t TASK_QUEUE_CAPACITY = 20;

    std::string devId_;
    std::string localDevId_;
    std::shared_ptr<DAudioSourceMgrCallback> mgrCallback_;
    std::shared_ptr<TaskQueue> taskQueue_;
    std::shared_ptr<DSpeakerDev> speaker_ = nullptr;
    std::shared_ptr<DMicDev> mic_ = nullptr;

    std::shared_ptr<DAudioSourceDevCtrlMgr> audioSourceCtrlMgr_ = nullptr;
    std::mutex taskQueueMutex_;

    std::mutex rpcWaitMutex_;
    std::condition_variable rpcWaitCond_;
    bool rpcResult_ = false;
    uint8_t rpcNotify_ = 0;
};
void to_json(json &j, const AudioParam &audioParam);
void from_json(const json &j, AudioParam &audioParam);
} // DistributedHardware
} // OHOS
#endif