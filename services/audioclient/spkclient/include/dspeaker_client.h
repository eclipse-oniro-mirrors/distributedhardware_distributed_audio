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

#ifndef OHOS_DSPEAKER_CLIENT_H
#define OHOS_DSPEAKER_CLIENT_H

#include <atomic>
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
#include "audio_status.h"
#include "audio_decode_transport.h"
#include "audio_event.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "iaudio_data_transport.h"
#include "iaudio_datatrans_callback.h"
#include "iaudio_event_callback.h"

namespace OHOS {
namespace DistributedHardware {
class DSpeakerClient : public IAudioDataTransCallback,
    public AudioStandard::VolumeKeyEventCallback,
    public AudioStandard::AudioRendererCallback,
    public std::enable_shared_from_this<DSpeakerClient> {
public:
    DSpeakerClient(const std::string &devId, const std::shared_ptr<IAudioEventCallback> &callback)
        : devId_(devId), eventCallback_(callback) {};
    ~DSpeakerClient() override;

    int32_t OnStateChange(const AudioEventType type) override;
    void OnStateChange(const AudioStandard::RendererState state,
        const AudioStandard::StateChangeCmdType __attribute__((unused)) cmdType) override;
    int32_t OnDecodeTransDataDone(const std::shared_ptr<AudioData> &audioData) override;
    void OnVolumeKeyEvent(AudioStandard::VolumeEvent volumeEvent) override;
    void OnInterrupt(const AudioStandard::InterruptEvent &interruptEvent) override;
    int32_t SetUp(const AudioParam &param);
    int32_t Release();
    int32_t StartRender();
    int32_t StopRender();
    int32_t SetMute(const AudioEvent &event);
    int32_t SetAudioParameters(const AudioEvent &event);
    std::string GetVolumeLevel();
    void PlayStatusChange(const std::string &args);

private:
    void PlayThreadRunning();
    void Pause();
    void ReStart();

private:
    constexpr static size_t DATA_QUEUE_MAX_SIZE = 5;
    constexpr static size_t REQUEST_DATA_WAIT = 10;

    std::string devId_;
    std::thread renderDataThread_;
    AudioParam audioParam_;
    std::atomic<bool> isRenderReady_ = false;
    std::mutex dataQueueMtx_;
    std::mutex devMtx_;
    std::queue<std::shared_ptr<AudioData>> dataQueue_;
    std::condition_variable dataQueueCond_;
    AudioClientStatus clientStatus_ = CLIENT_STATUS_IDLE;

    std::unique_ptr<AudioStandard::AudioRenderer> audioRenderer_ = nullptr;
    std::shared_ptr<IAudioDataTransport> speakerTrans_ = nullptr;
    std::weak_ptr<IAudioEventCallback> eventCallback_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DSPEAKER_CLIENT_H
