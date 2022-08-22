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

int32_t DAudioSinkDev::AwakeAudioDev(const std::string localDevId)
{
    constexpr size_t capacity = 20;
    taskQueue_ = std::make_shared<TaskQueue>(capacity);
    taskQueue_->Start();
    localDevId_ = localDevId;
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
    DHLOGI("%s: Notify event, eventType: %d.", LOG_TAG, (int32_t)audioEvent->type);
    if (IsSpeakerEvent(audioEvent)) {
        NotifySpeakerEvent(audioEvent);
        return;
    } else if (IsMicEvent(audioEvent)) {
        NotifyMicEvent(audioEvent);
        return;
    } else if (IsVolumeEvent(audioEvent)) {
        NotifyVolumeEvent(audioEvent);
        return;
    }
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
        case SET_PARAM:
            NotifySetParam(audioEvent);
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

void DAudioSinkDev::NotifySpeakerEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify speaker event, eventType: %d.", LOG_TAG, audioEvent->type);
    switch (audioEvent->type) {
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
        default:
            return;
    }
}

void DAudioSinkDev::NotifyMicEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify mic event, eventType: %d.", LOG_TAG, audioEvent->type);
    switch (audioEvent->type) {
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
            return;
    }
}

void DAudioSinkDev::NotifyVolumeEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify volume event, eventType: %d.", LOG_TAG, audioEvent->type);
    switch (audioEvent->type) {
        case VOLUME_SET:
            NotifySetVolume(audioEvent);
            break;
        case VOLUME_MUTE_SET:
            NotifySetMute(audioEvent);
            break;
        case VOLUME_CHANGE:
            NotifyVolumeChange(audioEvent);
            break;
        default:
            return;
    }
}

bool DAudioSinkDev::IsSpeakerEvent(const std::shared_ptr<AudioEvent> &event)
{
    const int32_t formatNum = 10;
    const int32_t spkEventBegin = 1;
    return ((int32_t)((int32_t)event->type / formatNum) == spkEventBegin);
}

bool DAudioSinkDev::IsMicEvent(const std::shared_ptr<AudioEvent> &event)
{
    const int32_t formatNum = 10;
    const int32_t micEventBegin = 2;
    return ((int32_t)((int32_t)event->type / formatNum) == micEventBegin);
}

bool DAudioSinkDev::IsVolumeEvent(const std::shared_ptr<AudioEvent> &event)
{
    const int32_t formatNum = 10;
    const int32_t volumeEventBegin = 3;
    return ((int32_t)((int32_t)event->type / formatNum) == volumeEventBegin);
}
int32_t DAudioSinkDev::NotifyOpenCtrlChannel(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify open ctrl channel.", LOG_TAG);
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
    DHLOGI("%s: Notify close ctrl channel.", LOG_TAG);
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
    DHLOGI("%s: Notify ctrl opened.", LOG_TAG);
    (void)audioEvent;
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyCtrlClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify ctrl closed.", LOG_TAG);
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
    DHLOGI("%s: Notify open dSpeaker.", LOG_TAG);
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
    DHLOGI("%s: Notify close dSpeaker.", LOG_TAG);
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
    DHLOGI("%s: Notify speaker opened.", LOG_TAG);
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
    (void)audioEvent;
    DHLOGI("%s: Notify speaker closed.", LOG_TAG);
    auto task =
        GenerateTask(this, &DAudioSinkDev::TaskCloseDSpeaker, "", "Sink Close Speaker", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyMicOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void)audioEvent;
    DHLOGI("%s: Notify mic opened.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyMicClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify mic closed.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: Audio event is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDMic, "", "Sink Close Mic", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyOpenMic(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify open dMic.", LOG_TAG);
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
    DHLOGI("%s: Notify close dMic.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDMic, audioEvent->content, "Sink Close Mic",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetParam(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify set param.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskSetParameter, audioEvent->content, "Sink Set Param",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetVolume(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify set volume.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskSetVolume, audioEvent->content,
        "Sink Notify SetVolume", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetMute(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify set mute.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskSetMute, audioEvent->content,
        "Sink NotifySetMute", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyVolumeChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify volume change.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskVolumeChange, audioEvent->content,
        "Sink Notify Volume Change", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyFocusChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify focus change.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskFocusChange, audioEvent->content,
        "Sink Notify Focus Change", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyRenderStateChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: Notify render state change.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskRenderStateChange,
        audioEvent->content, "Sink Notify Render State Change", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::TaskOpenCtrlChannel(const std::string &args)
{
    DHLOGI("%s: Open ctrl channel.", LOG_TAG);
    json jParam = json::parse(args, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_DH_ID })) {
        return ERR_DH_AUDIO_FAILED;
    }

    if (audioCtrlMgr_ != nullptr && audioCtrlMgr_->IsOpened()) {
        DHLOGI("%s: Ctrl channel already opened.", LOG_TAG);
        NotifySourceDev(NOTIFY_OPEN_CTRL_RESULT, jParam[KEY_DH_ID], DH_SUCCESS);
        return DH_SUCCESS;
    }

    audioCtrlMgr_ = std::make_shared<DAudioSinkDevCtrlMgr>(devId_, shared_from_this());
    int32_t ret = audioCtrlMgr_->SetUp();
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
    (void)args;
    DHLOGI("%s: Close ctrl channel.", LOG_TAG);
    if (audioCtrlMgr_ == nullptr || !audioCtrlMgr_->IsOpened()) {
        audioCtrlMgr_ = nullptr;
        DHLOGI("%s: Ctrl channel already closed.", LOG_TAG);
        return DH_SUCCESS;
    }

    int32_t ret = audioCtrlMgr_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:Stop ctrl mgr failed.", LOG_TAG);
    }
    ret = audioCtrlMgr_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:Release ctrl mgr failed.", LOG_TAG);
    }
    audioCtrlMgr_ = nullptr;
    DHLOGI("%s: Close ctrl channel success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskOpenDSpeaker(const std::string &args)
{
    DHLOGI("%s: Open speaker device.", LOG_TAG);
    json jParam = json::parse(args, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_DH_ID, KEY_AUDIO_PARAM })) {
        return ERR_DH_AUDIO_FAILED;
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
    (void)args;
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
    if (!JsonParamCheck(jParam, { KEY_DH_ID, KEY_AUDIO_PARAM })) {
        return ERR_DH_AUDIO_FAILED;
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
    (void)args;
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
    auto event = std::make_shared<AudioEvent>(AudioEventType::EVENT_UNKNOWN, args);

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
    auto event = std::make_shared<AudioEvent>(AudioEventType::VOLUME_SET, args);
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
    auto event = std::make_shared<AudioEvent>(AudioEventType::VOLUME_MUTE_SET, args);
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
    auto event = std::make_shared<AudioEvent>(AudioEventType::VOLUME_CHANGE, args);
    if (audioCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return audioCtrlMgr_->SendAudioEvent(event);
}

int32_t DAudioSinkDev::TaskFocusChange(const std::string &args)
{
    DHLOGI("%s: Audio focus changed.", LOG_TAG);
    auto event = std::make_shared<AudioEvent>(AudioEventType::AUDIO_FOCUS_CHANGE, args);
    if (audioCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return audioCtrlMgr_->SendAudioEvent(event);
}

int32_t DAudioSinkDev::TaskRenderStateChange(const std::string &args)
{
    DHLOGI("%s: Audio render state changed.", LOG_TAG);
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>(AudioEventType::AUDIO_RENDER_STATE_CHANGE, args);
    if (audioCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return audioCtrlMgr_->SendAudioEvent(event);
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

int32_t DAudioSinkDev::from_json(const json &j, AudioParam &audioParam)
{
    if (!JsonParamCheck(j,
        { KEY_SAMPLING_RATE, KEY_CHANNELS, KEY_FORMAT, KEY_SOURCE_TYPE, KEY_CONTENT_TYPE, KEY_STREAM_USAGE })) {
        return ERR_DH_AUDIO_FAILED;
    }
    j.at(KEY_SAMPLING_RATE).get_to(audioParam.comParam.sampleRate);
    j.at(KEY_CHANNELS).get_to(audioParam.comParam.channelMask);
    j.at(KEY_FORMAT).get_to(audioParam.comParam.bitFormat);
    j.at(KEY_SOURCE_TYPE).get_to(audioParam.CaptureOpts.sourceType);
    j.at(KEY_CONTENT_TYPE).get_to(audioParam.renderOpts.contentType);
    j.at(KEY_STREAM_USAGE).get_to(audioParam.renderOpts.streamUsage);
    return DH_SUCCESS;
}

bool DAudioSinkDev::JsonParamCheck(const json &jParam, const std::initializer_list<std::string> &key)
{
    if (jParam.is_discarded()) {
        DHLOGE("%s: Json parameter is invalid.", LOG_TAG);
        return false;
    }
    for (auto it = key.begin(); it != key.end(); it++) {
        if (!jParam.contains(*it)) {
            DHLOGE("%s Json parameter not contain param(%s).", (*it).c_str(), LOG_TAG);
            return false;
        }
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
