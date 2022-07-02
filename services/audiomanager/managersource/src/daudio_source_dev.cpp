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
    DHLOGI("%s: NotifyEvent, eventType: %d.", LOG_TAG, event->type);
    switch (event->type) {
        case AudioEventType::OPEN_SPEAKER:
            HandleOpenDSpeaker(event);
            break;
        case AudioEventType::CLOSE_SPEAKER:
            HandleCloseDSpeaker(event);
            break;
        case AudioEventType::OPEN_MIC:
            HandleOpenDMic(event);
            break;
        case AudioEventType::CLOSE_MIC:
            HandleCloseDMic(event);
            break;
        case AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT:
            HandleNotifyRPC(event);
            break;
        case AudioEventType::NOTIFY_OPEN_MIC_RESULT:
            HandleNotifyRPC(event);
            break;
        case AudioEventType::NOTIFY_OPEN_CTRL_RESULT:
            HandleNotifyRPC(event);
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

int32_t DAudioSourceDev::HandleOpenDSpeaker(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Open speaker device.", LOG_TAG);
    if (audioSourceCtrlMgr_ == nullptr) {
        audioSourceCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId_, shared_from_this());
    }
    if (!audioSourceCtrlMgr_->IsOpened() && (HandleOpenCtrlTrans(event) != DH_SUCCESS)) {
        DHLOGE("%s: Open ctrl failed.");
        return ERR_DH_AUDIO_SA_OPEN_CTRL_FAILED;
    }

    auto task = GenerateTask(this, &DAudioSourceDev::TaskOpenDSpeaker, event->content, "OpenDSpeaker",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseDSpeaker(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Close speaker device.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseDSpeaker, event->content, "CloseDSpeaker",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleOpenDMic(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Open mic device.", LOG_TAG);
    if (audioSourceCtrlMgr_ == nullptr) {
        audioSourceCtrlMgr_ = std::make_shared<DAudioSourceDevCtrlMgr>(devId_, shared_from_this());
    }
    if (!audioSourceCtrlMgr_->IsOpened() && (HandleOpenCtrlTrans(event) != DH_SUCCESS)) {
        DHLOGE("%s: Open ctrl failed.");
        return ERR_DH_AUDIO_SA_OPEN_CTRL_FAILED;
    }

    auto task =
        GenerateTask(this, &DAudioSourceDev::TaskOpenDMic, event->content, "OpenDMic", &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseDMic(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Close mic device.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseDMic, event->content, "CloseDMic",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleOpenCtrlTrans(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Open control trans.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskOpenCtrlChannel, event->content, "OpenCtrlTrans",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleCloseCtrlTrans()
{
    DHLOGI("%s: Close control trans.", LOG_TAG);
    auto task = GenerateTask(this, &DAudioSourceDev::TaskCloseCtrlChannel, "", "CloseCtrlTrans",
        &DAudioSourceDev::OnTaskResult);
    return taskQueue_->Produce(task);
}

int32_t DAudioSourceDev::HandleNotifyRPC(const std::shared_ptr<AudioEvent> &event)
{
    std::lock_guard<std::mutex> dataLock(rpcWaitMutex_);
    json jParam = json::parse(event->content, nullptr, false);
    auto result = jParam["result"];
    rpcResult_ = false;
    if (result == DH_SUCCESS) {
        rpcResult_ = true;
    }

    switch (event->type) {
        case AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT:
            DHLOGI("%s: Notify RPC event: Open Speaker, result: %d.", LOG_TAG, result);
            rpcNotify_ = EVENT_NOTIFY_SPK;
            break;
        case AudioEventType::NOTIFY_OPEN_MIC_RESULT:
            DHLOGI("%s: Notify RPC event: Open Mic, result: %d.", LOG_TAG, result);
            rpcNotify_ = EVENT_NOTIFY_MIC;
            break;
        case AudioEventType::NOTIFY_OPEN_CTRL_RESULT:
            DHLOGI("%s: Notify RPC event: Open Ctrl, result: %d.", LOG_TAG, result);
            rpcNotify_ = EVENT_NOTIFY_CTRL;
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
}

int32_t DAudioSourceDev::WaitForRPC(const AudioEventType type)
{
    std::unique_lock<std::mutex> lck(rpcWaitMutex_);
    auto status = rpcWaitCond_.wait_for(lck, std::chrono::seconds(RPC_WAIT_SECONDS), [this, type]() {
        DHLOGI("%s: Wait RPC recive type: %d.", LOG_TAG, type);
        switch (type) {
            case AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_SPK;
            case AudioEventType::NOTIFY_OPEN_MIC_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_MIC;
            case AudioEventType::NOTIFY_OPEN_CTRL_RESULT:
                return rpcNotify_ == EVENT_NOTIFY_CTRL;
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
    return speaker_->EnableDSpeaker(dhId, attrs);
}

int32_t DAudioSourceDev::EnableDMic(const int32_t dhId, const std::string &attrs)
{
    if (mic_ == nullptr) {
        DHLOGI("%s: Create new mic device.", LOG_TAG);
        mic_ = std::make_shared<DMicDev>(devId_, shared_from_this());
    }
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
    return speaker_->DisableDSpeaker(dhId);
}

int32_t DAudioSourceDev::DisableDMic(const int32_t dhId)
{
    if (mic_ == nullptr) {
        DHLOGE("%s: Mic device is null.", LOG_TAG);
        return ERR_DH_AUDIO_NULLPTR;
    }
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
    to_json(jAudioParam, *(speaker_->GetAudioParam()));
    json jParam = json::parse(args, nullptr, false);
    jParam["eventType"] = AudioEventType::OPEN_SPEAKER;
    jParam["networkId"] = localDevId_;
    jParam["audioParam"] = jAudioParam;
    DAudioSourceManager::GetInstance().DAudioNotify(devId_, jParam["dhId"], AudioEventType::OPEN_SPEAKER,
        jParam.dump());

    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT;
    event->content = HDF_EVENT_RESULT_FAILED;
    int32_t ret = WaitForRPC(AudioEventType::NOTIFY_OPEN_SPEAKER_RESULT);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: TaskOpenDSpeaker waitForRPC Failed.", LOG_TAG);
        speaker_->NotifyHdfAudioEvent(event);
        return ret;
    }
    ret = speaker_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker setup failed.", LOG_TAG);
        speaker_->NotifyHdfAudioEvent(event);
        return ret;
    }
    ret = speaker_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker start failed.", LOG_TAG);
        speaker_->NotifyHdfAudioEvent(event);
        return ret;
    }
    event->content = HDF_EVENT_RESULT_SUCCESS;
    speaker_->NotifyHdfAudioEvent(event);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseDSpeaker(const std::string &args)
{
    DHLOGI("%s: TaskCloseDSpeaker. args: %s.", LOG_TAG, args.c_str());
    if (speaker_ == nullptr) {
        DHLOGE("%s: Speaker device not init", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_DEVICE_NOT_INIT;
    }

    int32_t ret = speaker_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker stop failed.", LOG_TAG);
    }
    ret = speaker_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker release failed.", LOG_TAG);
    }
    if (!speaker_->IsOpened() && !mic_->IsOpened()) {
        DHLOGI("%s: No distributed audio device used.", LOG_TAG);
        return HandleCloseCtrlTrans();
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskOpenDMic(const std::string &args)
{
    DHLOGI("%s: TaskOpenDMic. args: %s.", LOG_TAG, args.c_str());
    if (mic_ == nullptr) {
        DHLOGE("%s: Mic device not init", LOG_TAG);
        return ERR_DH_AUDIO_SA_MIC_DEVICE_NOT_INIT;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::NOTIFY_OPEN_MIC_RESULT;
    event->content = HDF_EVENT_RESULT_FAILED;
    int32_t ret = mic_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic setup failed.", LOG_TAG);
        mic_->NotifyHdfAudioEvent(event);
        return ret;
    }

    json jAudioParam;
    to_json(jAudioParam, *(mic_->GetAudioParam()));
    json jParam = json::parse(args, nullptr, false);
    jParam["eventType"] = AudioEventType::OPEN_MIC;
    jParam["networkId"] = localDevId_;
    jParam["audioParam"] = jAudioParam;
    DAudioSourceManager::GetInstance().DAudioNotify(devId_, jParam["dhId"], AudioEventType::OPEN_MIC, jParam.dump());
    ret = WaitForRPC(AudioEventType::NOTIFY_OPEN_MIC_RESULT);
    if (ret != DH_SUCCESS) {
        mic_->NotifyHdfAudioEvent(event);
        return ret;
    }
    ret = mic_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: speaker start failed.", LOG_TAG);
        mic_->NotifyHdfAudioEvent(event);
        return ret;
    }
    event->content = HDF_EVENT_RESULT_SUCCESS;
    mic_->NotifyHdfAudioEvent(event);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseDMic(const std::string &args)
{
    DHLOGI("%s: TaskCloseDMic. args: %s.", LOG_TAG, args.c_str());
    if (mic_ == nullptr) {
        DHLOGE("%s: Mic device not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_MIC_DEVICE_NOT_INIT;
    }

    int32_t ret = mic_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic stop failed.", LOG_TAG);
    }
    ret = mic_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic release failed.", LOG_TAG);
    }
    if (!speaker_->IsOpened() && !mic_->IsOpened()) {
        DHLOGI("%s: No distributed audio device used.", LOG_TAG);
        return HandleCloseCtrlTrans();
    }
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskOpenCtrlChannel(const std::string &args)
{
    DHLOGI("%s: TaskOpenCtrlChannel. args: %s.", LOG_TAG, args.c_str());
    if (audioSourceCtrlMgr_ == nullptr) {
        DHLOGE("%s: AudioSourceCtrlMgr not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SOURCECTRLMGR_NOT_INIT;
    }

    json jParam = json::parse(args, nullptr, false);
    jParam["eventType"] = AudioEventType::OPEN_CTRL;
    jParam["networkId"] = localDevId_;
    DAudioSourceManager::GetInstance().DAudioNotify(devId_, jParam["dhId"], AudioEventType::OPEN_CTRL, jParam.dump());
    int32_t ret = WaitForRPC(AudioEventType::NOTIFY_OPEN_CTRL_RESULT);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: TaskOpenCtrlChannel WaitForRPC Failed", LOG_TAG);
        return ret;
    }
    ret = audioSourceCtrlMgr_->SetUp();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Audio source ctrl manager setup failed.", LOG_TAG);
        return ret;
    }
    ret = audioSourceCtrlMgr_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Audio source ctrl manager start failed.", LOG_TAG);
        return ret;
    }

    DHLOGI("%s: TaskOpenCtrlChannel success.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSourceDev::TaskCloseCtrlChannel(const std::string &args)
{
    DHLOGI("%s: TaskCloseCtrlChannel. args: %s.", LOG_TAG, args.c_str());
    if (audioSourceCtrlMgr_ == nullptr) {
        DHLOGE("%s: Audio source ctrl magr not init.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SOURCECTRLMGR_NOT_INIT;
    }

    int32_t ret = audioSourceCtrlMgr_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Audio source ctrl manager stop failed.", LOG_TAG);
        return ret;
    }
    DHLOGI("%s: TaskCloseCtrlChannel success.", LOG_TAG);
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
