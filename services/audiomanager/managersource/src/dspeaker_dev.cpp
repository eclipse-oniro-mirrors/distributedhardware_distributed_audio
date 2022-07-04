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
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DSpeakerDev::EnableDSpeaker(const int32_t dhId, const std::string &capability)
{
    DHLOGI("%s: EnableDSpeaker dhId: %d.", LOG_TAG, dhId);
    if (enabledPorts_.empty()) {
        DHLOGI("%s: Enable default speaker device.", LOG_TAG);
        int32_t ret = DAudioHdiHandler::GetInstance().RegisterAudioDevice(devId_, PIN_OUT_DAUDIO_DEFAULT, capability,
            shared_from_this());
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Register default speaker device failed, ret: %d.", LOG_TAG, ret);
            return ret;
        }
        enabledPorts_.insert(PIN_OUT_DAUDIO_DEFAULT);
    }

    int32_t ret = DAudioHdiHandler::GetInstance().RegisterAudioDevice(devId_, dhId, capability, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Register audio device failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    enabledPorts_.insert(dhId);
    return ret;
}

int32_t DSpeakerDev::DisableDSpeaker(const int32_t dhId)
{
    DHLOGI("%s: DisableDSpeaker.", LOG_TAG);
    int32_t ret = DAudioHdiHandler::GetInstance().UnRegisterAudioDevice(devId_, dhId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: UnRegister audio device failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    enabledPorts_.erase(dhId);

    if (enabledPorts_.size() == SINGLE_ITEM && enabledPorts_.find(PIN_OUT_DAUDIO_DEFAULT) != enabledPorts_.end()) {
        int32_t ret = DAudioHdiHandler::GetInstance().UnRegisterAudioDevice(devId_, PIN_OUT_DAUDIO_DEFAULT);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: UnRegister default speaker device failed, ret: %d.", LOG_TAG, ret);
            return ret;
        }
    }
    enabledPorts_.erase(PIN_OUT_DAUDIO_DEFAULT);
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
    if (devId != devId_ || dhId != curPort_) {
        DHLOGE("%s: Device id or port id is wrong.", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    json jParam;
    jParam["dhId"] = std::to_string(dhId);
    event->type = AudioEventType::OPEN_SPEAKER;
    event->content = jParam.dump();
    cbObj->NotifyEvent(event);
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
    if (devId != devId_ || dhId != curPort_) {
        DHLOGE("%s: Device id or port id is wrong.", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::CLOSE_SPEAKER;
    cbObj->NotifyEvent(event);
    return DH_SUCCESS;
}

int32_t DSpeakerDev::SetParameters(const std::string &devId, const int32_t dhId, const AudioParamHDF &param)
{
    DHLOGI("%s: Set speaker parameters {samplerate: %d, channelmask: %d, format: %d, streamusage: %d, period: %d, "
        "framesize: %d, ext{%s}}.",
        LOG_TAG, param.sampleRate, param.channelMask, param.bitFormat, param.streamUsage, param.period, param.frameSize,
        param.ext.c_str());
    if (devId != devId_) {
        DHLOGE("%s: Device Id is wrong, set speaker parameters failed.", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }
    curPort_ = dhId;
    audioParamHDF_ = param;
    audioParamHDF_.bitFormat = SAMPLE_S16LE;
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
    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    audioEvent->type = event.type;
    audioEvent->content = event.content;
    cbObj->NotifyEvent(audioEvent);
    return DH_SUCCESS;
}

int32_t DSpeakerDev::SetUp()
{
    DHLOGI("%s: SetUp speaker device.", LOG_TAG);
    if (speakerTrans_ == nullptr) {
        speakerTrans_ = std::make_shared<AudioEncodeTransport>(devId_);
    }

    AudioParam param;
    param.comParam.sampleRate = audioParamHDF_.sampleRate;
    param.comParam.channelMask = audioParamHDF_.channelMask;
    param.comParam.bitFormat = audioParamHDF_.bitFormat;
    param.comParam.codecType = AudioCodecType::AUDIO_CODEC_AAC;
    int32_t ret = speakerTrans_->SetUp(param, param, shared_from_this(), "speaker");
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
        channelWaitCond_.wait_for(lck, std::chrono::seconds(CHANNEL_WAIT_SECONDS), [this]() { return isTransReady_; });
    if (!status) {
        DHLOGE("%s: Wait channel open timeout(%ds).", LOG_TAG, CHANNEL_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_SPEAKER_CHANNEL_WAIT_TIMEOUT;
    }
    isOpened_ = true;
    return DH_SUCCESS;
}

int32_t DSpeakerDev::Stop()
{
    DHLOGI("%s: Stop speaker device.", LOG_TAG);
    if (speakerTrans_ == nullptr) {
        DHLOGE("%s: Speaker trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_SPEAKER_TRANS_NULL;
    }

    int32_t ret = speakerTrans_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop speaker trans failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    isOpened_ = false;
    isTransReady_ = false;
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
    return isOpened_;
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
    DHLOGI("%s: WriteStreamData, dhId:%d", LOG_TAG, dhId);
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

std::shared_ptr<AudioParam> DSpeakerDev::GetAudioParam()
{
    std::shared_ptr<AudioParam> param = std::make_shared<AudioParam>();
    param->comParam.sampleRate = audioParamHDF_.sampleRate;
    param->comParam.channelMask = audioParamHDF_.channelMask;
    param->comParam.bitFormat = audioParamHDF_.bitFormat;
    param->renderOpts.contentType = CONTENT_TYPE_MUSIC;
    param->renderOpts.streamUsage = STREAM_USAGE_MEDIA;
    param->CaptureOpts.sourceType = SOURCE_TYPE_MIC;
    param->CaptureOpts.capturerFlags = 0;
    return param;
}

int32_t DSpeakerDev::NotifyHdfAudioEvent(std::shared_ptr<AudioEvent> &event)
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
    switch (type) {
        case AudioEventType::DATA_OPENED:
            isTransReady_ = true;
            channelWaitCond_.notify_all();
            break;
        case AudioEventType::DATA_CLOSED:
            isOpened_ = false;
            isTransReady_ = false;
            break;
        default:
            break;
    }
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS