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

#ifndef OHOS_DMIC_DEV_H
#define OHOS_DMIC_DEV_H

#include <queue>
#include <set>
#include "nlohmann/json.hpp"

#include "audio_param.h"
#include "daudio_hdi_handler.h"
#include "iaudio_datatrans_callback.h"
#include "iaudio_data_transport.h"
#include "iaudio_event_callback.h"
#include "idaudio_hdi_callback.h"

using json = nlohmann::json;
namespace OHOS {
namespace DistributedHardware {
class DMicDev : public IDAudioHdiCallback,
    public IAudioDataTransCallback,
    public std::enable_shared_from_this<DMicDev> {
public:
    DMicDev(const std::string &devId, std::shared_ptr<IAudioEventCallback> callback)
        : devId_(devId), audioEventCallback_(callback) {};
    ~DMicDev() = default;

    int32_t EnableDMic(const int32_t dhId, const std::string &capability);
    int32_t DisableDMic(const int32_t dhId);

    int32_t OpenDevice(const std::string &devId, int32_t dhId) override;
    int32_t CloseDevice(const std::string &devId, int32_t dhId) override;
    int32_t SetParameters(const std::string &devId, int32_t dhId, const AudioParamHDF &param) override;
    int32_t WriteStreamData(const std::string &devId, int32_t dhId, std::shared_ptr<AudioData> &data) override;
    int32_t ReadStreamData(const std::string &devId, int32_t dhId, std::shared_ptr<AudioData> &data) override;
    int32_t NotifyEvent(const std::string &devId, int32_t dhId, const AudioEvent &event) override;

    int32_t SetUp();
    int32_t Start();
    int32_t Stop();
    int32_t Release();
    bool IsOpened();

    AudioParam GetAudioParam() const;
    int32_t NotifyHdfAudioEvent(const std::shared_ptr<AudioEvent> &event);
    int32_t OnStateChange(int32_t type) override;
    int32_t WriteStreamBuffer(const std::shared_ptr<AudioData> &audioData) override;

private:
    int32_t EnableDevice(const int32_t dhId, const std::string &capability);
    int32_t DisableDevice(const int32_t dhId);

private:
    static constexpr uint8_t CHANNEL_WAIT_SECONDS = 5;
    static constexpr size_t DATA_QUEUE_MAX_SIZE = 5;
    static constexpr size_t FRAME_SIZE = 4096;
    static const constexpr char *LOG_TAG = "DMicDev";
    std::string devId_;
    std::set<int32_t> enabledPorts_;
    std::mutex dataQueueMtx_;
    std::queue<std::shared_ptr<AudioData>> dataQueue_;
    int32_t curPort_ = 0;

    std::weak_ptr<IAudioEventCallback> audioEventCallback_;
    std::shared_ptr<IAudioDataTransport> micTrans_ = nullptr;

    // Mic capture parameters
    AudioParamHDF paramHDF_;
    AudioParam param_;

    std::atomic<bool> isTransReady_ = false;
    std::atomic<bool> isOpened_ = false;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DAUDIO_DMIC_DEV_H