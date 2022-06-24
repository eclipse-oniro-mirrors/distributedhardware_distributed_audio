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

#include "daudio_sink_dev_ctrl_manager.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "audio_ctrl_transport.h"
#include "audio_param.h"
#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"


namespace OHOS {
namespace DistributedHardware {
DAudioSinkDevCtrlMgr::DAudioSinkDevCtrlMgr(const std::string &devId,
    std::shared_ptr<IAudioEventCallback> audioEventCallback)
{
    DHLOGI("%s: Distributed audio sink ctrl constructed.", LOG_TAG);
    devId_ = devId;
    audioEventCallback_ = audioEventCallback;
}

DAudioSinkDevCtrlMgr::~DAudioSinkDevCtrlMgr()
{
    DHLOGI("%s: Distributed audio sink ctrl destructed.", LOG_TAG);
    audioCtrlTrans_ = nullptr;
    audioEventCallback_ = nullptr;
    isOpened_ = false;
}

void DAudioSinkDevCtrlMgr::OnStateChange(int32_t type)
{
    DHLOGI("%s: SinkDevCtrlMgr OnStateChange, type: %d.", LOG_TAG, type);
    switch (type) {
        case AudioEventType::CTRL_OPENED:
            isOpened_ = true;
            return;
        case AudioEventType::CTRL_CLOSED:
            isOpened_ = false;
            return;
        default:
            DHLOGE("%s:SinkDevCtrlMgr OnStateChange not a valid state, type: %d.", LOG_TAG, type);
            return;
    }
}

int32_t DAudioSinkDevCtrlMgr::Init()
{
    DHLOGI("%s: Init.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSinkDevCtrlMgr::UnInit()
{
    DHLOGI("%s: UnInit.", LOG_TAG);
    if (audioCtrlTrans_ != nullptr) {
        audioCtrlTrans_->Release();
        audioCtrlTrans_ = nullptr;
    }

    if (audioEventCallback_ != nullptr) {
        audioEventCallback_ = nullptr;
    }

    isOpened_ = false;
    return DH_SUCCESS;
}

int32_t DAudioSinkDevCtrlMgr::SetUp()
{
    DHLOGI("%s: SetUp.", LOG_TAG);
    if (!audioCtrlTrans_) {
        audioCtrlTrans_ = std::make_shared<AudioCtrlTransport>(devId_);
    }

    int32_t ret = audioCtrlTrans_->SetUp(shared_from_this());
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Ctrl trans setup failed.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}

int32_t DAudioSinkDevCtrlMgr::Start()
{
    DHLOGI("%s: Start.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINK_CTRL_TRANS_NULL;
    }
    int32_t ret = audioCtrlTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: Ctrl trans start failed.", LOG_TAG);
        return ret;
    }

    std::unique_lock<std::mutex> lck(channelWaitMutex_);
    auto status =
        channelWaitCond_.wait_for(lck, std::chrono::seconds(CHANNEL_WAIT_SECONDS), [this]() { return false; });
    if (!status) {
        DHLOGE("%s: Wait channel open timeout(%ds).", LOG_TAG, CHANNEL_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_MIC_CHANNEL_WAIT_TIMEOUT;
    }
    isOpened_ = true;
    return DH_SUCCESS;
}

int32_t DAudioSinkDevCtrlMgr::Stop()
{
    DHLOGI("%s: Stop.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINK_CTRL_TRANS_NULL;
    }

    int32_t ret = audioCtrlTrans_->Stop();
    if (ret != DH_SUCCESS) {
        return ret;
    }
    isOpened_ = false;
    return DH_SUCCESS;
}

int32_t DAudioSinkDevCtrlMgr::Release()
{
    DHLOGI("%s: Release.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINK_CTRL_TRANS_NULL;
    }
    int32_t ret = audioCtrlTrans_->Release();
    if (ret != DH_SUCCESS) {
        return ret;
    }
    return UnInit();
}

bool DAudioSinkDevCtrlMgr::IsOpened()
{
    return isOpened_;
}

void DAudioSinkDevCtrlMgr::OnEventReceived(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: OnEventReceived.", LOG_TAG);
    audioEventCallback_->NotifyEvent(audioEvent_);
}

int32_t DAudioSinkDevCtrlMgr::SendAudioEvent(const std::shared_ptr<AudioEvent> &audioEvent_)
{
    DHLOGI("%s: SendAudioEvent.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        return ERR_DH_AUDIO_SA_SINK_CTRL_TRANS_NULL;
    }
    int32_t ret = audioCtrlTrans_->SendAudioEvent(audioEvent_);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: SendAudioEvent is error.", LOG_TAG);
        return ret;
    }
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
