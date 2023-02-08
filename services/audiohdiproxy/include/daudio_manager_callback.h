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

#ifndef OHOS_DAUDIO_MANAGER_CALLBACK_H
#define OHOS_DAUDIO_MANAGER_CALLBACK_H

#include <v1_0/id_audio_callback.h>
#include <v1_0/types.h>

#include "idaudio_hdi_callback.h"

namespace OHOS {
namespace DistributedHardware {
class DAudioManagerCallback : public OHOS::HDI::DistributedAudio::Audioext::V1_0::IDAudioCallback {
public:
    explicit DAudioManagerCallback(const std::shared_ptr<IDAudioHdiCallback> callback) : callback_(callback) {};
    ~DAudioManagerCallback() override = default;

    int32_t OpenDevice(const std::string &adpName, int32_t devId) override;

    int32_t CloseDevice(const std::string &adpName, int32_t devId) override;

    int32_t SetParameters(const std::string &adpNam, int32_t devId,
        const OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioParameter &param) override;

    int32_t NotifyEvent(const std::string &adpNam, int32_t devId,
        const OHOS::HDI::DistributedAudio::Audioext::V1_0::DAudioEvent &event) override;

    int32_t WriteStreamData(const std::string &adpNam, int32_t devId,
        const OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioData &data) override;

    int32_t ReadStreamData(const std::string &adpName, int32_t devId,
        OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioData &data) override;

    int32_t ReadMmapPosition(const std::string &adpName, int32_t devId,
        uint64_t &frames, uint64_t &timeStamp) override;

    int32_t RefreshAshmemInfo(const std::string &adpName, int32_t devId,
        int32_t fd, int32_t ashmemLength, int32_t lengthPerTrans) override;

private:
    int32_t GetAudioParamHDF(const OHOS::HDI::DistributedAudio::Audioext::V1_0::AudioParameter& param,
        AudioParamHDF& paramHDF);

private:
    std::shared_ptr<IDAudioHdiCallback> callback_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DAUDIO_MANAGER_CALLBACK_H
