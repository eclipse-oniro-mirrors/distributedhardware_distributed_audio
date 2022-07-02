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

#include "daudio_render_callback_internal.h"

#include <v1_0/audio_render_callback_stub.h>

#include "daudio_errcode.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRenderCallback;

class AudioRenderCallbackImpl final : public IAudioRenderCallback {
public:
    AudioRenderCallbackImpl(RenderCallback callback, void *cookie) : callback_(callback), cookie_(cookie) {}
    ~AudioRenderCallbackImpl() {}

    virtual int32_t OnAudioWriteCompleted() override;
    virtual int32_t OnAudioDrainCompleted() override;
    virtual int32_t OnAudioFlushCompleted() override;
    virtual int32_t OnAudioRenderFull() override;
    virtual int32_t OnAudioErrorOccur() override;
private:
    RenderCallback callback_ = nullptr;
    void *cookie_ = nullptr;
};

AudioRenderCallbackContext::AudioRenderCallbackContext(RenderCallback callback, void *cookie)
{
    callbackStub_ = new AudioRenderCallbackImpl(callback, cookie);
}

int32_t AudioRenderCallbackImpl::OnAudioWriteCompleted()
{
    if (callback_ != nullptr) {
        callback_(AUDIO_NONBLOCK_WRITE_COMPELETED, nullptr, cookie_);
        return DH_SUCCESS;
    } else {
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
}

int32_t AudioRenderCallbackImpl::OnAudioDrainCompleted()
{
    if (callback_ != nullptr) {
        callback_(AUDIO_DRAIN_COMPELETED, nullptr, cookie_);
        return DH_SUCCESS;
    } else {
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
}

int32_t AudioRenderCallbackImpl::OnAudioFlushCompleted()
{
    if (callback_ != nullptr) {
        callback_(AUDIO_FLUSH_COMPLETED, nullptr, cookie_);
        return DH_SUCCESS;
    } else {
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
}

int32_t AudioRenderCallbackImpl::OnAudioRenderFull()
{
    if (callback_ != nullptr) {
        callback_(AUDIO_RENDER_FULL, nullptr, cookie_);
        return DH_SUCCESS;
    } else {
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
}

int32_t AudioRenderCallbackImpl::OnAudioErrorOccur()
{
    if (callback_ != nullptr) {
        callback_(AUDIO_ERROR_OCCUR, nullptr, cookie_);
        return DH_SUCCESS;
    } else {
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
}
} // namespace DistributedHardware
} // namespace OHOS