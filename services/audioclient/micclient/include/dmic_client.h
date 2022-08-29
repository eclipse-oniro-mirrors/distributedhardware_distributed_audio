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

#ifndef OHOS_DMIC_CLIENT_H
#define OHOS_DMIC_CLIENT_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "audio_capturer.h"
#include "audio_info.h"

#include "audio_data.h"
#include "audio_encode_transport.h"
#include "audio_event.h"
#include "audio_param.h"
#include "audio_status.h"
#include "daudio_errorcode.h"
#include "daudio_log.h"
#include "iaudio_data_transport.h"
#include "iaudio_datatrans_callback.h"
#include "iaudio_event_callback.h"

namespace OHOS {
namespace DistributedHardware {
class DMicClient : public IAudioDataTransCallback, public std::enable_shared_from_this<DMicClient> {
public:
    DMicClient(const std::string &devId, const std::shared_ptr<IAudioEventCallback> &callback)
        : devId_(devId), eventCallback_(callback) {};
    ~DMicClient();
    int32_t OnStateChange(const AudioEventType type) override;
    int32_t OnDecodeTransDataDone(const std::shared_ptr<AudioData> &audioData) override;
    int32_t SetUp(const AudioParam &param);
    int32_t Release();
    int32_t StartCapture();
    int32_t StopCapture();

private:
    void CaptureThreadRunning();

private:
    constexpr static uint8_t CHANNEL_WAIT_SECONDS = 5;

    std::string devId_;
    std::mutex channelWaitMutex_;
    std::thread captureDataThread_;
    std::condition_variable channelWaitCond_;
    AudioParam audioParam_;
    bool isBlocking_ = false;
    bool isChannelReady_ = false;
    std::atomic<bool> isCaptureReady_ = false;
    std::mutex devMtx_;
    AudioClientStatus clientStatus_ = CLIENT_STATUS_IDLE;

    std::weak_ptr<IAudioEventCallback> eventCallback_;
    std::unique_ptr<AudioStandard::AudioCapturer> audioCapturer_ = nullptr;
    std::shared_ptr<IAudioDataTransport> micTrans_ = nullptr;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DMIC_CLIENT_H
