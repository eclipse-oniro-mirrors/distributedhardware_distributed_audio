/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DAUDIO_SINK_MANAGER_H
#define OHOS_DAUDIO_SINK_MANAGER_H

#include <map>
#include <mutex>

#include "single_instance.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#ifdef DEVICE_SECURITY_LEVEL_ENABLE
#include "device_security_defines.h"
#include "device_security_info.h"
#endif

#include "daudio_ctrl_channel_listener.h"
#include "daudio_sink_dev.h"
#include "idaudio_source.h"
#include "idaudio_sink_ipc_callback.h"
#include "i_av_engine_provider_callback.h"

namespace OHOS {
namespace DistributedHardware {
class EngineProviderListener : public IAVEngineProviderCallback {
public:
    EngineProviderListener() {};
    ~EngineProviderListener() override {};

    int32_t OnProviderEvent(const AVTransEvent &event) override;
};

class CtrlChannelListener : public CtrlChannelListenerCallback {
public:
    CtrlChannelListener() {};
    ~CtrlChannelListener() override {};

    void OnCtrlChannelEvent(const AVTransEvent &event) override;
};

class DeviceInitCallback : public DmInitCallback {
    void OnRemoteDied() override;
};

class DAudioSinkManager : public std::enable_shared_from_this<DAudioSinkManager> {
DECLARE_SINGLE_INSTANCE_BASE(DAudioSinkManager);
public:
    int32_t Init(const sptr<IDAudioSinkIpcCallback> &sinkCallback);
    int32_t UnInit();
    int32_t HandleDAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
        const std::string &eventContent);
    int32_t DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
        const std::string &eventContent);
    void OnSinkDevReleased(const std::string &devId);
    void NotifyEvent(const std::string &devId, const int32_t eventType, const std::string &eventContent);
    void ClearAudioDev(const std::string &devId);
    int32_t CreateAudioDevice(const std::string &devId);
    void SetChannelState(const std::string &content);
    int32_t PauseDistributedHardware(const std::string &networkId);
    int32_t ResumeDistributedHardware(const std::string &networkId);
    int32_t StopDistributedHardware(const std::string &networkId);
    void SetCallerTokenId(uint64_t tokenId);
    int32_t CheckOsType(const std::string &networkId, bool &isInvalid);

private:
    DAudioSinkManager();
    ~DAudioSinkManager();
    int32_t LoadAVSenderEngineProvider();
    int32_t UnloadAVSenderEngineProvider();
    int32_t LoadAVReceiverEngineProvider();
    int32_t UnloadAVReceiverEngineProvider();
    bool CheckDeviceSecurityLevel(const std::string &srcDeviceId, const std::string &dstDeviceId);
    int32_t GetDeviceSecurityLevel(const std::string &udid);
    std::string GetUdidByNetworkId(const std::string &networkId);
    int32_t VerifySecurityLevel(const std::string &devId);
    int32_t InitAudioDevice(std::shared_ptr<DAudioSinkDev> dev, const std::string &devId, bool isSpkOrMic);
    int32_t ParseValueFromCjson(std::string args, std::string key);

private:
    static constexpr const char* DEVCLEAR_THREAD = "sinkClearTh";
    std::mutex devMapMutex_;
    std::unordered_map<std::string, std::shared_ptr<DAudioSinkDev>> audioDevMap_;
    std::mutex remoteSvrMutex_;
    std::mutex ipcCallbackMutex_;
    std::map<std::string, sptr<IDAudioSource>> sourceServiceMap_;
    std::thread devClearThread_;
    std::string localNetworkId_;
    ChannelState channelState_ = ChannelState::UNKNOWN;

    std::shared_ptr<EngineProviderListener> providerListener_ = nullptr;
    std::shared_ptr<CtrlChannelListener> ctrlListenerCallback_ = nullptr;
    std::shared_ptr<DaudioCtrlChannelListener> ctrlListener_ = nullptr;
    IAVEngineProvider *sendProviderPtr_ = nullptr;
    IAVEngineProvider *rcvProviderPtr_ = nullptr;
    void *pSHandler_ = nullptr;
    void *pRHandler_ = nullptr;
    bool isSensitive_ = false;
    bool isSameAccount_ = false;
    bool isCheckSecLevel_ = false;
    sptr<IDAudioSinkIpcCallback> ipcSinkCallback_ = nullptr;
    std::shared_ptr<DmInitCallback> initCallback_ = nullptr;
    uint64_t callerTokenId_ = 0;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DAUDIO_SINK_MANAGER_H
