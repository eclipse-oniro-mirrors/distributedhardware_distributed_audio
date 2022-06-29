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

#ifndef OHOS_DSPEAKER_CLIENT_H
#define OHOS_DSPEAKER_CLIENT_H

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_system_manager.h"

#include "audio_data.h"
#include "audio_decode_transport.h"
#include "audio_event.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "iaudio_datatrans_callback.h"
#include "iaudio_event_callback.h"

namespace OHOS {
namespace DistributedHardware {
class DSpeakerClient : public IAudioDataTransCallback,
    public AudioStandard::VolumeKeyEventCallback,
    public std::enable_shared_from_this<DSpeakerClient> {
public:
    DSpeakerClient(const std::string &devId, const std::shared_ptr<IAudioEventCallback> &callback)
        : devId_(devId), eventCallback_(callback) {};
    ~DSpeakerClient();

    int32_t OnStateChange(int32_t type) override;
    void OnVolumeKeyEvent(AudioStandard::AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi) override;

    int32_t SetUp(const AudioParam &param);
    int32_t StartRender();
    int32_t StopRender();
    int32_t GetAudioParameters(const std::shared_ptr<AudioEvent> &event);
    int32_t SetAudioParameters(const std::shared_ptr<AudioEvent> &event);

private:
    void PlayThreadRunning();
    void StringSplit(const std::string &str, const uint8_t &splits, std::vector<std::string> &res);

private:
    constexpr static const char *LOG_TAG = "DSpeakerClient";
    constexpr static  size_t NUMBER_ZERO = 0;
    constexpr static  size_t NUMBER_ONE = 1;

    std::string devId_;
    std::thread renderDataThread_;
    AudioParam audioParam_;
    bool isRenderReady_ = false;

    std::unique_ptr<AudioStandard::AudioRenderer> audioRenderer_;
    std::shared_ptr<AudioDecodeTransport> speakerTrans_ = nullptr;
    std::shared_ptr<IAudioEventCallback> eventCallback_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DMIC_CLIENT_H
