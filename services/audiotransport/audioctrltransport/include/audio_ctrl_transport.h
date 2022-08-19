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

#ifndef OHOS_AUDIO_CTRL_TRANSPORT_H
#define OHOS_AUDIO_CTRL_TRANSPORT_H

#include <memory>

#include "iaudio_ctrl_trans_callback.h"
#include "iaudio_ctrl_transport.h"
#include "iaudio_channel.h"

#include "audio_event.h"
#include "audio_data.h"
#include "daudio_log.h"
#include "daudio_errorcode.h"

namespace OHOS {
namespace DistributedHardware {
class AudioCtrlTransport : public IAudioCtrlTransport,
    public IAudioChannelListener,
    public std::enable_shared_from_this<AudioCtrlTransport> {
public:
    explicit AudioCtrlTransport(const std::string &devId) : devId_(devId) {};
    ~AudioCtrlTransport() = default;

    int32_t SetUp(const std::shared_ptr<IAudioCtrlTransCallback> &callback) override;
    int32_t Release() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t SendAudioEvent(const std::shared_ptr<AudioEvent> &event) override;

    void OnSessionOpened() override;
    void OnSessionClosed() override;
    void OnDataReceived(const std::shared_ptr<AudioData> &data) override;
    void OnEventReceived(const std::shared_ptr<AudioEvent> &event) override;

private:
    int32_t InitAudioCtrlTrans(const std::string &devId);
    int32_t RegisterChannelListener();

private:
    static const constexpr char *LOG_TAG = "AudioCtrlTrans";

    const std::string devId_;
    std::weak_ptr<IAudioCtrlTransCallback> ctrlTransCallback_;
    std::shared_ptr<IAudioChannel> audioChannel_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AUDIO_CTRL_TRANSPORT_H
