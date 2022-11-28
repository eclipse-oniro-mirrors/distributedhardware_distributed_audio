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
#include "daudio_hisysevent.h"
#include "daudio_hitrace.h"
#include "daudio_log.h"
#include "daudio_util.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DMicDev"

namespace OHOS {
namespace DistributedHardware {
int32_t DMicDev::EnableDMic(const int32_t dhId, const std::string &capability)
{
    DHLOGI("Enable distributed mic dhId: %d.", dhId);
    if (enabledPorts_.empty()) {
        if (EnableDevice(PIN_IN_DAUDIO_DEFAULT, capability) != DH_SUCCESS) {
            return ERR_DH_AUDIO_FAILED;
        }
    }
    int32_t ret = EnableDevice(dhId, capability);
    if (ret != DH_SUCCESS) {
        return ret;
    }

    DaudioFinishAsyncTrace(DAUDIO_REGISTER_AUDIO, DAUDIO_REGISTER_AUDIO_TASKID);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUIDO_REGISTER, devId_, std::to_string(dhId),
        "daudio mic enable success.");
    return DH_SUCCESS;
}

int32_t DMicDev::EnableDevice(const int32_t dhId, const std::string &capability)
{
    DHLOGI("Enable default mic device.");
    int32_t ret = DAudioHdiHandler::GetInstance().RegisterAudioDevice(devId_, dhId, capability, shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("Register mic device failed, ret: %d.", ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_REGISTER_FAIL, devId_, std::to_string(dhId), ret,
            "daudio register mic device failed.");
        return ret;
    }
    enabledPorts_.insert(dhId);
    return DH_SUCCESS;
}

int32_t DMicDev::DisableDMic(const int32_t dhId)
{
    DHLOGI("Disable distributed mic.");
    if (dhId == curPort_) {
        isOpened_.store(false);
    }
    if (DisableDevice(dhId) != DH_SUCCESS) {
        return ERR_DH_AUDIO_FAILED;
    }

    if (enabledPorts_.size() == SINGLE_ITEM && enabledPorts_.find(PIN_IN_DAUDIO_DEFAULT) != enabledPorts_.end()) {
        if (DisableDevice(PIN_IN_DAUDIO_DEFAULT) != DH_SUCCESS) {
            return ERR_DH_AUDIO_FAILED;
        }
    }

    DaudioFinishAsyncTrace(DAUDIO_UNREGISTER_AUDIO, DAUDIO_UNREGISTER_AUDIO_TASKID);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_UNREGISTER, devId_, std::to_string(dhId),
        "daudio mic disable success.");
    return DH_SUCCESS;
}

int32_t DMicDev::DisableDevice(const int32_t dhId)
{
    int32_t ret = DAudioHdiHandler::GetInstance().UnRegisterAudioDevice(devId_, dhId);
    if (ret != DH_SUCCESS) {
        DHLOGE("unregister audio device failed, ret: %d", ret);
        DAudioHisysevent::GetInstance().SysEventWriteFault(DAUDIO_UNREGISTER_FAIL, devId_, std::to_string(dhId), ret,
            "daudio unregister audio mic device failed.");
        return ret;
    }
    enabledPorts_.erase(dhId);
    return DH_SUCCESS;
}

int32_t DMicDev::OpenDevice(const std::string &devId, const int32_t dhId)
{
    DHLOGI("Open mic device devId: %s, dhId: %d.", GetAnonyString(devId).c_str(), dhId);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("Event callback is null");
        return ERR_DH_AUDIO_SA_MICCALLBACK_NULL;
    }
    json jParam = { { KEY_DH_ID, std::to_string(dhId) } };
    AudioEvent event(AudioEventType::OPEN_MIC, jParam.dump());
    cbObj->NotifyEvent(event);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_OPEN, devId, std::to_string(dhId),
        "daudio mic device open success.");
    return DH_SUCCESS;
}

int32_t DMicDev::CloseDevice(const std::string &devId, const int32_t dhId)
{
    DHLOGI("Close mic device devId: %s, dhId: %d.", GetAnonyString(devId).c_str(), dhId);
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("Event callback is null");
        return ERR_DH_AUDIO_SA_MICCALLBACK_NULL;
    }
    json jParam = { { KEY_DH_ID, std::to_string(dhId) } };
    AudioEvent event(AudioEventType::CLOSE_MIC, jParam.dump());
    cbObj->NotifyEvent(event);
    DAudioHisysevent::GetInstance().SysEventWriteBehavior(DAUDIO_CLOSE, devId, std::to_string(dhId),
        "daudio mic device close success.");
    curPort_ = 0;
    return DH_SUCCESS;
}

int32_t DMicDev::SetParameters(const std::string &devId, const int32_t dhId, const AudioParamHDF &param)
{
    DHLOGI("Set mic parameters {samplerate: %d, channelmask: %d, format: %d, period: %d, "
        "framesize: %d, ext{%s}}.",
        param.sampleRate, param.channelMask, param.bitFormat, param.period, param.frameSize,
        param.ext.c_str());
    curPort_ = dhId;
    paramHDF_ = param;

    param_.comParam.sampleRate = paramHDF_.sampleRate;
    param_.comParam.channelMask = paramHDF_.channelMask;
    param_.comParam.bitFormat = paramHDF_.bitFormat;
    param_.comParam.codecType = AudioCodecType::AUDIO_CODEC_AAC;
    param_.captureOpts.sourceType = SOURCE_TYPE_MIC;
    param_.captureOpts.capturerFlags = 0;
    return DH_SUCCESS;
}

int32_t DMicDev::NotifyEvent(const std::string &devId, int32_t dhId, const AudioEvent &event)
{
    DHLOGI("Notify mic event.");
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("Event callback is null");
        return ERR_DH_AUDIO_SA_EVENT_CALLBACK_NULL;
    }
    AudioEvent audioEvent(event.type, event.content);
    cbObj->NotifyEvent(audioEvent);
    return DH_SUCCESS;
}

int32_t DMicDev::SetUp()
{
    DHLOGI("Set up mic device.");
    if (micTrans_ == nullptr) {
        micTrans_ = std::make_shared<AudioDecodeTransport>(devId_);
    }
    int32_t ret = micTrans_->SetUp(param_, param_, shared_from_this(), CAP_MIC);
    if (ret != DH_SUCCESS) {
        DHLOGE("Mic trans set up failed. ret: %d.", ret);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DMicDev::Start()
{
    DHLOGI("Start mic device.");
    if (micTrans_ == nullptr) {
        DHLOGE("Mic trans is null.");
        return ERR_DH_AUDIO_SA_MIC_TRANS_NULL;
    }
    int32_t ret = micTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("Mic trans start failed, ret: %d.", ret);
        return ret;
    }

    std::unique_lock<std::mutex> lck(channelWaitMutex_);
    auto status = channelWaitCond_.wait_for(lck, std::chrono::seconds(CHANNEL_WAIT_SECONDS),
        [this]() { return isTransReady_.load(); });
    if (!status) {
        DHLOGE("Wait channel open timeout(%ds).", CHANNEL_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_SPEAKER_CHANNEL_WAIT_TIMEOUT;
    }
    isOpened_.store(true);
    return DH_SUCCESS;
}

int32_t DMicDev::Stop()
{
    DHLOGI("Stop mic device.");
    if (micTrans_ == nullptr) {
        DHLOGE("Mic trans is null.");
        return ERR_DH_AUDIO_SA_MIC_TRANS_NULL;
    }

    isOpened_.store(false);
    isTransReady_.store(false);
    int32_t ret = micTrans_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("Stop mic trans failed, ret: %d.", ret);
    }
    return DH_SUCCESS;
}

int32_t DMicDev::Release()
{
    DHLOGI("Release mic device.");
    if (micTrans_ == nullptr) {
        DHLOGE("Mic trans is null.");
        return ERR_DH_AUDIO_SA_MIC_TRANS_NULL;
    }

    int32_t ret = micTrans_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Release mic trans failed, ret: %d.", ret);
        return ret;
    }
    return DH_SUCCESS;
}

bool DMicDev::IsOpened()
{
    return isOpened_.load();
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
    std::lock_guard<std::mutex> lock(dataQueueMtx_);
    if (dataQueue_.empty()) {
        DHLOGI("Data queue is empty.");
        data = std::make_shared<AudioData>(FRAME_SIZE);
    } else {
        data = dataQueue_.front();
        dataQueue_.pop();
    }
    return DH_SUCCESS;
}

AudioParam DMicDev::GetAudioParam() const
{
    return param_;
}

int32_t DMicDev::NotifyHdfAudioEvent(const AudioEvent &event)
{
    int32_t ret = DAudioHdiHandler::GetInstance().NotifyEvent(devId_, curPort_, event);
    if (ret != DH_SUCCESS) {
        DHLOGE("Notify event: %d, result: %s.", event.type, event.content.c_str());
    }
    return DH_SUCCESS;
}

int32_t DMicDev::OnStateChange(const AudioEventType type)
{
    DHLOGI("On mic device state change, type: %d", type);
    AudioEvent event;
    switch (type) {
        case AudioEventType::DATA_OPENED:
            isTransReady_.store(true);
            channelWaitCond_.notify_all();
            event.type = AudioEventType::MIC_OPENED;
            break;
        case AudioEventType::DATA_CLOSED:
            isTransReady_.store(false);
            event.type = AudioEventType::MIC_CLOSED;
            break;
        default:
            break;
    }
    std::shared_ptr<IAudioEventCallback> cbObj = audioEventCallback_.lock();
    if (cbObj == nullptr) {
        DHLOGE("Event callback is null");
        return ERR_DH_AUDIO_SA_MICCALLBACK_NULL;
    }
    cbObj->NotifyEvent(event);
    return DH_SUCCESS;
}

int32_t DMicDev::OnDecodeTransDataDone(const std::shared_ptr<AudioData> &audioData)
{
    if (audioData == nullptr) {
        DHLOGE("The parameter is empty.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    std::lock_guard<std::mutex> lock(dataQueueMtx_);
    while (dataQueue_.size() > DATA_QUEUE_MAX_SIZE) {
        DHLOGD("Data queue overflow.");
        dataQueue_.pop();
    }
    dataQueue_.push(audioData);
    DHLOGD("Push new mic data, buf len: %d", dataQueue_.size());
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS
