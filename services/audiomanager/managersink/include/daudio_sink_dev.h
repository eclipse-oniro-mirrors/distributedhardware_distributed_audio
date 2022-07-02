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

#ifndef OHOS_DAUDIO_SINK_DEV_H
#define OHOS_DAUDIO_SINK_DEV_H

#include <condition_variable>
#include <mutex>

#include "daudio_sink_dev_ctrl_manager.h"
#include "dmic_client.h"
#include "dspeaker_client.h"
#include "task_queue.h"
#include "nlohmann/json.hpp"
#include "iaudio_event_callback.h"

using json = nlohmann::json;

namespace OHOS {
namespace DistributedHardware {
class DAudioSinkDev : public IAudioEventCallback, public std::enable_shared_from_this<DAudioSinkDev> {
public:
    DAudioSinkDev(const std::string &networkId);
    ~DAudioSinkDev();

    void NotifyEvent(const std::shared_ptr<AudioEvent> &audioEvent) override;
    void NotifyEventSub(const std::shared_ptr<AudioEvent> &audioEvent);

    int32_t TaskOpenCtrlChannel(const std::string &args);
    int32_t TaskCloseCtrlChannel(const std::string &args);
    int32_t TaskOpenDSpeaker(const std::string &args);
    int32_t TaskCloseDSpeaker(const std::string &args);
    int32_t TaskOpenDMic(const std::string &args);
    int32_t TaskCloseDMic(const std::string &args);
    int32_t TaskSetParameter(const std::string &args);
    int32_t TaskVolumeChange(const std::string &args);
    int32_t TaskSetVolume(const std::string &args);
    void OnTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName);

private:
    int32_t NotifyOpenCtrlChannel(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyCloseCtrlChannel(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyCtrlOpened(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyCtrlClosed(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyOpenSpeaker(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyCloseSpeaker(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifySpeakerOpened(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifySpeakerClosed(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyOpenMic(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyCloseMic(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyMicOpened(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyMicClosed(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifySetVolume(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifyVolumeChange(const std::shared_ptr<AudioEvent> &audioEvent);
    int32_t NotifySetParam(const std::shared_ptr<AudioEvent> &audioEvent);
    bool JudgeJsonValid(const json &resultJson);

    static constexpr uint8_t RPC_WAIT_SECONDS = 5;
    static const constexpr char *LOG_TAG = "DAudioSinkDev";
    std::shared_ptr<DSpeakerClient> speakerClient_ = nullptr;
    std::shared_ptr<DMicClient> micClient_ = nullptr;
    std::shared_ptr<DAudioSinkDevCtrlMgr> dAudioSinkDevCtrlMgr_ = nullptr;
    std::string devId_;

    std::mutex rpcWaitMutex_;
    std::condition_variable rpcWaitCond_;

    std::mutex taskQueueMutex_;
    std::shared_ptr<TaskQueue> taskQueue_;
};
void from_json(const json &j, AudioParam &audioParam);
} // DistributedHardware
} // OHOS
#endif // OHOS_DAUDIO_SINK_DEV_H