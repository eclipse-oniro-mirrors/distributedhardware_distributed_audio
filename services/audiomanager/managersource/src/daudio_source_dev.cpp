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

#include "daudio_source_dev.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "daudio_source_manager.h"
#include "daudio_util.h"
#include "task_impl.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DAudioSourceDev::AwakeAudioDev()
{
    taskQueue_ = std::make_shared<TaskQueue>(TASK_QUEUE_CAPACITY);
    if (taskQueue_ == nullptr) {
        DHLOGE("%s: Create task queue failed.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    taskQueue_->Start();

    int32_t ret = GetLocalDeviceNetworkId(localDevId_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get local network id failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

void DAudioSourceDev::SleepAudioDev()
{
    taskQueue_->Stop();
    taskQueue_ = nullptr;
}

int32_t DAudioSourceDev::EnableDAudio(const std::string &dhId, const std::string &attrs)
{
    DHLOGI("%s: EnableDAudio, dhId: %s.", LOG_TAG, dhId.c_str());
    json jParam;
    jParam["networkId"] = devId_;
    jParam["dhId"] = dhId;
    jParam["attrs"] = attrs;
    auto task =
        GenerateTask(this, &DAudioSourceDev::TaskEnableDAudio, jParam.dump(), "", &DAudioSourceDev::OnEnableTaskResult);
    DHLOGI("%s: EnableDAudio task generate success.", LOG_TAG);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::DisableDAudio(const std::string &dhId)
{
    DHLOGI("%s: DisableDAudio, dhId: %s.", LOG_TAG, dhId.c_str());
    json jParam;
    jParam["networkId"] = devId_;
    jParam["dhId"] = dhId;
    auto task = GenerateTask(this, &DAudioSourceDev::TaskDisableDAudio, jParam.dump(), "",
        &DAudioSourceDev::OnDisableTaskResult);
    DHLOGI("%s: DisableDAudio task generate success.", LOG_TAG);
    return taskQueue_->Produce(task);
}

void DAudioSourceDev::NotifyEvent(const std::shared_ptr<AudioEvent> &event)
{
    if (event == nullptr) {
        DHLOGE("%s: Event is null.");
        return;
    }
    if (IsSpeakerEvent(event)) {
        NotifySpeakerEvent(event);
        return;
    }
    if (IsMicEvent(event)) {
        NotifyMicEvent(event);
        return;
    }
    DHLOGI("%s: NotifyEvent, eventType: %d.", LOG_TAG, event->type);
    switch (event->type) {
        case AudioEventType::NOTIFY_OPEN_CTRL_RESULT:
            HandleNotifyRPC(event);
            break;
        case AudioEventType::NOTIFY_CLOSE_CTRL_RESULT:
            HandleNotifyRPC(event);
            break;
        case AudioEventType::CTRL_OPENED:
            break;
        case AudioEventType::CTRL_CLOSED:
            HandleCtrlTransClosed(event);
            break;
        case AudioEventType::VOLUME_SET:
            HandleVolumeSet(event);
            break;
        case AudioEventType::VOLUME_MUTE_SET:
            HandleVolumeSet(event);
            break;
        case AudioEventType::VOLUME_CHANGE:
            HandleVolumeChange(event);
            break;
        default:
            DHLOGE("%s: Unknown event type.", LOG_TAG);
    }
}

bool DAudioSourceDev::IsSpeakerEvent(const std::shared_ptr<AudioEvent> &event)
{
    const int32_t formatNum = 10;
    const int32_t spkEventBegin = 1;
    return ((int32_t)((int32_t)event->type / formatNum) == spkEventBegin);
}

bool DAudioSourceDev::IsMicEvent(const std::shared_ptr<AudioEvent> &event)
{
    const int32_t formatNum = 10;
    const int32_t micEventBegin = 2;
    return ((int32_t)((int32_t)event->type / formatNum) == micEventBegin);
}

void DAudioSourceDev::NotifySpeakerEvent(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Notify speaker event, eventType: %d.", LOG_TAG, event->type);
    switch (event->type) {
        case AudioEventType::OPEN_SPEAKER:
            HandleOpenDSpeaker(event);
            break;
        case AudioEventType::CLOSE_SPEAKER:
            HandleCloseDSpeaker(event);
            break;
        case AudioEventType::SPEAKER_OPENED:
            HandleDSpeakerOpened(event);
            break;
        case AudioEventType::SPEAKER_CLOSED:
            HandleDSpeakerClosed(event);
            break;
        case AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT:
            HandleNotifyRPC(event);
            break;
        case AudioEventType::NOTIFY_CLOSE_SPEAKER_RESULT:
            HandleNotifyRPC(event);
            break;
        default:
            return;
    }
}

void DAudioSourceDev::NotifyMicEvent(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Notify mic event, eventType: %d.", LOG_TAG, event->type);
    switch (event->type) {
        case AudioEventType::OPEN_MIC:
            HandleOpenDMic(event);
            break;
        case AudioEventType::CLOSE_MIC:
            HandleCloseDMic(event);
            break;
        case AudioEventType::MIC_OPENED:
            HandleDMicOpened(event);
            break;
        case AudioEventType::MIC_CLOSED:
            HandleDMicClosed(event);
            break;
        case AudioEventType::NOTIFY_OPEN_MIC_RESULT:
            HandleNotifyRPC(event);
            break;
        case AudioEventType::NOTIFY_CLOSE_MIC_RESULT:
            HandleNotifyRPC(event);
            break;
        default:
            return;
    }
}

int32_t DAudioSourceDev::HandleOpenDSpeaker(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Open speaker device.", LOG_TAG);
    if (taskQueue_ == nullptr || event == nullptr) {
        DHLOGE("%s: Task queue or event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }

    if (audioSourceCtrlMgr_ == nullptr) {
        audioSourceCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId_, shared_from_this());
    }
    if (!audioSourceCtrlMgr_->IsOpened() && (HandleOpenCtrlTrans(event) != DH_SUCCESS)) {
        DHLOGE("%s: Open ctrl failed.");
        return ERR_DH_AUDIO_SA_OPEN_CTRL_FAILED;
    }

    auto task = GenerateTask(this, &DAudioSourceDev::TaskOpenDSpeaker, event->content, "Open Spk Device",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseDSpeaker(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Close speaker device.", LOG_TAG);
    if (taskQueue_ == nullptr || event == nullptr) {
        DHLOGE("%s: Task queue or event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }

    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseDSpeaker, event->content, "Close Spk Device",
        &DAudioSourceDev::OnTaskResult);
    taskQueue_->Produce(task);

    if (audioSourceCtrlMgr_ != nullptr && !mic_->IsOpened()) {
        DHLOGI("%s: No distributed audio device used, close ctrl trans.", LOG_TAG);
        task = GenerateTask(this, &DAudioSourceDev::TaskCloseCtrlChannel, event->content, "Close Ctrl Trans",
            &DAudioSourceDev::OnTaskResult);
        taskQueue_->Produce(task);
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleDSpeakerOpened(const std::shared_ptr<AudioEvent> &event)
{
    (void) event;
    DHLOGI("%s: Speaker device opened.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleDSpeakerClosed(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Speaker device closed.", LOG_TAG);
    if (speaker_ == nullptr) {
        DHLOGE("%s: Speaker already closed.", LOG_TAG);
        return DH_SUCCESS;
    }
    return speaker_->NotifyHdfAudioEvent(event);
}

int32_t DAudioSourceDev::HandleOpenDMic(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Open mic device.", LOG_TAG);
    if (taskQueue_ == nullptr || event == nullptr) {
        DHLOGE("%s: Task queue or event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }

    if (audioSourceCtrlMgr_ == nullptr) {
        audioSourceCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId_, shared_from_this());
    }
    if (!audioSourceCtrlMgr_->IsOpened() && (HandleOpenCtrlTrans(event) != DH_SUCCESS)) {
        DHLOGE("%s: Open ctrl failed.");
        return ERR_DH_AUDIO_SA_OPEN_CTRL_FAILED;
    }

    auto task = GenerateTask(this, &DAudioSourceDev::TaskOpenDMic, event->content, "Open Mic Device",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseDMic(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Close mic device.", LOG_TAG);
    if (taskQueue_ == nullptr || event == nullptr) {
        DHLOGE("%s: Task queue or event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }

    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseDMic, event->content, "Close Mic Device",
        &DAudioSourceDev::OnTaskResult);
    taskQueue_->Produce(task);

    if (audioSourceCtrlMgr_ != nullptr && !speaker_->IsOpened()) {
        DHLOGI("%s: No distributed audio device used, close ctrl trans.", LOG_TAG);
        task = GenerateTask(this, &DAudioSourceDev::TaskCloseCtrlChannel, event->content, "Close Ctrl Trans",
            &DAudioSourceDev::OnTaskResult);
        taskQueue_->Produce(task);
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleDMicOpened(const std::shared_ptr<AudioEvent> &event)
{
    (void) event;
    DHLOGI("%s: Mic device opened.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleDMicClosed(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Mic device closed.", LOG_TAG);
    if (mic_ == nullptr) {
        DHLOGE("%s: Mic already closed.", LOG_TAG);
        return DH_SUCCESS;
    }
    return mic_->NotifyHdfAudioEvent(event);
}

int32_t DAudioSourceDev::HandleOpenCtrlTrans(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Open control trans.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskOpenCtrlChannel, event->content, "Open Ctrl Trans",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseCtrlTrans(const std::shared_ptr<AudioEvent> &event)
{
    (void) event;
    DHLOGI("%s: Close control trans.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseCtrlChannel, "", "Close Ctrl Trans",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCtrlTransClosed(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Control trans closed.", LOG_TAG);
    HandleCloseCtrlTrans(event);
    if (speaker_->IsOpened()) {
        event->type = SPEAKER_CLOSED;
        HandleDSpeakerClosed(event);
    }
    if (mic_->IsOpened()) {
        event->type = MIC_CLOSED;
        HandleDMicClosed(event);
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleNotifyRPC(const std::shared_ptr<AudioEvent> &event)
{
    std::lock_guard<std::mutex> dataLock(rpcWaitMutex_);
    json jParam = json::parse(event->content, nullptr, false);
    rpcResult_ = (jParam["result"] == DH_SUCCESS) ? true : false;
    DHLOGI("%s: Notify RPC event: %d, result: %d.", LOG_TAG, (int32_t)event->type, (int32_t)rpcResult_);
    switch (event->type) {
        case AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT:
            rpcNotify_ = EVENT_NOTIFY_OPEN_SPK;
            break;
        case AudioEventType::NOTIFY_CLOSE_SPEAKER_RESULT:
            rpcNotify_ = EVENT_NOTIFY_CLOSE_SPK;
            break;
        case AudioEventType::NOTIFY_OPEN_MIC_RESULT:
            rpcNotify_ = EVENT_NOTIFY_OPEN_MIC;
            break;
        case AudioEventType::NOTIFY_CLOSE_MIC_RESULT:
            rpcNotify_ = EVENT_NOTIFY_CLOSE_MIC;
            break;
        case AudioEventType::NOTIFY_OPEN_CTRL_RESULT:
            rpcNotify_ = EVENT_NOTIFY_OPEN_CTRL;
            break;
        case AudioEventType::NOTIFY_CLOSE_CTRL_RESULT:
            rpcNotify_ = EVENT_NOTIFY_CLOSE_CTRL;
            break;
        default:
            DHLOGI("%s: Notify RPC event not define.", LOG_TAG);
            break;
    }
    rpcWaitCond_.notify_all();
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleVolumeSet(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Start handle volume set.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskSetVolume, event->content, "set volume",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleVolumeChange(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Start handle volume change.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskChangeVolume, event->content, "volume change",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::WaitForRPC(const AudioEventType type)
{
    std::unique_lock<std::mutex> lck(rpcWaitMutex_);
    DHLOGI("%s: Wait sink device notify type: %d.", LOG_TAG, type);
    auto status = rpcWaitCond_.wait_for(lck, std::chrono::seconds(RPC_WAIT_SECONDS), [this, type]() {
        switch (type) {
            case AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_OPEN_SPK;
            case AudioEventType::NOTIFY_CLOSE_SPEAKER_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_CLOSE_SPK;
            case AudioEventType::NOTIFY_OPEN_MIC_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_OPEN_MIC;
            case AudioEventType::NOTIFY_CLOSE_MIC_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_CLOSE_MIC;
            case AudioEventType::NOTIFY_OPEN_CTRL_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_OPEN_CTRL;
            case AudioEventType::NOTIFY_CLOSE_CTRL_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_CLOSE_CTRL;
            default:
                return false;
        }
    });
    if (!status) {
        DHLOGE("%s: RPC notify wait timeout(%ds).", LOG_TAG, RPC_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_RPC_WAIT_TIMEOUT;
    }
    if (!rpcResult_) {
        DHLOGI("%s: RPC notify Result Failed.", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }
    DHLOGI("%s: Receive sink device notify type: %d.", LOG_TAG, type);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskEnableDAudio(const std::string &args)
{
    DHLOGI("%s: Enable audio device.", LOG_TAG);
    json jParam = json::parse(args, nullptr, false);
    if (jParam.is_discarded()) {
        DHLOGE("%s: Enable param json is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_ENABLE_PARAM_INVALID;
    }
    if (!jParam.contains("networkId") || !jParam.contains("dhId") || !jParam.contains("attrs")) {
        DHLOGE("%s: Enable param json is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_ENABLE_PARAM_INVALID;
    }
    if (jParam["networkId"] != devId_) {
        DHLOGE("%s: Enable daudio, devId id is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_ENABLE_PARAM_INVALID;
    }
    int32_t dhId = std::stoi((std::string)jParam["dhId"]);

    switch (GetDevTypeByDHId(dhId)) {
        case AUDIO_DEVICE_TYPE_SPEAKER:
            return EnableDSpeaker(dhId, jParam["attrs"]);
        case AUDIO_DEVICE_TYPE_MIC:
            return EnableDMic(dhId, jParam["attrs"]);
        case AUDIO_DEVICE_TYPE_UNKNOWN:
        default:
            DHLOGE("%s: Unknown audio device.", LOG_TAG);
            return ERR_DH_AUDIO_NOT_SUPPORT;
    }
}

int32_t DAudioSourceDev::EnableDSpeaker(const int32_t dhId, const std::string &attrs)
{
    if (speaker_ == nullptr) {
        DHLOGI("%s: Create new speaker device.", LOG_TAG);
        speaker_ = std::make_shared<DSpeakerDev>(devId_, shared_from_this());
    }
    DAUDIO_SYNC_TRACE(DAUDIO_ENABLE_SPK);
    return speaker_->EnableDSpeaker(dhId, attrs);
}

int32_t DAudioSourceDev::EnableDMic(const int32_t dhId, const std::string &attrs)
{
    if (mic_ == nullptr) {
        DHLOGI("%s: Create new mic device.", LOG_TAG);
        mic_ = std::make_shared<DMicDev>(devId_, shared_from_this());
    }
    DAUDIO_SYNC_TRACE(DAUDIO_ENABLE_MIC);
    return mic_->EnableDMic(dhId, attrs);
}

void DAudioSourceDev::OnEnableTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName)
{
    (void)funcName;
    DHLOGI("%s: OnEnableTaskResult.", LOG_TAG);
    json resultJson = json::parse(result, nullptr, false);
    if (resultJson.is_discarded()) {
        DHLOGE("%s: result json is invalid.", LOG_TAG);
        return;
    }
    if (!resultJson.contains("dhId") || !resultJson.contains("networkId")) {
        DHLOGE("%s: result json is invalid.", LOG_TAG);
        return;
    }
    std::string devId = resultJson["networkId"];
    std::string dhId = resultJson["dhId"];
    mgrCallback_->OnEnableAudioResult(devId, dhId, resultCode);
}

int32_t DAudioSourceDev::TaskDisableDAudio(const std::string &args)
{
    DHLOGI("%s, TaskDisableDAudio.", LOG_TAG);
    json jParam = json::parse(args, nullptr, false);
    if (jParam.is_discarded()) {
        DHLOGE("%s: Disable param json is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_DISABLE_PARAM_INVALID;
    }
    if (!jParam.contains("networkId") || !jParam.contains("dhId")) {
        DHLOGE("%s: Disable param json is invalid", LOG_TAG);
        return ERR_DH_AUDIO_SA_DISABLE_PARAM_INVALID;
    }
    if (jParam["networkId"] != devId_) {
        DHLOGE("%s: disable daudio, network id is invalid", LOG_TAG);
        return ERR_DH_AUDIO_SA_ENABLE_PARAM_INVALID;
    }
    int32_t dhId = std::stoi((std::string)jParam["dhId"]);
    switch (GetDevTypeByDHId(dhId)) {
        case AUDIO_DEVICE_TYPE_SPEAKER:
            return DisableDSpeaker(dhId);
        case AUDIO_DEVICE_TYPE_MIC:
            return DisableDMic(dhId);
        case AUDIO_DEVICE_TYPE_UNKNOWN:
        default:
            DHLOGE("%s: Unknown audio device.", LOG_TAG);
            return ERR_DH_AUDIO_NOT_SUPPORT;
    }
}

int32_t DAudioSourceDev::DisableDSpeaker(const int32_t dhId)
{
    if (speaker_ == nullptr) {
        DHLOGE("%s: Speaker device is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    DAUDIO_SYNC_TRACE(DAUDIO_DISABLE_SPK);
    return speaker_->DisableDSpeaker(dhId);
}

int32_t DAudioSourceDev::DisableDMic(const int32_t dhId)
{
    if (mic_ == nullptr) {
        DHLOGE("%s: Mic device is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    DAUDIO_SYNC_TRACE(DAUDIO_DISABLE_MIC);
    return mic_->DisableDMic(dhId);
}

void DAudioSourceDev::OnDisableTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName)
{
    (void)funcName;
    DHLOGI("%s: OnDisableTaskResult.", LOG_TAG);
    json resultJson = json::parse(result, nullptr, false);
    if (resultJson.is_discarded()) {
        DHLOGE("%s: Result json is invalid", LOG_TAG);
        return;
    }
    if (!resultJson.contains("dhId") || !resultJson.contains("networkId")) {
        DHLOGE("%s: Result json is invalid.", LOG_TAG);
        return;
    }
    std::string devId = resultJson["networkId"];
    std::string dhId = resultJson["dhId"];
    mgrCallback_->OnDisableAudioResult(devId, dhId, resultCode);
}

int32_t DAudioSourceDev::TaskOpenDSpeaker(const std::string &args)
{
    DHLOGI("%s: TaskOpenDSpeaker. args: %s.", LOG_TAG, args.c_str());
    if (speaker_ == nullptr) {
        DHLOGE("%s: Speaker device not init", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_DEVICE_NOT_INIT;
    }

    json jAudioParam;
    json jParam = json::parse(args, nullptr, false);
    to_json(jAudioParam, *(speaker_->GetAudioParam()));
    int32_t ret = NotifySinkDev(OPEN_SPEAKER, jAudioParam, jParam["dhId"]);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify sink open speaker failed.", LOG_TAG);
        return ret;
    }
    ret = speaker_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker setup failed.", LOG_TAG);
        return ret;
    }
    ret = speaker_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker start failed.", LOG_TAG);
        speaker_->Stop();
        speaker_->Release();
        return ret;
    }
    NotifyHDF(NOTIFY_OPEN_SPEAKER_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseDSpeaker(const std::string &args)
{
    DHLOGI("%s: TaskCloseDSpeaker. args: %s.", LOG_TAG, args.c_str());
    if (speaker_ == nullptr) {
        DHLOGI("%s: Speaker already closed.", LOG_TAG);
        NotifyHDF(NOTIFY_CLOSE_SPEAKER_RESULT, HDF_EVENT_RESULT_SUCCESS);
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = speaker_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker stop failed.", LOG_TAG);
        closeStatus = false;
    }
    ret = speaker_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker release failed.", LOG_TAG);
        closeStatus = false;
    }
    json jAudioParam;
    json jParam = json::parse(args, nullptr, false);
    NotifySinkDev(CLOSE_SPEAKER, jAudioParam, jParam["dhId"]);

    if (!closeStatus) {
        return ERR_DH_AUDIO_FAILED;
    }
    NotifyHDF(NOTIFY_CLOSE_SPEAKER_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskOpenDMic(const std::string &args)
{
    DHLOGI("%s: TaskOpenDMic. args: %s.", LOG_TAG, args.c_str());
    if (mic_ == nullptr) {
        DHLOGE("%s: Mic device not init", LOG_TAG);
        return ERR_DH_AUDIO_SA_MIC_DEVICE_NOT_INIT;
    }
    int32_t ret = mic_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic setup failed.", LOG_TAG);
        return ret;
    }

    json jAudioParam;
    json jParam = json::parse(args, nullptr, false);
    to_json(jAudioParam, *(mic_->GetAudioParam()));
    ret = NotifySinkDev(OPEN_MIC, jAudioParam, jParam["dhId"]);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify sink open speaker failed.", LOG_TAG);
        mic_->Release();
        return ret;
    }

    ret = mic_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: speaker start failed.", LOG_TAG);
        mic_->Stop();
        mic_->Release();
        return ret;
    }

    NotifyHDF(NOTIFY_OPEN_MIC_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseDMic(const std::string &args)
{
    DHLOGI("%s: TaskCloseDMic. args: %s.", LOG_TAG, args.c_str());
    if (mic_ == nullptr) {
        DHLOGE("%s: Mic device already closed.", LOG_TAG);
        NotifyHDF(NOTIFY_CLOSE_MIC_RESULT, HDF_EVENT_RESULT_SUCCESS);
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = mic_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic stop failed.", LOG_TAG);
        closeStatus = false;
    }
    ret = mic_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic release failed.", LOG_TAG);
        closeStatus = false;
    }
    json jAudioParam;
    json jParam = json::parse(args, nullptr, false);
    NotifySinkDev(CLOSE_MIC, jAudioParam, jParam["dhId"]);

    if (!closeStatus) {
        return ERR_DH_AUDIO_FAILED;
    }
    NotifyHDF(NOTIFY_CLOSE_MIC_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskOpenCtrlChannel(const std::string &args)
{
    DHLOGI("%s: TaskOpenCtrlChannel. args: %s.", LOG_TAG, args.c_str());
    if (audioSourceCtrlMgr_ == nullptr) {
        DHLOGE("%s: Audio source ctrl mgr not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SOURCECTRLMGR_NOT_INIT;
    }

    json jAudioParam;
    json jParam = json::parse(args, nullptr, false);
    int32_t ret = NotifySinkDev(OPEN_CTRL, jAudioParam, jParam["dhId"]);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify sink open ctrl failed.", LOG_TAG);
        return ret;
    }

    ret = audioSourceCtrlMgr_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: SetUp audio ctrl failed.", LOG_TAG);
        return ret;
    }
    ret = audioSourceCtrlMgr_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Start audio ctrl failed.", LOG_TAG);
        audioSourceCtrlMgr_ ->Release();
        audioSourceCtrlMgr_ = nullptr;
        return ret;
    }

    DHLOGI("%s: Open audio ctrl success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseCtrlChannel(const std::string &args)
{
    DHLOGI("%s: TaskCloseCtrlChannel. args: %s.", LOG_TAG, args.c_str());
    if (audioSourceCtrlMgr_ == nullptr) {
        DHLOGI("%s: Audio source ctrl magr already closed.", LOG_TAG);
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = audioSourceCtrlMgr_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop audio ctrl failed.", LOG_TAG);
        closeStatus = false;
    }
    ret = audioSourceCtrlMgr_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release audio ctrl failed.", LOG_TAG);
        closeStatus = false;
    }
    audioSourceCtrlMgr_ = nullptr;
    if (!closeStatus) {
        return ERR_DH_AUDIO_FAILED;
    }

    DHLOGI("%s: Close audio ctrl mgr success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskSetVolume(const std::string &args)
{
    DHLOGI("%s: SetVolumeTask. args: %s.", LOG_TAG, args.c_str());
    if (audioSourceCtrlMgr_ == nullptr) {
        DHLOGE("%s: AudioSourceCtrlMgr not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SOURCECTRLMGR_NOT_INIT;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::VOLUME_SET;
    event->content = args;
    int32_t ret = audioSourceCtrlMgr_->SendAudioEvent(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: SendAudioEvent failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskChangeVolume(const std::string &args)
{
    DHLOGI("%s: ChangeVolumeTask. args: %s.", LOG_TAG, args.c_str());
    if (speaker_ == nullptr) {
        DHLOGE("%s: Speaker device not init", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_DEVICE_NOT_INIT;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::VOLUME_CHANGE;
    event->content = args;
    int32_t ret = speaker_->NotifyHdfAudioEvent(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: TaskChangeVolume failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

void DAudioSourceDev::OnTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName)
{
    (void)resultCode;
    (void)result;
    (void)funcName;
    DHLOGI("%s: OnTaskResult. resultcode: %d, result: %s, funcName: %s", LOG_TAG, resultCode, result.c_str(),
        funcName.c_str());
}

int32_t DAudioSourceDev::NotifySinkDev(const AudioEventType type, const json Param, const std::string dhId)
{
    constexpr uint32_t eventOffset = 4;
    json jParam;
    jParam["dhId"] = dhId;
    jParam["audioParam"] = Param;
    jParam["eventType"] = type;
    jParam["networkId"] = localDevId_;
    DAudioSourceManager::GetInstance().DAudioNotify(devId_, dhId, type, jParam.dump());

    return WaitForRPC((AudioEventType)((int32_t)type + eventOffset));
}

void DAudioSourceDev::NotifyHDF(const AudioEventType type, const std::string result)
{
    auto event = std::make_shared<AudioEvent>();
    event->type = type;
    event->content = result;
    switch (type) {
        case NOTIFY_OPEN_SPEAKER_RESULT:
        case NOTIFY_CLOSE_SPEAKER_RESULT:
            speaker_->NotifyHdfAudioEvent(event);
            break;
        case NOTIFY_OPEN_MIC_RESULT:
        case NOTIFY_CLOSE_MIC_RESULT:
            mic_->NotifyHdfAudioEvent(event);
            break;
        default:
            DHLOGE("%s: NotifyHDF unknown type.", LOG_TAG);
            break;
    }
}

void to_json(json &j, const AudioParam &audioParam)
{
    j = json {
        {"samplingRate", audioParam.comParam.sampleRate},
        {"format", audioParam.comParam.bitFormat},
        {"channels", audioParam.comParam.channelMask},
        {"contentType", audioParam.renderOpts.contentType},
        {"streamUsage", audioParam.renderOpts.streamUsage},
        {"sourceType", audioParam.CaptureOpts.sourceType},
    };
}

void from_json(const json &j, AudioParam &audioParam)
{
    j.at("samplingRate").get_to(audioParam.comParam.sampleRate);
    j.at("format").get_to(audioParam.comParam.bitFormat);
    j.at("channels").get_to(audioParam.comParam.channelMask);
    j.at("contentType").get_to(audioParam.renderOpts.contentType);
    j.at("streamUsage").get_to(audioParam.renderOpts.streamUsage);
    j.at("sourceType").get_to(audioParam.CaptureOpts.sourceType);
}
} // namespace DistributedHardware
} // namespace OHOS