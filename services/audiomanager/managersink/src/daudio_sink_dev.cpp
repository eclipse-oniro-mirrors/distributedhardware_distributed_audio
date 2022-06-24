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
DAudioSinkDev::DAudioSinkDev(const std::string &devId)
{
    DHLOGI("%s: Distributed audio sink device constructed.", LOG_TAG);
    devId_ = devId;
    taskQueue_ = std::make_shared<TaskQueue>(DAUDIO_MAX_TASKQUEUE_LEN);
    if (taskQueue_ == nullptr) {
        DHLOGE("%s: DAudioSinkDev:Create task queue failed.", LOG_TAG);
    } else {
        taskQueue_->Start();
    }
}

DAudioSinkDev::~DAudioSinkDev()
{
    DHLOGI("%s: Distributed audio sink device destructed.", LOG_TAG);
    dAudioSinkDevCtrlMgr_ = nullptr;
    speakerClient_ = nullptr;
    micClient_ = nullptr;
    taskQueue_ = nullptr;
}

void DAudioSinkDev::NotifyEvent(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyEvent.", LOG_TAG);
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
            NotifyCloseCtrlChannel();
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
        case SET_PARAM:
            NotifySetParam(audioEvent);
            break;
        case SEND_PARAM:
            NotifyParam(audioEvent);
            break;
        default:
            DHLOGE("%s: Unknown event type: %d", LOG_TAG, (int32_t)audioEvent->type);
    }
}

int32_t DAudioSinkDev::NotifyOpenCtrlChannel(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyOpenCtrlChannel.", LOG_TAG);
    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        dAudioSinkDevCtrlMgr_ = std::make_shared<DAudioSinkDevCtrlMgr>(devId_, shared_from_this());
    }
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    int32_t produceTaskRet = DH_SUCCESS;
    if (!dAudioSinkDevCtrlMgr_->IsOpened()) {
        std::string openCtrlTaskParam = audioEvent->content;
        std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::OpenCtrlChannelTask,
            openCtrlTaskParam, "Sink Open Ctrl", &DAudioSinkDev::OnTaskResult);
        produceTaskRet = taskQueue_->Produce(task);
    }
    return produceTaskRet;
}

int32_t DAudioSinkDev::NotifyCloseCtrlChannel()
{
    DHLOGI("%s: NotifyCloseCtrlChannel.", LOG_TAG);
    std::shared_ptr<TaskImplInterface> task =
        GenerateTask(this, &DAudioSinkDev::CloseCtrlChannelTask, "", "Sink Close Ctrl", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyOpenSpeaker(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyOpenDSpeaker.", LOG_TAG);
    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        dAudioSinkDevCtrlMgr_ = std::make_shared<DAudioSinkDevCtrlMgr>(devId_, shared_from_this());
    }
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::OpenDSpeakerTask, audioEvent->content,
        "Sink Open Speaker", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseSpeaker(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyCloseDSpeaker.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::CloseDSpeakerTask, audioEvent->content,
        "Sink Close Speaker", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySpeakerOpened(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void)audioEvent;
    DHLOGI("%s: NotifySpeakerOpened.", LOG_TAG);
    if (speakerClient_ == nullptr) {
        return ERR_DH_AUDIO_SA_SPEAKER_CLIENT_NOT_INIT;
    }
    int32_t ret = speakerClient_->StartRender();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: NotifySpeakerOpend start Render failed. ret: %d.", LOG_TAG, ret);
        return ret;
    }
    DHLOGI("%s: NotifySpeakerOpend start Render success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifySpeakerClosed(const std::shared_ptr<AudioEvent> &audioEvent)
{
    (void)audioEvent;
    DHLOGI("%s: NotifySpeakerOpened.", LOG_TAG);
    if (speakerClient_ == nullptr) {
        return ERR_DH_AUDIO_SA_SPEAKER_CLIENT_NOT_INIT;
    }
    int32_t ret = speakerClient_->StopRender();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: SpeakerClient stop failed. ret: %d", LOG_TAG, ret);
        return ret;
    }
    DHLOGI("%s: SpeakerClient stop success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::NotifyOpenMic(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyOpenDMic.", LOG_TAG);
    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        dAudioSinkDevCtrlMgr_ = std::make_shared<DAudioSinkDevCtrlMgr>(devId_, shared_from_this());
    }
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::OpenDMicTask, audioEvent->content,
        "Sink Open Mic", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyCloseMic(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyCloseDMic.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::CloseDMicTask, audioEvent->content,
        "Sink CloseMic", &DAudioSinkDev::OnTaskResult);

    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifySetParam(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifySetParam.", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::SetParameterTask, audioEvent->content,
        "Sink SetParam", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::NotifyParam(const std::shared_ptr<AudioEvent> &audioEvent)
{
    DHLOGI("%s: NotifyParam", LOG_TAG);
    if (audioEvent == nullptr) {
        DHLOGE("%s: audioEvent is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::shared_ptr<TaskImplInterface> task = GenerateTask(this, &DAudioSinkDev::SendParameterTask, audioEvent->content,
        "Sink NotifyParam", &DAudioSinkDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSinkDev::OpenCtrlChannelTask(const std::string &args)
{
    DHLOGI("%s: OpenCtrlChannelTask.", LOG_TAG);
    json resultJson = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(resultJson)) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }

    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        DHLOGE("%s:OpenCtrlChannelTask: audioSinkCtrlMgr not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }

    int32_t ret = dAudioSinkDevCtrlMgr_->Init();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:OpenCtrlChannelTask: audio sink ctrl manager init failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s: OpenCtrlChannelTask: audio sink ctrl manager init success.", LOG_TAG);

    ret = dAudioSinkDevCtrlMgr_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:OpenCtrlChannelTask: audio sink ctrl manager set up failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s:OpenCtrlChannelTask: audio sink ctrl manager set up success.", LOG_TAG);

    json eventContentJson;
    eventContentJson["eventType"] = AudioEventType::NOTIFY_OPEN_CTRL_RESULT;
    std::string devId;
    ret = GetLocalDeviceNetworkId(devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:OpenCtrlChannelTask: Get local network id failed.", LOG_TAG);
        return ret;
    }
    eventContentJson["devId"] = devId;
    eventContentJson["result"] = DH_SUCCESS;
    eventContentJson["dhId"] = resultJson["dhId"];
    std::string eventContent = eventContentJson.dump();
    DHLOGI("%s:OpenCtrlChannelTask: start to DAudioNotify.", LOG_TAG);
    DAudioSinkManager::GetInstance().DAudioNotify(devId_, eventContentJson["dhId"], eventContentJson["eventType"],
        eventContent);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::CloseCtrlChannelTask(const std::string &args)
{
    (void)args;
    DHLOGI("%s: CloseCtrlChannelTask", LOG_TAG);
    if (dAudioSinkDevCtrlMgr_ == nullptr) {
        DHLOGE("%s:CloseCtrlChannelTask: dAudioSinkDevCtrlMgr_ not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SINKCTRLMGR_NOT_INIT;
    }

    int32_t ret = dAudioSinkDevCtrlMgr_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:CloseCtrlChannelTask: audio sink ctrl manager stop failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s:CloseCtrlChannelTask: audio sink ctrl manager stop success.", LOG_TAG);

    ret = dAudioSinkDevCtrlMgr_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:CloseCtrlChannelTask: audio sink ctrl manager release failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s:CloseCtrlChannelTask: audio sink ctrl manager release success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::OpenDSpeakerTask(const std::string &args)
{
    DHLOGI("%s: OpenDSpeakerTask.", LOG_TAG);
    json resultJson = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(resultJson)) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }

    if (speakerClient_ == nullptr) {
        speakerClient_ = std::make_shared<DSpeakerClient>(devId_, shared_from_this());
    }

    AudioParam audioParam;
    from_json(resultJson["audioParam"], audioParam);

    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT;
    event->content = args;

    int32_t ret = speakerClient_->SetUp(audioParam, event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: OpenDSpeakerTask: speakerClient setup fail.", LOG_TAG);
        return ret;
    }

    json eventContentJson;
    eventContentJson["audioParam"] = resultJson["audioParam"];
    eventContentJson["eventType"] = AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT;
    std::string devId;
    ret = GetLocalDeviceNetworkId(devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get local network id failed.", LOG_TAG);
        return ret;
    }
    eventContentJson["devId"] = devId;
    eventContentJson["result"] = DH_SUCCESS;
    eventContentJson["dhId"] = resultJson["dhId"];
    std::string eventContent = eventContentJson.dump();
    DAudioSinkManager::GetInstance().DAudioNotify(devId_, eventContentJson["dhId"], eventContentJson["eventType"],
        eventContent);

    DHLOGI("%s: speakerClient start success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::CloseDSpeakerTask(const std::string &args)
{
    (void)args;
    DHLOGI("%s: CloseDSpeakerTask.", LOG_TAG);
    if (speakerClient_ == nullptr) {
        DHLOGE("%s: CloseDSpeakerTask: speaker client not init", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_CLIENT_NOT_INIT;
    }

    int32_t ret = speakerClient_->StopRender();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: CloseDSpeakerTask: failed", LOG_TAG);
        return ret;
    }

    speakerClient_ = nullptr;
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::OpenDMicTask(const std::string &args)
{
    DHLOGI("%s: OpenDMicTask.", LOG_TAG);
    json resultJson = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(resultJson)) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    if (micClient_ == nullptr) {
        micClient_ = std::make_shared<DMicClient>(devId_, shared_from_this());
    }

    AudioParam audioParam;
    from_json(resultJson["audioParam"], audioParam);

    int32_t ret = micClient_->SetUp(audioParam);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: OpenDMicTask: mic client setup failed.", LOG_TAG);
        return ret;
    }

    ret = micClient_->StartCapture();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: OpenDMicTask: mic client start failed.", LOG_TAG);
        return ret;
    }

    json eventContentJson;
    eventContentJson["audioParam"] = resultJson["audioParam"];
    eventContentJson["eventType"] = AudioEventType::NOTIFY_OPEN_MIC_RESULT;
    std::string devId;
    ret = GetLocalDeviceNetworkId(devId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Get local network id failed.", LOG_TAG);
        return ret;
    }
    eventContentJson["devId"] = devId;
    eventContentJson["result"] = DH_SUCCESS;
    eventContentJson["dhId"] = resultJson["dhId"];
    std::string eventContent = eventContentJson.dump();
    DAudioSinkManager::GetInstance().DAudioNotify(devId_, eventContentJson["dhId"], eventContentJson["eventType"],
        eventContent);
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::CloseDMicTask(const std::string &args)
{
    (void)args;
    DHLOGI("%s: CloseDMicTask", LOG_TAG);
    if (micClient_ == nullptr) {
        return ERR_DH_AUDIO_SA_MIC_CLIENT_NOT_INIT;
    }
    int32_t ret = micClient_->StopCapture();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: CloseDMicTask: mic StopCapture failed.", LOG_TAG);
        return ret;
    }

    micClient_ = nullptr;
    return DH_SUCCESS;
}

int32_t DAudioSinkDev::SetParameterTask(const std::string &args)
{
    DHLOGI("%s: SetParameterTask", LOG_TAG);
    json resultJson = json::parse(args, nullptr, false);
    if (!JudgeJsonValid(resultJson)) {
        return ERR_DH_AUDIO_SA_ENABLE_PARAM_INVALID;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::CTRL_OPENED;
    event->content = args;

    if (speakerClient_ == nullptr) {
        return ERR_DH_AUDIO_SA_SPEAKER_CLIENT_NOT_INIT;
    }
    return speakerClient_->SetAudioParameters(event);
}

int32_t DAudioSinkDev::SendParameterTask(const std::string &args)
{
    DHLOGI("%s: SendParameterTask", LOG_TAG);
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::CTRL_OPENED;
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
    DHLOGI("%s: OnTaskResult. resultCode: %d, result: %s, funcName: %s", LOG_TAG, resultCode, result.c_str(),
        funcName.c_str());
}

bool DAudioSinkDev::JudgeJsonValid(const json &resultJson)
{
    if (resultJson.is_discarded()) {
        DHLOGE("%s: JudgeJsonValid: result json is invalid", LOG_TAG);
        return false;
    }
    if (!resultJson.contains("dhId")) {
        DHLOGE("%s: JudgeJsonValid: result json is invalid not contains", LOG_TAG);
        return false;
    }
    return true;
}
void from_json(const json &j, AudioParam &audioParam)
{
    j.at("channels").get_to(audioParam.comParam.channelMask);
    j.at("contentType").get_to(audioParam.renderOpts.contentType);
    j.at("format").get_to(audioParam.comParam.bitFormat);
    j.at("samplingRate").get_to(audioParam.comParam.sampleRate);
    j.at("streamUsage").get_to(audioParam.renderOpts.streamUsage);
    j.at("sourceType").get_to(audioParam.CaptureOpts.sourceType);
}
} // namespace DistributedHardware
} // namespace OHOS
