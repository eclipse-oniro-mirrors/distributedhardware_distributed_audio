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

#include "dspeaker_dev.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "audio_encode_transport.h"
#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DSpeakerDev::EnableDSpeaker(const int32_t dhId, const std::string &capability)
{
    DHLOGI("%s: Enable speaker device dhId: %d.", LOG_TAG, dhId);
    if (enabledPorts_.empty()) {
        if (EnableDevice(PIN_OUT_DAUDIO_DEFAULT, capability) != DH_SUCCESS) {
            return ERR_DH_AUDIO_FAILED;
        }
    }
    int32_t ret = EnableDevice(dhId, capability);

    DaudioFinishAsyncTrace(DAUDIO_REGISTER_AUDIO, DAUDIO_REGISTER_AUDIO_TASKID);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUIDO_REGISTER, devId_, std::to_string(dhId),
        "daudio spk enable success.");
    return ret;
}

int32_t DSpeakerDev::EnableDevice(const int32_t dhId, const std::string &capability)
{
    int32_t ret = DAudioHdiHandler::GetInstance().RegisterAudioDevice(devId_, dhId, capability, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register speaker device failed, ret: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_REGISTER_FAIL, devId_, std::to_string(dhId), ret,
            "daudio register speaker device failed.");
        return ret;
    }
    enabledPorts_.insert(dhId);
    return DH_SUCCESS;
}

int32_t DSpeakerDev::DisableDSpeaker(const int32_t dhId)
{
    DHLOGI("%s: DisableDSpeaker.", LOG_TAG);
    if (dhId == curPort_) {
        isOpened_.store(false);
    }
    int32_t ret = DisableDevice(dhId);
    if (ret != DH_SUCCESS) {
        return ret;
    }
    if (enabledPorts_.size() == SINGLE_ITEM && enabledPorts_.find(PIN_OUT_DAUDIO_DEFAULT) != enabledPorts_.end()) {
        ret = DisableDevice(PIN_OUT_DAUDIO_DEFAULT);
        if (ret != DH_SUCCESS) {
            return ret;
        }
    }

    DaudioFinishAsyncTrace(DAUDIO_UNREGISTER_AUDIO, DAUDIO_UNREGISTER_AUDIO_TASKID);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_UNREGISTER, devId_, std::to_string(dhId),
        "daudio spk disable success.");
    return DH_SUCCESS;
}

int32_t DSpeakerDev::DisableDevice(const int32_t dhId)
{
    int32_t ret = DAudioHdiHandler::GetInstance().UnRegisterAudioDevice(devId_, dhId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: UnRegister speaker device failed, ret: %d.", LOG_TAG, ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_UNREGISTER_FAIL, devId_, std::to_string(dhId), ret,
            "daudio unregister speaker device failed.");
        return ret;
    }
    enabledPorts_.erase(dhId);
    return DH_SUCCESS;
}

int32_t DSpeakerDev::OpenDevice(const std::string &devId, const int32_t dhId)
{
    DHLOGI("%s: OpenDevice devId: %s, dhId: %d.", LOG_TAG, GetAnonyString(devId).c_str(), dhId);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("%s: Event callback is null", LOG_TAG);
        return ERR_DH_AUDIO_SA_EVENT_CALLBACK_NULL;
    }

    json jParam = { { KEY_DH_ID, std::to_string(dhId) } };
    auto event = std::make_shared<AudioEvent>(AudioEventType::OPEN_SPEAKER, jParam.dump());
    cbObj->NotifyEvent(event);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_OPEN, devId, std::to_string(dhId),
        "daudio spk device open success.");
    return DH_SUCCESS;
}

int32_t DSpeakerDev::CloseDevice(const std::string &devId, const int32_t dhId)
{
    DHLOGI("%s: CloseDevice devId: %s, dhId: %d.", LOG_TAG, GetAnonyString(devId).c_str(), dhId);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("%s: Event, callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_EVENT_CALLBACK_NULL;
    }

    json jParam = { { KEY_DH_ID, std::to_string(dhId) } };
    auto event = std::make_shared<AudioEvent>(AudioEventType::CLOSE_SPEAKER, jParam.dump());
    cbObj->NotifyEvent(event);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_CLOSE, devId, std::to_string(dhId),
        "daudio spk device close success.");
    curPort_ = 0;
    return DH_SUCCESS;
}

int32_t DSpeakerDev::SetParameters(const std::string &devId, const int32_t dhId, const AudioParamHDF &param)
{
    DHLOGI("%s: Set speaker parameters {samplerate: %d, channelmask: %d, format: %d, streamusage: %d, period: %d, "
        "framesize: %d, ext{%s}}.",
        LOG_TAG, param.sampleRate, param.channelMask, param.bitFormat, param.streamUsage, param.period, param.frameSize,
        param.ext.c_str());
    curPort_ = dhId;
    paramHDF_ = param;

    param_.comParam.sampleRate = paramHDF_.sampleRate;
    param_.comParam.channelMask = paramHDF_.channelMask;
    param_.comParam.bitFormat = paramHDF_.bitFormat;
    param_.comParam.codecType = AudioCodecType::AUDIO_CODEC_AAC;
    param_.renderOpts.contentType = CONTENT_TYPE_MUSIC;
    param_.renderOpts.streamUsage = paramHDF_.streamUsage;
    return DH_SUCCESS;
}

int32_t DSpeakerDev::NotifyEvent(const std::string &devId, int32_t dhId, const AudioEvent &event)
{
    DHLOGI("%s: Notify speaker event.", LOG_TAG);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("%s: Eventcallback is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_EVENT_CALLBACK_NULL;
    }
    auto audioEvent = std::make_shared<AudioEvent>(event.type, event.content);
    cbObj->NotifyEvent(audioEvent);
    return DH_SUCCESS;
}

int32_t DSpeakerDev::SetUp()
{
    DHLOGI("%s: SetUp speaker device.", LOG_TAG);
    if (speakerTrans_ == nullptr) {
        speakerTrans_ = std::make_shared<AudioEncodeTransport>(devId_);
    }

    int32_t ret = speakerTrans_->SetUp(param_, param_, shared_from_this(), "speaker");
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker trans set up failed. ret:%d", LOG_TAG, ret);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DSpeakerDev::Start()
{
    DHLOGI("%s: Start speaker device.", LOG_TAG);
    if (speakerTrans_ == nullptr) {
        DHLOGE("%s: Speaker trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_TRANS_NULL;
    }

    int32_t ret = speakerTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker trans start failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }

    std::unique_lock<std::mutex> lck(channelWaitMutex_);
    auto status =
        channelWaitCond_.wait_for(lck, std::chrono::seconds(CHANNEL_WAIT_SECONDS),
            [this]() { return isTransReady_.load(); });
    if (!status) {
        DHLOGE("%s: Wait channel open timeout(%ds).", LOG_TAG, CHANNEL_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_SPEAKER_CHANNEL_WAIT_TIMEOUT;
    }
    isOpened_.store(true);
    return DH_SUCCESS;
}

int32_t DSpeakerDev::Stop()
{
    DHLOGI("%s: Stop speaker device.", LOG_TAG);
    if (speakerTrans_ == nullptr) {
        DHLOGE("%s: Speaker trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_TRANS_NULL;
    }

    isOpened_.store(false);
    isTransReady_.store(false);
    int32_t ret = speakerTrans_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop speaker trans failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DSpeakerDev::Release()
{
    DHLOGI("%s: Release speaker device.", LOG_TAG);
    if (speakerTrans_ == nullptr) {
        DHLOGE("%s: Speaker trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_TRANS_NULL;
    }

    int32_t ret = speakerTrans_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: release speaker trans failed, ret: %d.", LOG_TAG, ret);
    }
    return DH_SUCCESS;
}

bool DSpeakerDev::IsOpened()
{
    return isOpened_.load();
}

int32_t DSpeakerDev::ReadStreamData(const std::string &devId, const int32_t dhId, std::shared_ptr<AudioData> &data)
{
    (void)devId;
    (void)dhId;
    (void)data;
    return DH_SUCCESS;
}

int32_t DSpeakerDev::WriteStreamData(const std::string &devId, const int32_t dhId, std::shared_ptr<AudioData> &data)
{
    DHLOGD("%s: WriteStreamData, dhId:%d", LOG_TAG, dhId);
    if (speakerTrans_ == nullptr) {
        DHLOGE("%s: ReadStreamData, speaker trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_TRANS_NULL;
    }
    int32_t ret = speakerTrans_->FeedAudioData(data);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: WriteStreamData failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    return DH_SUCCESS;
}

AudioParam DSpeakerDev::GetAudioParam() const
{
    return param_;
}

int32_t DSpeakerDev::NotifyHdfAudioEvent(const std::shared_ptr<AudioEvent> &event)
{
    int32_t ret = DAudioHdiHandler::GetInstance().NotifyEvent(devId_, curPort_, event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify event: %d, result: %s.", LOG_TAG, event->type, event->content.c_str());
    }
    return DH_SUCCESS;
}

int32_t DSpeakerDev::OnStateChange(int32_t type)
{
    DHLOGI("%s: On speaker device state change, type: %d.", LOG_TAG, type);
    auto event = std::make_shared<AudioEvent>();
    switch (type) {
        case AudioEventType::DATA_OPENED:
            isTransReady_.store(true);
            channelWaitCond_.notify_all();
            event->type = AudioEventType::SPEAKER_OPENED;
            break;
        case AudioEventType::DATA_CLOSED:
            isOpened_.store(false);
            isTransReady_.store(false);
            event->type = AudioEventType::SPEAKER_CLOSED;
            break;
        default:
            break;
    }
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("%s: Callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_MICCALLBACK_NULL;
    }
    cbObj->NotifyEvent(event);
    return DH_SUCCESS;
}

int32_t DSpeakerDev::WriteStreamBuffer(const std::shared_ptr<AudioData> &audioData)
{
    (void) audioData;
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS