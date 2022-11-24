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

#include "daudio_source_dev.h"

#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "daudio_source_manager.h"
#include "daudio_util.h"
#include "task_impl.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSourceDev"

namespace OHOS {
namespace DistributedHardware {
DAudioSourceDev::DAudioSourceDev(const std::string &devId, const std::shared_ptr<DAudioSourceMgrCallback> &callback)
    : devId_(devId), mgrCallback_(callback)
{
    memberFuncMap_[OPEN_SPEAKER] = &DAudioSourceDev::HandleOpenDSpeaker;
    memberFuncMap_[CLOSE_SPEAKER] = &DAudioSourceDev::HandleCloseDSpeaker;
    memberFuncMap_[SPEAKER_OPENED] = &DAudioSourceDev::HandleDSpeakerOpened;
    memberFuncMap_[SPEAKER_CLOSED] = &DAudioSourceDev::HandleDSpeakerClosed;
    memberFuncMap_[NOTIFY_OPEN_SPEAKER_RESULT] = &DAudioSourceDev::HandleNotifyRPC;
    memberFuncMap_[NOTIFY_CLOSE_SPEAKER_RESULT] = &DAudioSourceDev::HandleNotifyRPC;
    memberFuncMap_[OPEN_MIC] = &DAudioSourceDev::HandleOpenDMic;
    memberFuncMap_[CLOSE_MIC] = &DAudioSourceDev::HandleCloseDMic;
    memberFuncMap_[MIC_OPENED] = &DAudioSourceDev::HandleDMicOpened;
    memberFuncMap_[MIC_CLOSED] = &DAudioSourceDev::HandleDMicClosed;
    memberFuncMap_[NOTIFY_OPEN_MIC_RESULT] = &DAudioSourceDev::HandleNotifyRPC;
    memberFuncMap_[NOTIFY_CLOSE_MIC_RESULT] = &DAudioSourceDev::HandleNotifyRPC;
    memberFuncMap_[CTRL_CLOSED] = &DAudioSourceDev::HandleCtrlTransClosed;
    memberFuncMap_[VOLUME_SET] = &DAudioSourceDev::HandleVolumeSet;
    memberFuncMap_[VOLUME_MUTE_SET] = &DAudioSourceDev::HandleVolumeSet;
    memberFuncMap_[VOLUME_CHANGE] = &DAudioSourceDev::HandleVolumeChange;
    memberFuncMap_[AUDIO_FOCUS_CHANGE] = &DAudioSourceDev::HandleFocusChange;
    memberFuncMap_[AUDIO_RENDER_STATE_CHANGE] = &DAudioSourceDev::HandleRenderStateChange;
    memberFuncMap_[CHANGE_PLAY_STATUS] = &DAudioSourceDev::HandlePlayStatusChange;

    eventNotifyMap_[NOTIFY_OPEN_SPEAKER_RESULT] = EVENT_NOTIFY_OPEN_SPK;
    eventNotifyMap_[NOTIFY_CLOSE_SPEAKER_RESULT] = EVENT_NOTIFY_CLOSE_SPK;
    eventNotifyMap_[NOTIFY_OPEN_MIC_RESULT] = EVENT_NOTIFY_OPEN_MIC;
    eventNotifyMap_[NOTIFY_CLOSE_MIC_RESULT] = EVENT_NOTIFY_CLOSE_MIC;
}

int32_t DAudioSourceDev::AwakeAudioDev()
{
    constexpr size_t capacity = 20;
    taskQueue_ = std::make_shared<TaskQueue>(capacity);
    taskQueue_->Start();
    return DH_SUCCESS;
}

void DAudioSourceDev::SleepAudioDev()
{
    taskQueue_->Stop();
    taskQueue_ = nullptr;
}

int32_t DAudioSourceDev::EnableDAudio(const std::string &dhId, const std::string &attrs)
{
    DHLOGI("Enable audio device, dhId: %s.", dhId.c_str());
    json jParam = { { KEY_DEV_ID, devId_ }, { KEY_DH_ID, dhId }, { KEY_ATTRS, attrs } };
    auto task =
        GenerateTask(this, &DAudioSourceDev::TaskEnableDAudio, jParam.dump(), "", &DAudioSourceDev::OnEnableTaskResult);
    DHLOGD("Enable audio task generate success.");
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::DisableDAudio(const std::string &dhId)
{
    DHLOGI("Disable audio device, dhId: %s.", dhId.c_str());
    json jParam = { { KEY_DEV_ID, devId_ }, { KEY_DH_ID, dhId } };
    auto task = GenerateTask(this, &DAudioSourceDev::TaskDisableDAudio, jParam.dump(), "",
        &DAudioSourceDev::OnDisableTaskResult);
    DHLOGD("Disable audio task generate success.");
    return taskQueue_->Produce(task);
}

void DAudioSourceDev::NotifyEvent(const AudioEvent &event)
{
    DHLOGI("Notify event, eventType: %d.", event.type);
    std::map<AudioEventType, DAudioSourceDevFunc>::iterator iter = memberFuncMap_.find(event.type);
    if (iter == memberFuncMap_.end()) {
        DHLOGE("Invalid eventType.");
        return;
    }
    DAudioSourceDevFunc &func = iter->second;
    (this->*func)(event);
}

int32_t DAudioSourceDev::HandleOpenDSpeaker(const AudioEvent &event)
{
    DHLOGI("Open speaker device.");
    if (taskQueue_ == nullptr) {
        DHLOGE("Task queue is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSourceDev::TaskOpenDSpeaker, event.content, "Open Spk Device",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseDSpeaker(const AudioEvent &event)
{
    DHLOGI("Close speaker device.");
    if (taskQueue_ == nullptr) {
        DHLOGE("Task queue is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseDSpeaker, event.content, "Close Spk Device",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleDSpeakerOpened(const AudioEvent &event)
{
    (void)event;
    DHLOGI("Speaker device opened.");
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleDSpeakerClosed(const AudioEvent &event)
{
    DHLOGI("Speaker device closed.");
    if (speaker_ == nullptr) {
        DHLOGE("Speaker already closed.");
        return DH_SUCCESS;
    }
    return speaker_->NotifyHdfAudioEvent(event);
}

int32_t DAudioSourceDev::HandleOpenDMic(const AudioEvent &event)
{
    DHLOGI("Open mic device.");
    if (taskQueue_ == nullptr) {
        DHLOGE("Task queue is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    auto task = GenerateTask(this, &DAudioSourceDev::TaskOpenDMic, event.content, "Open Mic Device",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseDMic(const AudioEvent &event)
{
    DHLOGI("Close mic device.");
    if (taskQueue_ == nullptr) {
        DHLOGE("Task queue is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }

    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseDMic, event.content, "Close Mic Device",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleDMicOpened(const AudioEvent &event)
{
    (void)event;
    DHLOGI("Mic device opened.");
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleDMicClosed(const AudioEvent &event)
{
    DHLOGI("Mic device closed.");
    if (mic_ == nullptr) {
        DHLOGE("Mic already closed.");
        return DH_SUCCESS;
    }
    return mic_->NotifyHdfAudioEvent(event);
}

int32_t DAudioSourceDev::HandleCtrlTransClosed(const AudioEvent &event)
{
    DHLOGI("Control trans closed.");
    AudioEvent audioEvent = event;
    if (speaker_->IsOpened()) {
        audioEvent.type = SPEAKER_CLOSED;
        HandleCloseDSpeaker(audioEvent);
        HandleDSpeakerClosed(audioEvent);
    }
    if (mic_->IsOpened()) {
        audioEvent.type = MIC_CLOSED;
        HandleCloseDMic(audioEvent);
        HandleDMicClosed(audioEvent);
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleNotifyRPC(const AudioEvent &event)
{
    std::lock_guard<std::mutex> dataLock(rpcWaitMutex_);
    if (event.content.length() > DAUDIO_MAX_JSON_LEN || event.content.empty()) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    json jParam = json::parse(event.content, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_RESULT })) {
        return ERR_DH_AUDIO_FAILED;
    }

    rpcResult_ = (jParam[KEY_RESULT] == DH_SUCCESS) ? true : false;
    DHLOGI("Notify RPC event: %d, result: %d.", (int32_t)event.type, (int32_t)rpcResult_);
    DHLOGI("Notify event, eventType: %d.", event.type);
    std::map<AudioEventType, uint8_t>::iterator iter = eventNotifyMap_.find(event.type);
    if (iter == eventNotifyMap_.end()) {
        DHLOGE("Invalid eventType.");
        return ERR_DH_AUDIO_NOT_FOUND_KEY;
    }
    rpcNotify_ = iter->second;
    rpcWaitCond_.notify_all();
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::HandleVolumeSet(const AudioEvent &event)
{
    DHLOGI("Start handle volume set.");
    auto task = GenerateTask(this, &DAudioSourceDev::TaskSetVolume, event.content, "set volume",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleVolumeChange(const AudioEvent &event)
{
    DHLOGI("Start handle volume change.");
    auto task = GenerateTask(this, &DAudioSourceDev::TaskChangeVolume, event.content, "volume change",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleFocusChange(const AudioEvent &event)
{
    DHLOGI("Start handle focus change.");
    auto task = GenerateTask(this, &DAudioSourceDev::TaskChangeFocus, event.content, "focus change",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleRenderStateChange(const AudioEvent &event)
{
    DHLOGI("Start handle render state change.");
    auto task = GenerateTask(this, &DAudioSourceDev::TaskChangeRenderState, event.content, "render state change",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandlePlayStatusChange(const AudioEvent &event)
{
    DHLOGI("Play status change, content: %s.", event.content.c_str());
    if (audioCtrlMgr_ == nullptr) {
        DHLOGE("Audio ctrl mgr not init.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    AudioEvent audioEvent(CHANGE_PLAY_STATUS, event.content);
    int32_t ret = audioCtrlMgr_->SendAudioEvent(audioEvent);
    if (ret != DH_SUCCESS) {
        DHLOGE("Play status change failed.");
    }

    if (event.content == AUDIO_EVENT_RESTART) {
        ret = speaker_->Restart();
        if (ret != DH_SUCCESS) {
            DHLOGE("Speaker restart  failed.");
        }
        return ret;
    } else if (event.content == AUDIO_EVENT_PAUSE) {
        ret = speaker_->Pause();
        if (ret != DH_SUCCESS) {
            DHLOGE("Speaker Pause failed.");
        }
        return ret;
    } else {
        DHLOGE("Play status error.");
        return ERR_DH_AUDIO_FAILED;
    }
}

int32_t DAudioSourceDev::WaitForRPC(const AudioEventType type)
{
    std::unique_lock<std::mutex> lck(rpcWaitMutex_);
    DHLOGI("Wait sink device notify type: %d.", type);
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
        DHLOGE("RPC notify wait timeout(%ds).", RPC_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_RPC_WAIT_TIMEOUT;
    }
    if (!rpcResult_) {
        DHLOGI("RPC notify Result Failed.");
        return ERR_DH_AUDIO_FAILED;
    }
    DHLOGI("Receive sink device notify type: %d.", type);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskEnableDAudio(const std::string &args)
{
    DHLOGI("Enable audio device.");
    if (args.length() > DAUDIO_MAX_JSON_LEN || args.empty()) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    json jParam = json::parse(args, nullptr, false);
    if (!CheckIsNum((std::string)jParam[KEY_DH_ID])) {
        return ERR_DH_AUDIO_NOT_SUPPORT;
    }
    int32_t dhId = std::stoi((std::string)jParam[KEY_DH_ID]);

    switch (GetDevTypeByDHId(dhId)) {
        case AUDIO_DEVICE_TYPE_SPEAKER:
            return EnableDSpeaker(dhId, jParam[KEY_ATTRS]);
        case AUDIO_DEVICE_TYPE_MIC:
            return EnableDMic(dhId, jParam[KEY_ATTRS]);
        case AUDIO_DEVICE_TYPE_UNKNOWN:
        default:
            DHLOGE("Unknown audio device.");
            return ERR_DH_AUDIO_NOT_SUPPORT;
    }
}

int32_t DAudioSourceDev::EnableDSpeaker(const int32_t dhId, const std::string &attrs)
{
    if (speaker_ == nullptr) {
        DHLOGI("Create new speaker device.");
        speaker_ = std::make_shared<DSpeakerDev>(devId_, shared_from_this());
    }
    DAUDIO_SYNC_TRACE(DAUDIO_ENABLE_SPK);
    return speaker_->EnableDSpeaker(dhId, attrs);
}

int32_t DAudioSourceDev::EnableDMic(const int32_t dhId, const std::string &attrs)
{
    if (mic_ == nullptr) {
        DHLOGI("Create new mic device.");
        mic_ = std::make_shared<DMicDev>(devId_, shared_from_this());
    }
    DAUDIO_SYNC_TRACE(DAUDIO_ENABLE_MIC);
    return mic_->EnableDMic(dhId, attrs);
}

void DAudioSourceDev::OnEnableTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName)
{
    (void)funcName;
    DHLOGI("On enable task result.");
    if (result.length() > DAUDIO_MAX_JSON_LEN || result.empty()) {
        return;
    }
    json jParam = json::parse(result, nullptr, false);
    mgrCallback_->OnEnableAudioResult(jParam[KEY_DEV_ID], jParam[KEY_DH_ID], resultCode);
}

int32_t DAudioSourceDev::TaskDisableDAudio(const std::string &args)
{
    DHLOGI("Task disable daudio.");
    if (args.length() > DAUDIO_MAX_JSON_LEN || args.empty()) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    json jParam = json::parse(args, nullptr, false);
    if (!CheckIsNum((std::string)jParam[KEY_DH_ID])) {
        return ERR_DH_AUDIO_NOT_SUPPORT;
    }
    int32_t dhId = std::stoi((std::string)jParam[KEY_DH_ID]);
    switch (GetDevTypeByDHId(dhId)) {
        case AUDIO_DEVICE_TYPE_SPEAKER:
            return DisableDSpeaker(dhId);
        case AUDIO_DEVICE_TYPE_MIC:
            return DisableDMic(dhId);
        case AUDIO_DEVICE_TYPE_UNKNOWN:
        default:
            DHLOGE("Unknown audio device.");
            return ERR_DH_AUDIO_NOT_SUPPORT;
    }
}

int32_t DAudioSourceDev::DisableDSpeaker(const int32_t dhId)
{
    if (speaker_ == nullptr) {
        DHLOGE("Speaker device is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    DAUDIO_SYNC_TRACE(DAUDIO_DISABLE_SPK);
    return speaker_->DisableDSpeaker(dhId);
}

int32_t DAudioSourceDev::DisableDMic(const int32_t dhId)
{
    if (mic_ == nullptr) {
        DHLOGE("Mic device is null.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    DAUDIO_SYNC_TRACE(DAUDIO_DISABLE_MIC);
    return mic_->DisableDMic(dhId);
}

void DAudioSourceDev::OnDisableTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName)
{
    (void)funcName;
    DHLOGI("On disable task result.");
    if (result.length() > DAUDIO_MAX_JSON_LEN || result.empty()) {
        return;
    }
    json jParam = json::parse(result, nullptr, false);
    mgrCallback_->OnDisableAudioResult(jParam[KEY_DEV_ID], jParam[KEY_DH_ID], resultCode);
}

int32_t DAudioSourceDev::TaskOpenDSpeaker(const std::string &args)
{
    DHLOGI("Task open speaker args: %s.", args.c_str());
    if (speaker_ == nullptr) {
        DHLOGE("Speaker device not init");
        return ERR_DH_AUDIO_SA_SPEAKER_DEVICE_NOT_INIT;
    }
    if (args.length() > DAUDIO_MAX_JSON_LEN || args.empty()) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    json jParam = json::parse(args, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_DH_ID })) {
        return ERR_DH_AUDIO_FAILED;
    }
    json jAudioParam;
    to_json(jAudioParam, speaker_->GetAudioParam());
    int32_t ret = NotifySinkDev(OPEN_SPEAKER, jAudioParam, jParam[KEY_DH_ID]);
    if (ret != DH_SUCCESS) {
        DHLOGE("Notify sink open speaker failed, error code %d.", ret);
        return ret;
    }

    ret = OpenCtrlChannel();
    if (ret != DH_SUCCESS) {
        DHLOGE("DSpeaker set up failed, error code %d.", ret);
        return ret;
    }

    ret = speaker_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("Speaker setup failed, error code %d.", ret);
        return ret;
    }
    ret = speaker_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("Speaker start failed, error code %d.", ret);
        speaker_->Stop();
        speaker_->Release();
        return ret;
    }
    DHLOGI("Open speaker success.");
    NotifyHDF(NOTIFY_OPEN_SPEAKER_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseDSpeaker(const std::string &args)
{
    DHLOGI("Task close speaker, args: %s.", args.c_str());
    if (speaker_ == nullptr) {
        DHLOGI("Speaker already closed.");
        NotifyHDF(NOTIFY_CLOSE_SPEAKER_RESULT, HDF_EVENT_RESULT_SUCCESS);
        return DH_SUCCESS;
    }
    if (args.length() > DAUDIO_MAX_JSON_LEN || args.empty()) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }

    bool closeStatus = true;
    int32_t ret = speaker_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("Speaker stop failed, error code %d.", ret);
        closeStatus = false;
    }
    ret = speaker_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Speaker release failed, error code %d.", ret);
        closeStatus = false;
    }

    ret = CloseCtrlChannel(CAP_SPK);
    if (ret != DH_SUCCESS) {
        DHLOGE("Close ctrl channel failed, error code %d", ret);
        closeStatus = false;
    }

    if (!speaker_->IsOpened()) {
        json jAudioParam;
        json jParam = json::parse(args, nullptr, false);
        if (!JsonParamCheck(jParam, { KEY_DH_ID })) {
            return ERR_DH_AUDIO_FAILED;
        }
        NotifySinkDev(CLOSE_SPEAKER, jAudioParam, jParam[KEY_DH_ID]);
    }

    if (!closeStatus) {
        return ERR_DH_AUDIO_FAILED;
    }
    NotifyHDF(NOTIFY_CLOSE_SPEAKER_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskOpenDMic(const std::string &args)
{
    DHLOGI("Task open mic, args: %s.", args.c_str());
    if (mic_ == nullptr) {
        DHLOGE("Mic device not init");
        return ERR_DH_AUDIO_SA_MIC_DEVICE_NOT_INIT;
    }
    if (args.length() > DAUDIO_MAX_JSON_LEN || args.empty()) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }
    json jParam = json::parse(args, nullptr, false);
    if (!JsonParamCheck(jParam, { KEY_DH_ID })) {
        return ERR_DH_AUDIO_FAILED;
    }
    json jAudioParam;
    to_json(jAudioParam, mic_->GetAudioParam());
    int32_t ret = NotifySinkDev(OPEN_MIC, jAudioParam, jParam[KEY_DH_ID]);
    if (ret != DH_SUCCESS) {
        DHLOGE("Notify sink open mic failed, error code %d.", ret);
        mic_->Release();
        return ret;
    }

    ret = OpenCtrlChannel();
    if (ret != DH_SUCCESS) {
        DHLOGE("DSpeaker set up failed, error code %d.", ret);
        return ret;
    }

    ret = mic_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("Mic setup failed, error code %d.", ret);
        return ret;
    }
    ret = mic_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("Mic start failed, error code %d.", ret);
        mic_->Stop();
        mic_->Release();
        return ret;
    }
    DHLOGI("Open mic success.");
    NotifyHDF(NOTIFY_OPEN_MIC_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseDMic(const std::string &args)
{
    DHLOGI("Task close mic, args: %s.", args.c_str());
    if (mic_ == nullptr) {
        DHLOGE("Mic device already closed.");
        NotifyHDF(NOTIFY_CLOSE_MIC_RESULT, HDF_EVENT_RESULT_SUCCESS);
        return DH_SUCCESS;
    }
    if (args.length() > DAUDIO_MAX_JSON_LEN || args.empty()) {
        return ERR_DH_AUDIO_SA_PARAM_INVALID;
    }

    bool closeStatus = true;
    int32_t ret = mic_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("Mic stop failed, error code %d", ret);
        closeStatus = false;
    }
    ret = mic_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Mic release failed, error code %d", ret);
        closeStatus = false;
    }

    ret = CloseCtrlChannel(CAP_MIC);
    if (ret != DH_SUCCESS) {
        DHLOGE("Close ctrl channel failed, error code %d", ret);
        closeStatus = false;
    }

    if (!mic_->IsOpened()) {
        json jAudioParam;
        json jParam = json::parse(args, nullptr, false);
        if (!JsonParamCheck(jParam, { KEY_DH_ID })) {
            return ERR_DH_AUDIO_FAILED;
        }
        NotifySinkDev(CLOSE_MIC, jAudioParam, jParam[KEY_DH_ID]);
    }

    if (!closeStatus) {
        return ERR_DH_AUDIO_FAILED;
    }
    NotifyHDF(NOTIFY_CLOSE_MIC_RESULT, HDF_EVENT_RESULT_SUCCESS);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskSetVolume(const std::string &args)
{
    DHLOGI("Task set volume, args: %s.", args.c_str());
    if (audioCtrlMgr_ == nullptr) {
        DHLOGE("Audio ctrl mgr not init.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    AudioEvent event(getEventTypeFromArgs(args), args);
    return audioCtrlMgr_->SendAudioEvent(event);
}

int32_t DAudioSourceDev::TaskChangeVolume(const std::string &args)
{
    DHLOGI("Task change volume, args: %s.", args.c_str());
    return NotifyHDF(AudioEventType::VOLUME_CHANGE, args);
}

int32_t DAudioSourceDev::TaskChangeFocus(const std::string &args)
{
    DHLOGI("Task change focus, args: %s.", args.c_str());
    return NotifyHDF(AudioEventType::AUDIO_FOCUS_CHANGE, args);
}

int32_t DAudioSourceDev::TaskChangeRenderState(const std::string &args)
{
    DHLOGI("Task change render state, args: %s.", args.c_str());
    return NotifyHDF(AudioEventType::AUDIO_RENDER_STATE_CHANGE, args);
}

void DAudioSourceDev::OnTaskResult(int32_t resultCode, const std::string &result, const std::string &funcName)
{
    (void)resultCode;
    (void)result;
    (void)funcName;
    DHLOGI("OnTaskResult. resultcode: %d, result: %s, funcName: %s", resultCode, result.c_str(),
        funcName.c_str());
}

int32_t DAudioSourceDev::OpenCtrlChannel()
{
    if (audioCtrlMgr_ == nullptr) {
        audioCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId_, shared_from_this());
    }
    if (audioCtrlMgr_->IsOpened()) {
        DHLOGI("Audio ctrl channel has been opened.");
        return DH_SUCCESS;
    }

    int32_t ret = audioCtrlMgr_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("Set up audio ctrl failed, error code %d.", ret);
        return ret;
    }

    ret = audioCtrlMgr_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("Start audio ctrl failed, error code %d.", ret);
        audioCtrlMgr_->Release();
        audioCtrlMgr_ = nullptr;
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::CloseCtrlChannel(const PortCapType capType)
{
    DHLOGI("Close ctrl channel, capType %d.", capType);
    if (audioCtrlMgr_ == nullptr || !audioCtrlMgr_->IsOpened()) {
        audioCtrlMgr_ = nullptr;
        DHLOGI("Audio source ctrl channel already closed.");
        return DH_SUCCESS;
    }

    if ((capType == CAP_MIC && speaker_->IsOpened()) || (capType == CAP_SPK && mic_->IsOpened())) {
        DHLOGI("Cannot close. Audio source ctrl channel is using.");
        return DH_SUCCESS;
    }

    bool closeStatus = true;
    int32_t ret = audioCtrlMgr_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("Stop ctrl channel failed, ret: %d.", ret);
        closeStatus = false;
    }
    ret = audioCtrlMgr_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Release ctrl channel failed, ret: %d.", ret);
        closeStatus = false;
    }
    audioCtrlMgr_ = nullptr;
    if (!closeStatus) {
        return ERR_DH_AUDIO_FAILED;
    }

    DHLOGI("Close ctrl channel success.");
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::NotifySinkDev(const AudioEventType type, const json Param, const std::string dhId)
{
    constexpr uint32_t eventOffset = 4;
    json jParam = { { KEY_DH_ID, dhId },
                    { KEY_EVENT_TYPE, type },
                    { KEY_AUDIO_PARAM, Param } };
    DAudioSourceManager::GetInstance().DAudioNotify(devId_, dhId, type, jParam.dump());
    return WaitForRPC(static_cast<AudioEventType>(static_cast<int32_t>(type) + eventOffset));
}

int32_t DAudioSourceDev::NotifyHDF(const AudioEventType type, const std::string result)
{
    AudioEvent event(type, result);
    switch (type) {
        case NOTIFY_OPEN_SPEAKER_RESULT:
        case NOTIFY_CLOSE_SPEAKER_RESULT:
        case VOLUME_CHANGE:
        case AUDIO_FOCUS_CHANGE:
        case AUDIO_RENDER_STATE_CHANGE:
            if (speaker_ == nullptr) {
                DHLOGE("Speaker device not init");
                return ERR_DH_AUDIO_NULLPTR;
            }
            return speaker_->NotifyHdfAudioEvent(event);
        case NOTIFY_OPEN_MIC_RESULT:
        case NOTIFY_CLOSE_MIC_RESULT:
            if (mic_ == nullptr) {
                DHLOGE("Mic device not init");
                return ERR_DH_AUDIO_NULLPTR;
            }
            return mic_->NotifyHdfAudioEvent(event);
        default:
            DHLOGE("NotifyHDF unknown type.");
            return ERR_DH_AUDIO_FAILED;
    }
    return DH_SUCCESS;
}

AudioEventType DAudioSourceDev::getEventTypeFromArgs(const std::string &args)
{
    std::string::size_type volume_mute_set = args.find(STREAM_MUTE_STATUS);
    if (volume_mute_set != std::string::npos) {
        return AudioEventType::VOLUME_MUTE_SET;
    }
    return AudioEventType::VOLUME_SET;
}

void DAudioSourceDev::to_json(json &j, const AudioParam &param)
{
    j = json {
        { KEY_SAMPLING_RATE, param.comParam.sampleRate },   { KEY_FORMAT, param.comParam.bitFormat },
        { KEY_CHANNELS, param.comParam.channelMask },       { KEY_CONTENT_TYPE, param.renderOpts.contentType },
        { KEY_STREAM_USAGE, param.renderOpts.streamUsage }, { KEY_SOURCE_TYPE, param.captureOpts.sourceType },
    };
}

bool DAudioSourceDev::JsonParamCheck(const json &jParam, const std::initializer_list<std::string> &key)
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

bool DAudioSourceDev::CheckIsNum(const std::string &jsonString)
{
    if (jsonString.empty() || jsonString.size() > MAX_KEY_DH_ID_LEN) {
        DHLOGE("Json string size %d, is zero or too long.", jsonString.size());
        return false;
    }
    for (char const &c : jsonString) {
        if (!std::isdigit(c)) {
            DHLOGE("Json string is not number.");
            return false;
        }
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS