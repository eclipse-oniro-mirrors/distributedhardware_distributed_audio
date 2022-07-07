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

#include "dspeaker_client.h"

#include "daudio_constants.h"

namespace OHOS {
namespace DistributedHardware {
DSpeakerClient::~DSpeakerClient()
{
    if (speakerTrans_ != nullptr) {
        DHLOGI("%s: ~DSpeakerClient. Release speaker client.", LOG_TAG);
        StopRender();
    }
}

int32_t DSpeakerClient::SetUp(const AudioParam &param)
{
    DHLOGI("%s: Set speaker client parameters, {sampleRate: %d, bitFormat: %d, channelMask: %d, contentType: %d, "
        "streamUsage: %d}.",
        LOG_TAG, param.comParam.sampleRate, param.comParam.bitFormat, param.comParam.channelMask,
        param.renderOpts.contentType, param.renderOpts.streamUsage);

    audioParam_ = param;
    audioParam_.comParam.bitFormat = SAMPLE_S16LE;
    AudioStandard::AudioRendererOptions rendererOptions = {
        {
            static_cast<AudioStandard::AudioSamplingRate>(audioParam_.comParam.sampleRate),
            AudioStandard::AudioEncodingType::ENCODING_PCM,
            static_cast<AudioStandard::AudioSampleFormat>(SAMPLE_S16LE),
            static_cast<AudioStandard::AudioChannel>(audioParam_.comParam.channelMask),
        },
        {
            static_cast<AudioStandard::ContentType>(audioParam_.renderOpts.contentType),
            static_cast<AudioStandard::StreamUsage>(audioParam_.renderOpts.streamUsage),
            NUMBER_ZERO,
        }
    };
    audioRenderer_ = AudioStandard::AudioRenderer::Create(rendererOptions);
    if (audioRenderer_ == nullptr) {
        DHLOGE("%s: Audio renderer create failed.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_CREATE_RENDER_FAILED;
    }

    int32_t ret =
        AudioStandard::AudioSystemManager::GetInstance()->RegisterVolumeKeyEventCallback(getpid(), shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Failed to register volume key event callback.", LOG_TAG);
        return ret;
    }

    if (speakerTrans_ == nullptr) {
        speakerTrans_ = std::make_shared<AudioDecodeTransport>(devId_);
    }
    ret = speakerTrans_->SetUp(audioParam_, audioParam_, shared_from_this(), "speaker");
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker trans setup failed.", LOG_TAG);
        return ret;
    }
    ret = speakerTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker trans start failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DSpeakerClient::StartRender()
{
    DHLOGI("%s: Start renderer.", LOG_TAG);
    if (audioRenderer_ == nullptr) {
        DHLOGE("%s: Audio renderer instantiation failed.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_RENDER_WITHOUT_INSTANCE;
    }

    if (!audioRenderer_->Start()) {
        DHLOGE("%s: Audio renderer start failed.", LOG_TAG);
        if (!audioRenderer_->Release()) {
            DHLOGE("%s: Audio renderer release failed.", LOG_TAG);
            return ERR_DH_AUDIO_CLIENT_RENDER_RELEASE_FAILED;
        }
        return ERR_DH_AUDIO_CLIENT_RENDER_STARTUP_FAILURE;
    }

    isRenderReady_.store(true);
    renderDataThread_ = std::thread(&DSpeakerClient::PlayThreadRunning, this);
    return DH_SUCCESS;
}

int32_t DSpeakerClient::StopRender()
{
    DHLOGI("%s: Stop renderer.", LOG_TAG);

    isRenderReady_.store(false);
    if (renderDataThread_.joinable()) {
        renderDataThread_.join();
    }

    if (audioRenderer_ == nullptr || speakerTrans_ == nullptr) {
        DHLOGE("%s: Audio renderer or speaker trans is nullptr.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_RENDER_OR_TRANS_IS_NULL;
    }

    int32_t ret = speakerTrans_->Stop();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker trans stop failed.", LOG_TAG);
        return ret;
    }
    ret = speakerTrans_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Speaker trans release failed.", LOG_TAG);
        return ret;
    }
    speakerTrans_ = nullptr;

    if (!audioRenderer_->Stop()) {
        OHOS::DistributedHardware::DHLOGE("%s: Audio renderer stop failed", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_RENDER_STOP_FAILED;
    }
    if (!audioRenderer_->Release()) {
        DHLOGE("%s: Audio renderer release failed.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_RENDER_RELEASE_FAILED;
    }
    return DH_SUCCESS;
}

void DSpeakerClient::PlayThreadRunning()
{
    DHLOGI("%s:Start the renderer thread.", LOG_TAG);
    size_t bufferLen = NUMBER_ZERO;
    if (audioRenderer_->GetBufferSize(bufferLen) != DH_SUCCESS) {
        DHLOGE("%s: Failed to get minimum buffer.", LOG_TAG);
        return;
    }
    DHLOGI("%s: Obtains the minimum buffer length, bufferlen: %d.", LOG_TAG, bufferLen);

    while (isRenderReady_.load()) {
        std::shared_ptr<AudioData> audioData = std::make_shared<AudioData>(bufferLen);
        int32_t ret = speakerTrans_->RequestAudioData(audioData);
        if (ret == DH_SUCCESS && audioData != nullptr) {
            audioRenderer_->Write(audioData->Data(), audioData->Capacity());
        } else {
            DHLOGI("%s: Failed to send data.", LOG_TAG);
        }
    }
}

int32_t DSpeakerClient::OnStateChange(int32_t type)
{
    DHLOGI("%s: On state change. type: %d", LOG_TAG, type);
    switch (type) {
        case AudioEventType::DATA_OPENED: {
            std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
            event->type = AudioEventType::SPEAKER_OPENED;
            event->content = "";
            eventCallback_->NotifyEvent(event);
            return DH_SUCCESS;
        }
        case AudioEventType::DATA_CLOSED: {
            std::shared_ptr<AudioEvent> event = std::make_shared<AudioEvent>();
            event->type = AudioEventType::SPEAKER_CLOSED;
            event->content = "";
            eventCallback_->NotifyEvent(event);
            return DH_SUCCESS;
        }
        default:
            DHLOGE("%s: Invalid parameter type: %d.", LOG_TAG, type);
            return ERR_DH_AUDIO_CLIENT_STATE_IS_INVALID;
    }
}

void DSpeakerClient::OnVolumeKeyEvent(AudioStandard::AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi)
{
    DHLOGI("%s: OnVolumeKeyEvent.", LOG_TAG);
    std::stringstream ss;
    ss << "VOLUME_CHANAGE;"
       << "AUDIO_STREAM_TYPE=" << streamType << ";"
       << "VOLUME_LEVEL=" << volumeLevel << ";"
       << "IS_UPDATEUI=" << isUpdateUi << ";";
    std::string str = ss.str();
    DHLOGI("%s: Volume change notification result, event: %s.", LOG_TAG, str.c_str());

    std::shared_ptr<AudioEvent> audioEvent = std::make_shared<AudioEvent>();
    audioEvent->type = VOLUME_CHANGE;
    audioEvent->content = str;
    eventCallback_->NotifyEvent(audioEvent);
}

int32_t DSpeakerClient::GetAudioParameters(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Get the volume.", LOG_TAG);
    std::shared_ptr<AudioEvent> audioEvent = event;
    std::vector<std::string> volumeList;
    StringSplit(audioEvent->content, '=', volumeList);
    if (volumeList[NUMBER_ZERO] != "VOLUMETYPE") {
        DHLOGE("%s: Invalid event content parameter.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_INVALID_EVENT_PARAM;
    }
    auto volumeType =
        static_cast<AudioStandard::AudioSystemManager::AudioVolumeType>(std::stoi(volumeList[NUMBER_ONE]));
    switch (audioEvent->type) {
        case VOLUME_GET: {
            auto volume = AudioStandard::AudioSystemManager::GetInstance()->GetVolume(volumeType);
            DHLOGI("%s: Get the current volume, volume: %d.", LOG_TAG, volume);
            return volume;
        }
        case VOLUME_MIN_GET: {
            auto volume = AudioStandard::AudioSystemManager::GetInstance()->GetMinVolume(volumeType);
            DHLOGI("%s: Get the Minimum  volume, volume: %d.", LOG_TAG, volume);
            return volume;
        }
        case VOLUME_MAX_GET: {
            auto volume = AudioStandard::AudioSystemManager::GetInstance()->GetMaxVolume(volumeType);
            DHLOGI("%s: Get the Maximum  volume, volume: %d.", LOG_TAG, volume);
            return volume;
        }
        default:
            DHLOGE("%s: Invalid event type parameter.", LOG_TAG);
            return ERR_DH_AUDIO_CLIENT_INVALID_EVENT_PARAM;
    }
}

int32_t DSpeakerClient::SetAudioParameters(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: Set the volume, arg: %s.", LOG_TAG, event->content.c_str());
    std::shared_ptr<AudioEvent> audioEvent = event;
    std::vector<std::string> conditionList;
    std::vector<std::string> typeList;
    std::vector<std::string> levelList;
    StringSplit(audioEvent->content, ';', conditionList);
    StringSplit(conditionList[NUMBER_TWO], '=', typeList);
    StringSplit(conditionList[NUMBER_THREE], '=', levelList);

    if (typeList[NUMBER_ZERO] != "AUDIO_VOLUME_TYPE") {
        DHLOGE("%s: Invalid event content parameter.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_INVALID_EVENT_PARAM;
    }
    auto volumeType = static_cast<AudioStandard::AudioSystemManager::AudioVolumeType>(std::stoi(typeList[NUMBER_ONE]));
    DHLOGE("%s: AudioVolumeType volumeType = %d.", LOG_TAG, volumeType);
    if (audioEvent->type != VOLUME_SET && audioEvent->type != VOLUME_MUTE_SET) {
        DHLOGE("%s: Invalid parameter.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_INVALID_VOLUME_PARAMETER;
    }
    DHLOGE("%s: volume level = %d.", LOG_TAG, std::stoi(levelList[NUMBER_ONE]));
    int32_t ret =
        AudioStandard::AudioSystemManager::GetInstance()->SetVolume(volumeType, std::stoi(levelList[NUMBER_ONE]));
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Voloume set failed.", LOG_TAG);
        return ERR_DH_AUDIO_CLIENT_SET_VOLUME_FAILED;
    }
    return DH_SUCCESS;
}

void DSpeakerClient::StringSplit(const std::string &str, const uint8_t &splits, std::vector<std::string> &res)
{
    if (splits != '=' && splits != ';') {
        DHLOGI("%s: Splits error.", LOG_TAG);
        return;
    }
    char ch = splits;

    if (str.empty()) {
        DHLOGI("%s: The string is empty.", LOG_TAG);
        return;
    }
    std::stringstream ss(str);
    std::string temp;
    while (getline(ss, temp, ch)) {
        res.push_back(temp);
    }
}
} // DistributedHardware
} // OHOS
