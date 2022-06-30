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

#include "dmic_dev.h"

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "audio_decode_transport.h"
#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DMicDev::EnableDMic(const int32_t dhId, const std::string &capability)
{
    DHLOGI("%s: EnableDMic dhId: %d.", LOG_TAG, dhId);
    if (enabledPorts_.empty()) {
        DHLOGI("%s: Enable default mic device.", LOG_TAG);
        int32_t ret = DAudioHdiHandler::GetInstance().RegisterAudioDevice(devId_, PIN_IN_DAUDIO_DEFAULT, capability,
            shared_from_this());
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: Register default mic device failed, ret: %d.", LOG_TAG, ret);
            return ret;
        }
        enabledPorts_.insert(PIN_IN_DAUDIO_DEFAULT);
    }
    int32_t ret = DAudioHdiHandler::GetInstance().RegisterAudioDevice(devId_, dhId, capability, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: register audio device failed, ret: %d", LOG_TAG, ret);
        return ret;
    }
    enabledPorts_.insert(dhId);
    return DH_SUCCESS;
}

int32_t DMicDev::DisableDMic(const int32_t dhId)
{
    DHLOGI("%s: DisableDMic.", LOG_TAG);
    int32_t ret = DAudioHdiHandler::GetInstance().UnRegisterAudioDevice(devId_, dhId);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: unregister audio device failed, ret: %d", LOG_TAG, ret);
    }
    enabledPorts_.erase(dhId);

    if (enabledPorts_.size() == SINGLE_ITEM && enabledPorts_.find(PIN_IN_DAUDIO_DEFAULT) != enabledPorts_.end()) {
        int32_t ret = DAudioHdiHandler::GetInstance().UnRegisterAudioDevice(devId_, PIN_IN_DAUDIO_DEFAULT);
        if (ret != DH_SUCCESS) {
            DHLOGE("%s: UnRegister default mic device failed, ret: %d.", LOG_TAG, ret);
            return ret;
        }
    }
    enabledPorts_.erase(PIN_IN_DAUDIO_DEFAULT);
    return DH_SUCCESS;
}

int32_t DMicDev::OpenDevice(const std::string &devId, const int32_t dhId)
{
    DHLOGI("%s: OpenDevice devId: %s, dhId: %d.", LOG_TAG, GetAnonyString(devId).c_str(), dhId);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("%s: OpenDevice, callback is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_MICCALLBACK_NULL;
    }
    if (devId != devId_ || dhId != curPort_) {
        DHLOGE("%s: Device id or port id is wrong.", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }

    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    json jParam;
    jParam["dhId"] = std::to_string(dhId);
    event->type = AudioEventType::OPEN_MIC;
    event->content = jParam.dump();
    cbObj->NotifyEvent(event);
    return DH_SUCCESS;
}

int32_t DMicDev::CloseDevice(const std::string &devId, const int32_t dhId)
{
    DHLOGI("%s: CloseDevice devId: %s, dhId: %d.", LOG_TAG, GetAnonyString(devId).c_str(), dhId);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("%s: CloseDevice, callback is null", LOG_TAG);
        return ERR_DH_AUDIO_SA_MICCALLBACK_NULL;
    }
    if (devId != devId_ || dhId != curPort_) {
        DHLOGE("%s: Device id or port id is wrong.", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }
    std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
    event->type = AudioEventType::CLOSE_MIC;
    cbObj->NotifyEvent(event);
    return DH_SUCCESS;
}

int32_t DMicDev::SetParameters(const std::string &devId, const int32_t dhId, const AudioParamHDF &param)
{
    DHLOGI("%s: Set mic parameters {samplerate: %d, channelmask: %d, format: %d, period: %d, "
        "framesize: %d, ext{%s}}.",
        LOG_TAG, param.sampleRate, param.channelMask, param.bitFormat, param.period, param.frameSize,
        param.ext.c_str());
    if (devId != devId_) {
        DHLOGE("%s: Device Id is wrong, set mic parameters failed.", LOG_TAG);
        return ERR_DH_AUDIO_FAILED;
    }

    curPort_ = dhId;
    sampleRate_ = param.sampleRate;
    channelMask_ = param.channelMask;
    bitFormat_ = AudioSampleFormat::SAMPLE_S16LE;
    period_ = param.period;
    frameSize_ = param.period;
    extParam_ = param.ext;
    return DH_SUCCESS;
}

int32_t DMicDev::NotifyEvent(const std::string &devId, int32_t dhId, const AudioEvent &event)
{
    DHLOGI("%s: Notify mic event.", LOG_TAG);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("%s: Eventcallback is null", LOG_TAG);
        return ERR_DH_AUDIO_SA_EVENT_CALLBACK_NULL;
    }
    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    cbObj->NotifyEvent(audioEvent);
    return DH_SUCCESS;
}

int32_t DMicDev::SetUp()
{
    DHLOGI("%s: SetUp mic device.", LOG_TAG);
    if (micTrans_ == nullptr) {
        micTrans_ = std::make_shared<AudioDecodeTransport>(devId_);
    }

    AudioParam param;
    param.comParam.sampleRate = sampleRate_;
    param.comParam.channelMask = channelMask_;
    param.comParam.bitFormat = bitFormat_;
    param.comParam.codecType = AudioCodecType::AUDIO_CODEC_AAC;
    int32_t ret = micTrans_->SetUp(param, param, shared_from_this(), "mic");
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic trans set up failed. ret: %d.", LOG_TAG, ret);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DMicDev::Start()
{
    DHLOGI("%s: Start mic device.", LOG_TAG);
    if (micTrans_ == nullptr) {
        DHLOGE("%s: Mic trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_MIC_TRANS_NULL;
    }
    int32_t ret = micTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Mic trans start failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    isOpened_ = true;
    return DH_SUCCESS;
}

int32_t DMicDev::Stop()
{
    DHLOGI("%s: Stop mic device.", LOG_TAG);
    if (micTrans_ == nullptr) {
        DHLOGE("%s: Mic trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_MIC_TRANS_NULL;
    }

    int32_t ret = micTrans_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Stop mic trans failed, ret: %d.", LOG_TAG, ret);
    }
    isOpened_ = false;
    return DH_SUCCESS;
}

int32_t DMicDev::Release()
{
    DHLOGI("%s: Release mic device.", LOG_TAG);
    if (micTrans_ == nullptr) {
        DHLOGE("%s: Mic trans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_MIC_TRANS_NULL;
    }

    int32_t ret = micTrans_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Release mic trans failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    return DH_SUCCESS;
}

bool DMicDev::IsOpened()
{
    return isOpened_;
}

int32_t DMicDev::WriteStreamData(const std::string& devId, const int32_t dhId, std::shared_ptr<AudioData> &data)
{
    (void)devId;
    (void)dhId;
    (void)data;
    return DH_SUCCESS;
}

int32_t DMicDev::ReadStreamData(const std::string &devId, const int32_t dhId, std::shared_ptr<AudioData> &data)
{
    if (micTrans_ == nullptr) {
        DHLOGE("%s: ReadStreamData, micTrans is null.", LOG_TAG);
        return ERR_DH_AUDIO_SA_MICTRANS_NULL;
    }
    int32_t ret = micTrans_->RequestAudioData(data);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: ReadStreamData failed, ret: %d.", LOG_TAG, ret);
        return ret;
    }
    return DH_SUCCESS;
}

std::shared_ptr<AudioParam> DMicDev::GetAudioParam()
{
    std::shared_ptr<AudioParam> param = std::make_shared<AudioParam>();
    param->comParam.sampleRate = sampleRate_;
    param->comParam.channelMask = channelMask_;
    param->comParam.bitFormat = bitFormat_;
    param->renderOpts.contentType = CONTENT_TYPE_MUSIC;
    param->renderOpts.streamUsage = STREAM_USAGE_MEDIA;
    param->CaptureOpts.sourceType = SOURCE_TYPE_MIC;
    param->CaptureOpts.capturerFlags = 0;
    return param;
}

int32_t DMicDev::NotifyHdfAudioEvent(std::shared_ptr<AudioEvent> &event)
{
    int32_t ret = DAudioHdiHandler::GetInstance().NotifyEvent(devId_, curPort_, event);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Notify event: %d, result: %s.", LOG_TAG, event->type, event->content.c_str());
    }
    return DH_SUCCESS;
}

int32_t DMicDev::OnStateChange(int32_t type)
{
    DHLOGI("%s: On State Change type: %d", LOG_TAG, type);
    switch (type) {
        case AudioEventType::DATA_OPENED:
            isTransReady_ = true;
            break;
        case AudioEventType::DATA_CLOSED:
            isTransReady_ = false;
            break;
        default:
            break;
    }
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS
