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

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSinkDev"

using json = nlohmann::json;
namespace OHOS {
namespace DistributedHardware {
DAudioSinkDev::DAudioSinkDev(const std::string &devId) : devId_(devId)
{
    DHLOGI("Distributed audio sink device constructed, devId: %s.", GetAnonyString(devId).c_str());
}

DAudioSinkDev::~DAudioSinkDev()
{
    DHLOGI("Distributed audio sink device destructed, devId: %s.", GetAnonyString(devId_).c_str());
}

int32_t DAudioSinkDev::AwakeAudioDev()
{
    constexpr size_t capacity = 20;
    taskQueue_ = std::make_shared<TaskQueue>(capacity);
    taskQueue_->Start();
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
        DHLOGE("Audio event is null.");
        return;
    }
    DHLOGI("Notify event, eventType: %d.", (int32_t)audioEvent->type);
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
            DHLOGE("Unknown event type: %d", (int32_t)audioEvent->type);
    }
}

void DAudioSinkDev::NotifySpeakerEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify speaker event, eventType: %d.", audioEvent->type);
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
    DHLOGI("Notify mic event, eventType: %d.", audioEvent->type);
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
    DHLOGI("Notify volume event, eventType: %d.", audioEvent->type);
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
    DHLOGI("Notify open ctrl channel.");
    if (audioEvent == nullptr) {
        DHLOGE("Audio event is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskOpenCtrlChannel, audioEvent->content, "Sink Open Ctrl",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseCtrlChannel(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify close ctrl channel.");
    if (audioEvent == nullptr) {
        DHLOGE("Audio event is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseCtrlChannel, audioEvent->content, "Sink Close Ctrl",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCtrlOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify ctrl opened.");
    (void)audioEvent;
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyCtrlClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify ctrl closed.");
    if (audioEvent == nullptr) {
        DHLOGE("Audio event is null.");
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
    DHLOGI("Notify open dSpeaker.");
    if (audioEvent == nullptr) {
        DHLOGE("Audio event is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskOpenDSpeaker, audioEvent->content, "Sink Open Speaker",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseSpeaker(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify close dSpeaker.");
    if (audioEvent == nullptr) {
        DHLOGE("Audio event is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDSpeaker, audioEvent->content, "Sink Close Speaker",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySpeakerOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify speaker opened.");
    if (audioEvent == nullptr || speakerClient_ == nullptr) {
        DHLOGE("Audio event or speaker client is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    int32_t ret = speakerClient_->StartRender();
    if (ret != DH_SUCCESS) {
        DHLOGE("Start render failed. ret: %d.", ret);
        return ret;
    }
    DHLOGI("Notify primary volume.");
    auto task = GenerateTask(this, &DAudioSinkDev::TaskVolumeChange, audioEvent->content, "Sink Notify Vol Change",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySpeakerClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void)audioEvent;
    DHLOGI("Notify speaker closed.");
    auto task =
        GenerateTask(this, &DAudioSinkDev::TaskCloseDSpeaker, "", "Sink Close Speaker", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyMicOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void)audioEvent;
    DHLOGI("Notify mic opened.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyMicClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify mic closed.");
    if (audioEvent == nullptr) {
        DHLOGE("Audio event is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDMic, "", "Sink Close Mic", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyOpenMic(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify open dMic.");
    if (audioEvent == nullptr) {
        DHLOGE("Audio event is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskOpenDMic, audioEvent->content, "Sink Open Mic",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseMic(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify close dMic.");
    auto task = GenerateTask(this, &DAudioSinkDev::TaskCloseDMic, audioEvent->content, "Sink Close Mic",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetParam(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify set param.");
    if (audioEvent == nullptr) {
        DHLOGE("audio event is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSinkDev::TaskSetParameter, audioEvent->content, "Sink Set Param",
        &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetVolume(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify set volume.");
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskSetVolume, audioEvent->content,
        "Sink Notify SetVolume", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetMute(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify set mute.");
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskSetMute, audioEvent->content,
        "Sink NotifySetMute", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyVolumeChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify volume change.");
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskVolumeChange, audioEvent->content,
        "Sink Notify Volume Change", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyFocusChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify focus change.");
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskFocusChange, audioEvent->content,
        "Sink Notify Focus Change", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyRenderStateChange(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("Notify render state change.");
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::TaskRenderStateChange,
        audioEvent->content, "Sink Notify Render State Change", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::TaskOpenCtrlChannel(const std::string &args)
{
    DHLOGI("Open ctrl channel.");
    json jParam = json::parse(args, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_DH_ID })) {
        return ERR_DH_AUDIO_FAILED;
    }

    if (audioCtrlMgr_ != nullptr && audioCtrlMgr_->IsOpened()) {
        DHLOGI("Ctrl channel already opened.");
        NotifySourceDev(NOTIFY_OPEN_CTRL_RESULT, jParam[KEY_DH_ID], DH_SUCCESS);
        return DH_SUCCESS;
    }

    audioCtrlMgr_ = std::make_shared<DAudioSinkDevCtrlMgr>(devId_, shared_from_this());
    int32_t ret = audioCtrlMgr_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("SetUp ctrl mgr failed.");
        NotifySourceDev(NOTIFY_OPEN_CTRL_RESULT, jParam[KEY_DH_ID], ERR_DH_AUDIO_FAILED);
        return ret;
    }

    NotifySourceDev(NOTIFY_OPEN_CTRL_RESULT, jParam[KEY_DH_ID], DH_SUCCESS);
    DHLOGI("Open ctrl channel success, notify open ctrl result.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskCloseCtrlChannel(const std::string &args)
{
    (void)args;
    DHLOGI("Close ctrl channel.");
    if (audioCtrlMgr_ == nullptr || !audioCtrlMgr_->IsOpened()) {
        audioCtrlMgr_ = nullptr;
        DHLOGI("Ctrl channel already closed.");
        return DH_SUCCESS;
    }

    int32_t ret = audioCtrlMgr_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("Stop ctrl mgr failed.");
    }
    ret = audioCtrlMgr_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Release ctrl mgr failed.");
    }
    audioCtrlMgr_ = nullptr;
    DHLOGI("Close ctrl channel success.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskOpenDSpeaker(const std::string &args)
{
    DHLOGI("Open speaker device.");
    json jParam = json::parse(args, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_DH_ID, KEY_AUDIO_PARAM })) {
        return ERR_DH_AUDIO_FAILED;
    }
    spkDhId_ = jParam[KEY_DH_ID];
    AudioParam audioParam;
    int32_t ret = from_json(jParam[KEY_AUDIO_PARAM], audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("Get audio param from json failed, error code %d.", ret);
        return ret;
    }

    speakerClient_ = std::make_shared<DSpeakerClient>(devId_, shared_from_this());
    ret = speakerClient_->SetUp(audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("Setup speaker failed.");
        NotifySourceDev(NOTIFY_OPEN_SPEAKER_RESULT, spkDhId_, ERR_DH_AUDIO_FAILED);
        return ERR_DH_AUDIO_FAILED;
    }

    NotifySourceDev(NOTIFY_OPEN_SPEAKER_RESULT, spkDhId_, DH_SUCCESS);
    DHLOGI("Open speaker device task excute success.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskCloseDSpeaker(const std::string &args)
{
    (void)args;
    DHLOGI("Close speaker device.");
    if (speakerClient_ == nullptr) {
        DHLOGI("Speaker client is null or already closed.");
        NotifySourceDev(NOTIFY_CLOSE_SPEAKER_RESULT, spkDhId_, DH_SUCCESS);
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = speakerClient_->StopRender();
    if (ret != DH_SUCCESS) {
        DHLOGE("Stop speaker client failed.");
        closeStatus = false;
    }
    ret = speakerClient_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Release speaker client failed.");
        closeStatus = false;
    }
    speakerClient_ = nullptr;

    if (closeStatus) {
        NotifySourceDev(NOTIFY_CLOSE_SPEAKER_RESULT, spkDhId_, DH_SUCCESS);
    } else {
        NotifySourceDev(NOTIFY_CLOSE_SPEAKER_RESULT, spkDhId_, ERR_DH_AUDIO_FAILED);
    }
    DHLOGI("Close speaker device task excute success.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskOpenDMic(const std::string &args)
{
    DHLOGI("Open mic device.");
    json jParam = json::parse(args, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_DH_ID, KEY_AUDIO_PARAM })) {
        return ERR_DH_AUDIO_FAILED;
    }
    micDhId_ = jParam[KEY_DH_ID];
    AudioParam audioParam;
    int32_t ret = from_json(jParam[KEY_AUDIO_PARAM], audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("Get audio param from json failed, error code %d.", ret);
        return ret;
    }

    micClient_ = std::make_shared<DMicClient>(devId_, shared_from_this());
    ret = micClient_->SetUp(audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("SetUp mic failed.");
        NotifySourceDev(NOTIFY_OPEN_MIC_RESULT, micDhId_, ERR_DH_AUDIO_FAILED);
        return ERR_DH_AUDIO_FAILED;
    }
    ret = micClient_->StartCapture();
    if (ret != DH_SUCCESS) {
        DHLOGE("Start capture failed.");
        NotifySourceDev(NOTIFY_OPEN_MIC_RESULT, micDhId_, ERR_DH_AUDIO_FAILED);
        return ret;
    }

    NotifySourceDev(NOTIFY_OPEN_MIC_RESULT, micDhId_, DH_SUCCESS);
    DHLOGI("Open mic device task excute success.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskCloseDMic(const std::string &args)
{
    (void)args;
    DHLOGI("Close mic device.");
    if (micClient_ == nullptr) {
        DHLOGI("mic client is null or already closed.");
        NotifySourceDev(NOTIFY_CLOSE_MIC_RESULT, micDhId_, DH_SUCCESS);
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = micClient_->StopCapture();
    if (ret != DH_SUCCESS) {
        DHLOGE("Stop mic client failed.");
        closeStatus = false;
    }
    ret = micClient_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Release mic client failed.");
        closeStatus = false;
    }
    micClient_ = nullptr;

    if (closeStatus) {
        NotifySourceDev(NOTIFY_CLOSE_MIC_RESULT, micDhId_, DH_SUCCESS);
    } else {
        NotifySourceDev(NOTIFY_CLOSE_MIC_RESULT, micDhId_, ERR_DH_AUDIO_FAILED);
    }
    DHLOGI("Close mic device task excute success.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskSetParameter(const std::string &args)
{
    DHLOGI("Set audio param.");
    auto event = std::make_shared<AudioEvent>(AudioEventType::EVENT_UNKNOWN, args);

    if (speakerClient_ == nullptr) {
        return ERR_DH_AUDIO_SA_SPEAKER_CLIENT_NOT_INIT;
    }
    return speakerClient_->SetAudioParameters(event);
}

int32_t DAudioSinkDev::TaskSetVolume(const std::string &args)
{
    DHLOGI("Set audio volume.");
    if (speakerClient_ == nullptr) {
        DHLOGE("Speaker client already closed.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto event = std::make_shared<AudioEvent>(AudioEventType::VOLUME_SET, args);
    int32_t ret = speakerClient_->SetAudioParameters(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("Volume set failed.");
        return ret;
    }
    DHLOGI("Set audio volume success.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskSetMute(const std::string &args)
{
    DHLOGI("Set audio mute.");
    if (speakerClient_ == nullptr) {
        DHLOGE("Speaker client already closed.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto event = std::make_shared<AudioEvent>(AudioEventType::VOLUME_MUTE_SET, args);
    int32_t ret = speakerClient_->SetMute(event);
    if (ret != DH_SUCCESS) {
        DHLOGE("Set mute failed.");
        return ret;
    }
    DHLOGI("Set mute success.");
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::TaskVolumeChange(const std::string &args)
{
    DHLOGI("Audio volume changed.");
    auto event = std::make_shared<AudioEvent>(AudioEventType::VOLUME_CHANGE, args);
    if (audioCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return audioCtrlMgr_->SendAudioEvent(event);
}

int32_t DAudioSinkDev::TaskFocusChange(const std::string &args)
{
    DHLOGI("Audio focus changed.");
    auto event = std::make_shared<AudioEvent>(AudioEventType::AUDIO_FOCUS_CHANGE, args);
    if (audioCtrlMgr_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }
    return audioCtrlMgr_->SendAudioEvent(event);
}

int32_t DAudioSinkDev::TaskRenderStateChange(const std::string &args)
{
    DHLOGI("Audio render state changed.");
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
    DHLOGI("On rask result. resultCode: %d, funcName: %s", resultCode, funcName.c_str());
}

void DAudioSinkDev::NotifySourceDev(const AudioEventType type, const std::string dhId, const int32_t result)
{
    json jEvent;
    jEvent[KEY_DH_ID] = dhId;
    jEvent[KEY_RESULT] = result;
    jEvent[KEY_EVENT_TYPE] = type;
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
        DHLOGE("Json parameter is invalid.");
        return false;
    }
    for (auto it = key.begin(); it != key.end(); it++) {
        if (!jParam.contains(*it)) {
            DHLOGE("Json parameter not contain param(%s).", (*it).c_str());
            return false;
        }
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
