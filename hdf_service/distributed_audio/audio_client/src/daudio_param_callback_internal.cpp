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

#include "daudio_param_callback_internal.h"

#include <string>
#include <v1_0/audio_param_callback_stub.h>
#include <v1_0/types.h>

#include "daudio_errcode.h"

#define HDF_LOG_TAG HDF_AUDIO
namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

class AudioParamCallbackImpl final : public IAudioParamCallback {
public:
    AudioParamCallbackImpl(ParamCallback callback, void *cookie) : callback_(callback), cookie_(cookie) {}
    ~AudioParamCallbackImpl() {}

    virtual int32_t OnAudioParamNotify(AudioExtParamKeyHAL key, const std::string& condition,
        const std::string& value) override;
private:
    ParamCallback callback_ = nullptr;
    void *cookie_ = nullptr;
};
AudioParamCallbackContext::AudioParamCallbackContext(ParamCallback callback, void *cookie)
{
    callbackStub_ = new AudioParamCallbackImpl(callback, cookie);
}

int32_t AudioParamCallbackImpl::OnAudioParamNotify(AudioExtParamKeyHAL key, const std::string& condition,
    const std::string& value)
{
    if (callback_ != nullptr) {
        callback_(static_cast<AudioExtParamKey>(key), condition.c_str(), value.c_str(), nullptr, cookie_);
        return DH_SUCCESS;
    } else {
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
}
} // namespace DistributedHardware
} // namespace OHOS