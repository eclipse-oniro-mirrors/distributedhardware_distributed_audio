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

#include "daudio_source_dev_ctrl_manager.h"

#include "audio_ctrl_transport.h"
#include "audio_param.h"
#include "daudio_constants.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "daudio_util.h"

namespace OHOS {
namespace DistributedHardware {
DAudioSourceDevCtrlMgr::DAudioSourceDevCtrlMgr(const std::string &devId,
    std::shared_ptr<IAudioEventCallback> audioEventCallback)
{
    DHLOGI("%s: Distributed audio source ctrl constructed.", LOG_TAG);
    devId_ = devId;
    audioEventCallback_ = audioEventCallback;
}

DAudioSourceDevCtrlMgr::~DAudioSourceDevCtrlMgr()
{
    DHLOGI("%s: Distributed audio source ctrl destructed.", LOG_TAG);
    audioCtrlTrans_ = nullptr;
    audioEventCallback_ = nullptr;
}

int32_t DAudioSourceDevCtrlMgr::Init()
{
    DHLOGI("%s: Init.", LOG_TAG);
    return DH_SUCCESS;
}

int32_t DAudioSourceDevCtrlMgr::UnInit()
{
    DHLOGI("%s: UnInit.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        return DH_SUCCESS;
    }
    int32_t ret = audioCtrlTrans_->Stop();
    if (ret != DH_SUCCESS) {
        return ret;
    }

    ret = audioCtrlTrans_->Release();
    if (ret != DH_SUCCESS) {
        return ret;
    }

    return DH_SUCCESS;
}

int32_t DAudioSourceDevCtrlMgr::SetUp()
{
    DHLOGI("%s: SetUp.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        audioCtrlTrans_ = std::make_shared<AudioCtrlTransport>(devId_);
    }
    audioCtrlTrans_->SetUp(shared_from_this());
    return DH_SUCCESS;
}

int32_t DAudioSourceDevCtrlMgr::Start()
{
    DHLOGI("%s: Start.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("%s: audio ctrl trans is null, start failed", LOG_TAG);
        return ERR_DH_AUDIO_SA_CTRL_TRANS_NULL;
    }

    int32_t ret = audioCtrlTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("%s: audio ctrl trans start failed, ret:%d.", LOG_TAG, ret);
        return ret;
    }

    std::unique_lock<std::mutex> lck(channelWaitMutex_);
    auto status = channelWaitCond_.wait_for(lck, std::chrono::seconds(CHANNEL_WAIT_SECONDS),
        [this]() { return isOpened_ == true; });
    if (!status) {
        DHLOGE("%s: wait channel open timeout(%ds).", LOG_TAG, CHANNEL_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_CTRL_CHANNEL_WAIT_TIMEOUT;
    }

    return DH_SUCCESS;
}

int32_t DAudioSourceDevCtrlMgr::Stop()
{
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("%s: audio ctrl trans is null, no need stop", LOG_TAG);
        return ERR_DH_AUDIO_SA_CTRL_TRANS_NULL;
    }
    isOpened_ = false;

    return audioCtrlTrans_->Stop();
}

int32_t DAudioSourceDevCtrlMgr::Release()
{
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("%s: audio ctrl trans is null, no need release.", LOG_TAG);
        return ERR_DH_AUDIO_SA_CTRL_TRANS_NULL;
    }

    return audioCtrlTrans_->Release();
}

bool DAudioSourceDevCtrlMgr::IsOpened()
{
    return isOpened_;
}

int32_t DAudioSourceDevCtrlMgr::SendAudioEvent(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: SendAudioEvent.", LOG_TAG);
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("%s: Send audio event, audio ctrl trans is null", LOG_TAG);
        return ERR_DH_AUDIO_SA_CTRL_TRANS_NULL;
    }

    return audioCtrlTrans_->SendAudioEvent(event);
}

void DAudioSourceDevCtrlMgr::OnStateChange(int32_t type)
{
    DHLOGI("%s: OnStateChange, type:%d", LOG_TAG, type);
    if (type == AudioEventType::CTRL_OPENED) {
        DHLOGI("%s: audio ctrl trans on Channel Open", LOG_TAG);
        isOpened_ = true;
        channelWaitCond_.notify_all();
    } else if (type == AudioEventType::CTRL_CLOSED) {
        isOpened_ = false;
    }
}

void DAudioSourceDevCtrlMgr::OnEventReceived(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("%s: OnEventReceived", LOG_TAG);
    audioEventCallback_->NotifyEvent(event);
}
} // namespace DistributedHardware
} // namespace OHOS
