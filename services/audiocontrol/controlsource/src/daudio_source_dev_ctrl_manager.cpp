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

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSourceDevCtrlMgr"

namespace OHOS {
namespace DistributedHardware {
DAudioSourceDevCtrlMgr::DAudioSourceDevCtrlMgr(const std::string &devId,
    std::shared_ptr<IAudioEventCallback> audioEventCallback)
{
    DHLOGI("Control manager constructed.");
    devId_ = devId;
    audioEventCallback_ = audioEventCallback;
}

DAudioSourceDevCtrlMgr::~DAudioSourceDevCtrlMgr()
{
    DHLOGI("Control manager deconstructed.");
}

int32_t DAudioSourceDevCtrlMgr::SetUp()
{
    DHLOGI("Set up source development control manager.");
    if (audioCtrlTrans_ == nullptr) {
        audioCtrlTrans_ = std::make_shared<AudioCtrlTransport>(devId_);
    }
    audioCtrlTrans_->SetUp(shared_from_this());
    return DH_SUCCESS;
}

int32_t DAudioSourceDevCtrlMgr::Start()
{
    DHLOGI("Start source development control manager.");
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("Audio ctrl trans is null, start failed");
        return ERR_DH_AUDIO_SA_CTRL_TRANS_NULL;
    }

    int32_t ret = audioCtrlTrans_->Start();
    if (ret != DH_SUCCESS) {
        DHLOGE("Audio ctrl trans start failed, ret:%d.", ret);
        return ret;
    }

    std::unique_lock<std::mutex> lck(channelWaitMutex_);
    auto status = channelWaitCond_.wait_for(lck, std::chrono::seconds(CHANNEL_WAIT_SECONDS),
        [this]() { return isOpened_ == true; });
    if (!status) {
        DHLOGE("Wait channel open timeout(%ds).", CHANNEL_WAIT_SECONDS);
        return ERR_DH_AUDIO_SA_CTRL_CHANNEL_WAIT_TIMEOUT;
    }

    return DH_SUCCESS;
}

int32_t DAudioSourceDevCtrlMgr::Stop()
{
    isOpened_ = false;
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("Audio ctrl trans is null, no need stop");
        return DH_SUCCESS;
    }

    return audioCtrlTrans_->Stop();
}

int32_t DAudioSourceDevCtrlMgr::Release()
{
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("Audio ctrl trans is null, no need release.");
        return DH_SUCCESS;
    }

    int32_t ret = audioCtrlTrans_->Release();
    if (ret != DH_SUCCESS) {
        DHLOGE("Release audio ctrl failed.");
        return ret;
    }
    audioCtrlTrans_ = nullptr;
    return DH_SUCCESS;
}

bool DAudioSourceDevCtrlMgr::IsOpened()
{
    return isOpened_;
}

int32_t DAudioSourceDevCtrlMgr::SendAudioEvent(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("Send audio event.");
    if (audioCtrlTrans_ == nullptr) {
        DHLOGE("Send audio event, Audio ctrl trans is null");
        return ERR_DH_AUDIO_SA_CTRL_TRANS_NULL;
    }
    if (event == nullptr) {
        DHLOGE("The parameter is empty.");
        return ERR_DH_AUDIO_NULLPTR;
    }
    return audioCtrlTrans_->SendAudioEvent(event);
}

void DAudioSourceDevCtrlMgr::OnStateChange(int32_t type)
{
    DHLOGI("On ctrl device state change, type: %d.", type);
    auto event = std::make_shared<AudioEvent>();
    event->type = (AudioEventType)type;
    if (event->type == AudioEventType::CTRL_OPENED) {
        DHLOGI("Audio ctrl trans on opened.");
        isOpened_ = true;
        channelWaitCond_.notify_all();
    } else if (event->type == AudioEventType::CTRL_CLOSED) {
        DHLOGI("Audio ctrl trans on closed.");
        isOpened_ = false;
    }
    if (audioEventCallback_ == nullptr) {
        DHLOGE("Callback is nullptr.");
        return;
    }
    audioEventCallback_->NotifyEvent(event);
}

void DAudioSourceDevCtrlMgr::OnEventReceived(const std::shared_ptr<AudioEvent> &event)
{
    DHLOGI("Received event");
    if (event == nullptr) {
        DHLOGE("The parameter is empty.");
        return;
    }
    if (audioEventCallback_ == nullptr) {
        DHLOGE("Callback is nullptr.");
        return;
    }
    audioEventCallback_->NotifyEvent(event);
}
} // namespace DistributedHardware
} // namespace OHOS
