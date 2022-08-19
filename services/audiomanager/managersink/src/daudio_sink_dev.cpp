/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "daudio_sink_dev.h"

#include "nlohmann/json.hpp"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_sink_manager.h"
#include "daudio_util.h"
#include "task_impl.h"

using json = nlohmann::json;
namespace OHOS {
namespace DistributedHardware {
DAudioSinkDev::DAudioSinkDev(const std::string &devId) : devId_(devId)
{
    DHLOGI("%s: Distributed audio sink device constructed, devId: %s.", LOG_TAG, GetAnonyString(devId).c_str());
}

DAudioSinkDev::~DAudioSinkDev()
{
    DHLOGI("%s: Distributed audio sink device destructed, devId: %s.", LOG_TAG, GetAnonyString(devId_).c_str());
}

int32_t DAudioSinkDev::AwakeAudioDev()
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

void DAudioSinkDev::SleepAudioDev()
{
    taskQueue_->Stop();
    taskQueue_ = nullptr;
}

void DAudioSinkDev::NotifyEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    if (audioEvent == nullptr) {
        DHLOGE("%s: AudioEvent is null.", LOG_TAG);
        return;
    }
    DHLOGI("%s: NotifyAudioEvent, eventType: %d.", LOG_TAG, (int32_t)audioEvent->type);
    switch (audioEvent->type) {
        case OPEN_CTRL:
            NotifyOpenCtrlChannel(audioEvent);
            break;
        case CLOSE_CTRL:
            NotifyCloseCtrlChannel(audioEvent);
            break;
        case CTRL_OPENED:
            NotifyCtrlOpened(audioEvent);
            break;
        case CTRL_CLOSED:
            NotifyCtrlClosed(audioEvent);
            break;
        case OPEN_SPEAKER:
            NotifyOpenSpeaker(audioEvent);
            break;
        case CLOSE_SPEAKER:
            NotifyCloseSpeaker(audioEvent);
            break;
        case SPEAKER_OPENED:
            NotifySpeakerOpened(audioEvent);
            break;
        case SPEAKER_CLOSED:
            NotifySpeakerClosed(audioEvent);
            break;
        case OPEN_MIC:
            NotifyOpenMic(audioEvent);
            break;
        case CLOSE_MIC:
            NotifyCloseMic(audioEvent);
            break;
        case MIC_OPENED:
            NotifyMicOpened(audioEvent);
            break;
        case MIC_CLOSED:
            NotifyMicClosed(audioEvent);
            break;
        default:
            NotifyEventSub(audioEvent);
    }
}
void DAudioSinkDev::NotifyEventSub(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyEventSub.", LOG_TAG);
    switch (audioEvent->type) {
        case SET_PARAM:
            NotifySetParam(audioEvent);
            break;
        case VOLUME_SET:
            NotifySetVolume(audioEvent);
            break;
        case VOLUME_MUTE_SET:
            NotifySetMute(audioEvent);
            break;
        case VOLUME_CHANGE:
            NotifyVolumeChange(audioEvent);
            break;
        case AUDIO_FOCUS_CHANGE:
            NotifyFocusChange(audioEvent);
            break;
        case AUDIO_RENDER_STATE_CHANGE:
            NotifyRenderStateChange(audioEvent);
            break;
        default:
            DHLOGE("%s: Unknown event type: %d", LOG_TAG, (int32_t)audioEvent->type);
    }
}

int32_t DAudioSinkDev::NotifyOpenCtrlChannel(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyOpenCtrlChannel.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskOpenCtrlChannel, audioEvent->content, "Sink Open Ctrl",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseCtrlChannel(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyCloseCtrlChannel.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseCtrlChannel, audioEvent->content, "Sink Close Ctrl",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCtrlOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyCtrlOpened.", LOG_TAG);
    (void)audioEvent;
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyCtrlClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyCtrlClosed.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task =
        GenerateTask(this, &DAudioSinkDev::TaskCloseCtrlChannel, "", "Sink Close Ctrl", &DAudioSinkDev::OnTaskResult);
    taskQueue_->Produce(task);
    task =
        GenerateTask(this, &DAudioSinkDev::TaskCloseDSpeaker, "", "Sink Close Speaker", &DAudioSinkDev::OnTaskResult);
    taskQueue_->Produce(task);
    task = GenerateTask(this, &DAudioSinkDev::TaskCloseDMic, "", "Sink Close Mic", &DAudioSinkDev::OnTaskResult);
    taskQueue_->Produce(task);
    DAudioSinkManager::GetInstance().OnSinkDevReleased(devId_);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyOpenSpeaker(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyOpenDSpeaker.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskOpenDSpeaker, audioEvent->content, "Sink Open Speaker",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseSpeaker(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyCloseDSpeaker.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDSpeaker, audioEvent->content, "Sink Close Speaker",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySpeakerOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifySpeakerOpened.", LOG_TAG);
    if (audioEvent == nullptr || speakerClient_ == nullptr) {
        DHLOGE("%s: Audio event or speaker client is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    int32_t ret = speakerClient_->StartRender();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Start Render failed. ret: %d.", LOG_TAG, ret);
        return ret;
    }
    DHLOGI("%s: Notify primary volume.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSinkDev::TaskVolumeChange, audioEvent->content, "Sink Notify Vol Change",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySpeakerClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void) audioEvent;
    DHLOGI("%s: NotifySpeakerClosed.", LOG_TAG);
    auto task =
        GenerateTask(this, &DAudioSinkDev::TaskCloseDSpeaker, "", "Sink Close Speaker", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyMicOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void) audioEvent;
    DHLOGI("%s: NotifyMicOpened.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyMicClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyMicClosed.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDMic, "", "Sink CloseMic", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyOpenMic(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyOpenDMic.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskOpenDMic, audioEvent->content, "Sink Open Mic",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseMic(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyCloseDMic.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDMic, audioEvent->content, "Sink CloseMic",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetParam(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifySetParam.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskSetParameter, audioEvent->content, "Sink SetParam",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetVolume(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Start notify set volume.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskSetVolume, audioEvent->content,
        "Sink NotifySetVolume", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetMute(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Start notify set mute.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskSetMute, audioEvent->content,
        "Sink NotifySetMute", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyVolumeChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Start NotifyVolumeChange.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskVolumeChange, audioEvent->content,
        "Sink NotifyVolumeChange", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyFocusChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Start NotifyFocusChange.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskFocusChange, audioEvent->content,
        "Sink NotifyFocusChange", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyRenderStateChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Start NotifyRenderStateChange.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskRenderStateChange,
        audioEvent->content, "Sink NotifyRenderStateChange", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::TaskOpenCtrlChannel(const std::string &args)
{
    DHLOGI("%s: Open ctrl channel.", LOG_TAG);
    json jParam = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(jParam)) {
        DHLOGE("%s: Open ctrl param json is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }

    if (dAudioSinkDevCtrlMgr_ != nullptr && dAudioSinkDevCtrlMgr_->IsOpened()) {
        DHLOGI("%s: Ctrl channel already opened.", LOG_TAG);
        NotifySourceDev(NOTIFY_OPEN_CTRL_RESULT, jParam[KEY_DH_ID], DH_SUCCESS);
        return DH_SUCCESS;
    }

    dAudioSinkDevCtrlMgr_ = std::make_shared<DAudioSinkDevCtrlMgr>(devId_, shared_from_this());
    int32_t ret = dAudioSinkDevCtrlMgr_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:SetUp ctrl mgr failed.", LOG_TAG);
        NotifySourceDev(NOTIFY_OPEN_CTRL_RESULT, jParam[KEY_DH_ID], ERR_DH_AUDIO_FAILED);
        return ret;
    }

    NotifySourceDev(NOTIFY_OPEN_CTRL_RESULT, jParam[KEY_DH_ID], DH_SUCCESS);
    DHLOGI("%s: Open ctrl channel success, notify open ctrl result.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskCloseCtrlChannel(const std::string &args)
{
    (void) args;
    DHLOGI("%s: Close ctrl channel.", LOG_TAG);
    if (dAudioSinkDevCtrlMgr_ == nullptr || !dAudioSinkDevCtrlMgr_->IsOpened()) {
        dAudioSinkDevCtrlMgr_ = nullptr;
        DHLOGI("%s: Ctrl channel already closed.", LOG_TAG);
        return DH_SUCCESS;
    }

    int32_t ret = dAudioSinkDevCtrlMgr_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:Stop ctrl mgr failed.", LOG_TAG);
    }
    ret = dAudioSinkDevCtrlMgr_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:Release ctrl mgr failed.", LOG_TAG);
    }
    dAudioSinkDevCtrlMgr_ = nullptr;
    DHLOGI("%s: Close ctrl channel success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskOpenDSpeaker(const std::string &args)
{
    DHLOGI("%s: Open speaker device.", LOG_TAG);
    json jParam = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(jParam)) {
        DHLOGE("%s: Open speaker json is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    spkDhId_ = jParam[KEY_DH_ID];
    AudioParam audioParam;
    int32_t ret = from_json(jParam[KEY_AUDIO_PARAM], audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get audio param from json failed, error code %d.", LOG_TAG, ret);
        return ret;
    }

    speakerClient_ = std::make_shared<DSpeakerClient>(devId_, shared_from_this());
    ret = speakerClient_->SetUp(audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Setup speaker failed.", LOG_TAG);
        NotifySourceDev(NOTIFY_OPEN_SPEAKER_RESULT, spkDhId_, ERR_DH_AUDIO_FAILED);
        return ERR_DH_AUDIO_FAILED;
    }

    NotifySourceDev(NOTIFY_OPEN_SPEAKER_RESULT, spkDhId_, DH_SUCCESS);
    DHLOGI("%s: Open speaker device task excute success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskCloseDSpeaker(const std::string &args)
{
    (void) args;
    DHLOGI("%s: Close speaker device.", LOG_TAG);
    if (speakerClient_ == nullptr) {
        DHLOGI("%s: Speaker client is null or already closed.", LOG_TAG);
        NotifySourceDev(NOTIFY_CLOSE_SPEAKER_RESULT, spkDhId_, DH_SUCCESS);
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = speakerClient_->StopRender();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop speaker client failed.", LOG_TAG);
        closeStatus = false;
    }
    ret = speakerClient_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release speaker client failed.", LOG_TAG);
        closeStatus = false;
    }
    speakerClient_ = nullptr;

    if (closeStatus) {
        NotifySourceDev(NOTIFY_CLOSE_SPEAKER_RESULT, spkDhId_, DH_SUCCESS);
    } else {
        NotifySourceDev(NOTIFY_CLOSE_SPEAKER_RESULT, spkDhId_, ERR_DH_AUDIO_FAILED);
    }
    DHLOGI("%s: Close speaker device task excute success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskOpenDMic(const std::string &args)
{
    DHLOGI("%s: Open mic device.", LOG_TAG);
    json jParam = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(jParam)) {
        DHLOGE("%s: Open mic json is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    micDhId_ = jParam[KEY_DH_ID];
    AudioParam audioParam;
    int32_t ret = from_json(jParam[KEY_AUDIO_PARAM], audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get audio param from json failed, error code %d.", LOG_TAG, ret);
        return ret;
    }

    micClient_ = std::make_shared<DMicClient>(devId_, shared_from_this());
    ret = micClient_->SetUp(audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: SetUp mic failed.", LOG_TAG);
        NotifySourceDev(NOTIFY_OPEN_MIC_RESULT, micDhId_, ERR_DH_AUDIO_FAILED);
        return ERR_DH_AUDIO_FAILED;
    }
    ret = micClient_->StartCapture();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Start capture failed.", LOG_TAG);
        NotifySourceDev(NOTIFY_OPEN_MIC_RESULT, micDhId_, ERR_DH_AUDIO_FAILED);
        return ret;
    }
    
    NotifySourceDev(NOTIFY_OPEN_MIC_RESULT, micDhId_, DH_SUCCESS);
    DHLOGI("%s: Open mic device task excute success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskCloseDMic(const std::string &args)
{
    (void) args;
    DHLOGI("%s: Close mic device.", LOG_TAG);
    if (micClient_ == nullptr) {
        DHLOGI("%s: mic client is null or already closed.", LOG_TAG);
        NotifySourceDev(NOTIFY_CLOSE_MIC_RESULT, micDhId_, DH_SUCCESS);
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = micClient_->StopCapture();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop mic client failed.", LOG_TAG);
        closeStatus = false;
    }
    ret = micClient_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release mic client failed.", LOG_TAG);
        closeStatus = false;
    }
    micClient_ = nullptr;

    if (closeStatus) {
        NotifySourceDev(NOTIFY_CLOSE_MIC_RESULT, micDhId_, DH_SUCCESS);
    } else {
        NotifySourceDev(NOTIFY_CLOSE_MIC_RESULT, micDhId_, ERR_DH_AUDIO_FAILED);
    }
    DHLOGI("%s: Close mic device task excute success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskSetParameter(const std::string &args)
{
    DHLOGI("%s: Set audio param.", LOG_TAG);
    json resultJson = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(resultJson)) {
        DHLOGE("%s: Result json is invalid.", LOG_TAG);
        return ERR_DH_AUDIO_SA_ENABLE_PARAM_INVALID;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::EVENT_UNKNOWN;
    event->content = args;
    if (speakerClient_ == nullptr) {
        return ERR_DH_AUDIO_SA_SPEAKER_CLIENT_NOT_INIT;
    }
    return speakerClient_->SetAudioParameters(event);
}

int32_t DAudioSinkDev::TaskSetVolume(const std::string &args)
{
    DHLOGI("%s:Set audio volume.", LOG_TAG);
    if (speakerClient_ == nullptr) {
        DHLOGE("%s: Speaker client already closed.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::VOLUME_SET;
    event->content = args;
    int32_t ret = speakerClient_->SetAudioParameters(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Volume set failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s: Set audio volume success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskSetMute(const std::string &args)
{
    DHLOGI("%s:Set audio mute.", LOG_TAG);
    if (speakerClient_ == nullptr) {
        DHLOGE("%s: Speaker client already closed.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::VOLUME_MUTE_SET;
    event->content = args;
    int32_t ret = speakerClient_->SetMute(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Set mute failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s: Set mute success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskVolumeChange(const std::string &args)
{
    DHLOGI("%s: Audio volume changed.", LOG_TAG);
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::VOLUME_CHANGE;
    event->content = args;
    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return dAudioSinkDevCtrlMgr_->SendAudioEvent(event);
}

int32_t DAudioSinkDev::TaskFocusChange(const std::string &args)
{
    DHLOGI("%s: Audio focus changed.", LOG_TAG);
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::AUDIO_FOCUS_CHANGE;
    event->content = args;
    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return dAudioSinkDevCtrlMgr_->SendAudioEvent(event);
}

int32_t DAudioSinkDev::TaskRenderStateChange(const std::string &args)
{
    DHLOGI("%s: Audio render state changed.", LOG_TAG);
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::AUDIO_RENDER_STATE_CHANGE;
    event->content = args;
    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return dAudioSinkDevCtrlMgr_->SendAudioEvent(event);
}

void DAudioSinkDev::OnTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName)
{
    (void)resultCode;
    (void)result;
    (void)funcName;
    DHLOGI("%s: OnTaskResult. resultCode: %d, funcName: %s", LOG_TAG, resultCode, funcName.c_str());
}

void DAudioSinkDev::NotifySourceDev(const AudioEventType type, const std::string dhId, const int32_t result)
{
    json jEvent;
    jEvent[KEY_DH_ID] = dhId;
    jEvent[KEY_RESULT] = result;
    jEvent[KEY_EVENT_TYPE] = type;
    jEvent[KEY_DEV_ID] = localDevId_;
    DAudioSinkManager::GetInstance().DAudioNotify(devId_, dhId, type, jEvent.dump());
}

bool DAudioSinkDev::JudgeJsonValid(const json &resultJson)
{
    if (resultJson.is_discarded()) {
        DHLOGE("%s: JudgeJsonValid: result json is invalid", LOG_TAG);
        return false;
    }
    if (!resultJson.contains(KEY_DH_ID)) {
        DHLOGE("%s: JudgeJsonValid: result json dose not contain key.", LOG_TAG);
        return false;
    }
    return true;
}

int32_t from_json(const json &j, AudioParam &audioParam)
{
    if (j.is_discarded()) {
        DHLOGE("DAudioSinkDev: Json data is discarded.");
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }

    if (!j.contains(KEY_SAMPLING_RATE) || !j.contains(KEY_CHANNELS) || !j.contains(KEY_FORMAT) ||
        !j.contains(KEY_SOURCE_TYPE) || !j.contains(KEY_CONTENT_TYPE) || !j.contains(KEY_STREAM_USAGE)) {
        DHLOGE("DAudioSinkDev: Json data dose not contain some keys.");
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    j.at(KEY_SAMPLING_RATE).get_to(audioParam.comParam.sampleRate);
    j.at(KEY_CHANNELS).get_to(audioParam.comParam.channelMask);
    j.at(KEY_FORMAT).get_to(audioParam.comParam.bitFormat);
    j.at(KEY_SOURCE_TYPE).get_to(audioParam.CaptureOpts.sourceType);
    j.at(KEY_CONTENT_TYPE).get_to(audioParam.renderOpts.contentType);
    j.at(KEY_STREAM_USAGE).get_to(audioParam.renderOpts.streamUsage);
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
