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

#include "daudio_adapter_internal.h"

#include <securec.h>
#include <string>
#include <v1_0/iaudio_render.h>
#include <v1_0/iaudio_capture.h>
#include <v1_0/types.h>

#include "audio_types.h"
#include "daudio_errcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture;

static const char * const AUDIO_LOG = "DAudioAdapterInternal";

static int32_t InitAllPortsInternal(struct AudioAdapter *adapter)
{
    if (adapter == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }
    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    return context->proxy_->InitAllPorts();
}

static void SetAudioSampleAttributesHAL(const struct AudioSampleAttributes *attrs, AudioSampleAttributesHAL &attrsHal)
{
    attrsHal.type = static_cast<uint32_t>(attrs->type);
    attrsHal.interleaved = static_cast<uint32_t>(attrs->interleaved);
    attrsHal.format = static_cast<uint32_t>(attrs->format);
    attrsHal.sampleRate = attrs->sampleRate;
    attrsHal.channelCount = attrs->channelCount;

    attrsHal.period = attrs->period;
    attrsHal.frameSize = attrs->frameSize;
    attrsHal.isBigEndian = static_cast<uint32_t>(attrs->isBigEndian);
    attrsHal.isSignedData = static_cast<uint32_t>(attrs->isSignedData);
    attrsHal.startThreshold = attrs->startThreshold;
    attrsHal.stopThreshold = attrs->stopThreshold;
    attrsHal.silenceThreshold = attrs->silenceThreshold;
    attrsHal.streamId = static_cast<int32_t>(attrs->streamId);
}

static int32_t CreateRenderInternal(struct AudioAdapter *adapter, const struct AudioDeviceDescriptor *desc,
    const struct AudioSampleAttributes *attrs, struct AudioRender **render)
{
    DHLOGI("%s: CreateRenderInternal enter.", AUDIO_LOG);
    if (adapter == nullptr || desc == nullptr || attrs == nullptr || render == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    AudioDeviceDescriptorHAL descHal = {
        .portId = desc->portId,
        .pins = desc->pins,
    };
    if (desc->desc == nullptr) {
        descHal.desc = "";
    } else {
        descHal.desc = desc->desc;
    }

    AudioSampleAttributesHAL attrsHal;
    SetAudioSampleAttributesHAL(attrs, attrsHal);
    sptr<IAudioRender> renderProxy = nullptr;
    int32_t ret = context->proxy_->CreateRender(descHal, attrsHal, renderProxy);
    if (ret != DH_SUCCESS) {
        *render = nullptr;
        return ret;
    }
    std::unique_ptr<AudioRenderContext> renderContext = std::make_unique<AudioRenderContext>();
    renderContext->proxy_ = renderProxy;
    *render = &renderContext->instance_;
    renderContext->descHal_ = descHal;
    {
        std::lock_guard<std::mutex> lock(context->mtx_);
        context->renders_.push_back(std::move(renderContext));
    }
    return DH_SUCCESS;
}

static int32_t DestroyRenderInternal(struct AudioAdapter *adapter, struct AudioRender *render)
{
    DHLOGI("%s: Enter to destroyRenderInternal.", AUDIO_LOG);
    if (adapter == nullptr || render == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *adapterContext = reinterpret_cast<AudioAdapterContext *>(adapter);
    AudioRenderContext *renderContext = reinterpret_cast<AudioRenderContext *>(render);
    std::lock_guard<std::mutex> lock(adapterContext->mtx_);

    for (auto it = adapterContext->renders_.begin(); it != adapterContext->renders_.end(); ++it) {
        if ((*it).get() == renderContext) {
            int32_t ret = adapterContext->proxy_->DestoryRender(renderContext->descHal_);
            if (ret != DH_SUCCESS) {
                return ret;
            }
            adapterContext->renders_.erase(it);
            break;
        }
    }
    return DH_SUCCESS;
}

static int32_t CreateCaptureInternal(struct AudioAdapter *adapter, const struct AudioDeviceDescriptor *desc,
    const struct AudioSampleAttributes *attrs, struct AudioCapture **capture)
{
    DHLOGI("%s: Dnter to createCaptureInternal.", AUDIO_LOG);
    if (adapter == nullptr || desc == nullptr || attrs == nullptr || capture == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    AudioDeviceDescriptorHAL descHal = {
        .portId = desc->portId,
        .pins = desc->pins,
    };
    if (desc->desc == nullptr) {
        descHal.desc = "";
    } else {
        descHal.desc = desc->desc;
    }
    AudioSampleAttributesHAL attrsHal;
    SetAudioSampleAttributesHAL(attrs, attrsHal);
    sptr<IAudioCapture> captureProxy = nullptr;
    int32_t ret = context->proxy_->CreateCapture(descHal, attrsHal, captureProxy);
    if (ret != DH_SUCCESS) {
        *capture = nullptr;
        return ret;
    }

    std::unique_ptr<AudioCaptureContext> captureContext = std::make_unique<AudioCaptureContext>();
    captureContext->proxy_ = captureProxy;
    *capture = &captureContext->instance_;
    captureContext->descHal_ = descHal;
    {
        std::lock_guard<std::mutex> lock(context->mtx_);
        context->captures_.push_back(std::move(captureContext));
    }
    return DH_SUCCESS;
}

static int32_t DestroyCaptureInternal(struct AudioAdapter *adapter, struct AudioCapture *capture)
{
    DHLOGI("%s: Enter to destroyCaptureInternal.", AUDIO_LOG);
    if (adapter == nullptr || capture == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *adapterContext = reinterpret_cast<AudioAdapterContext *>(adapter);
    AudioCaptureContext *captureContext = reinterpret_cast<AudioCaptureContext *>(capture);
    std::lock_guard<std::mutex> lock(adapterContext->mtx_);

    for (auto it = adapterContext->captures_.begin(); it != adapterContext->captures_.end(); ++it) {
        if ((*it).get() == captureContext) {
            int32_t ret = adapterContext->proxy_->DestoryCapture(captureContext->descHal_);
            if (ret != DH_SUCCESS) {
                return ret;
            }
            adapterContext->captures_.erase(it);
            break;
        }
    }
    return DH_SUCCESS;
}

static int32_t GetPassthroughModeInternal(struct AudioAdapter *adapter, const struct AudioPort *port,
    enum AudioPortPassthroughMode *mode)
{
    if (adapter == nullptr || port == nullptr || mode == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    AudioPortHAL portHal = {
        .dir = port->dir,
        .portId = port->portId,
        .portName= port->portName,
    };
    return context->proxy_->GetPassthroughMode(portHal, *(reinterpret_cast<AudioPortPassthroughModeHAL *>(mode)));
}

static int32_t InitAudioPortCapability(std::unique_ptr<AudioPortCapability> &capInternal,
    AudioPortCapabilityHAl &capabilityHal)
{
    AudioFormat *audioFormats = (AudioFormat *)malloc(capabilityHal.formatNum * sizeof(AudioFormat));
    if (audioFormats == nullptr) {
        DHLOGE("%s:Malloc failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAILURE;
    }

    capInternal->deviceType = capabilityHal.deviceType;
    capInternal->deviceId = capabilityHal.deviceId;
    capInternal->hardwareMode = static_cast<bool>(capabilityHal.hardwareMode);
    capInternal->formatNum = capabilityHal.formatNum;
    capInternal->formats = audioFormats;
    for (auto format : capabilityHal.formats) {
        *audioFormats = static_cast<AudioFormat>(format);
        audioFormats++;
    }
    capInternal->sampleRateMasks = capabilityHal.sampleRateMasks;
    capInternal->channelMasks = static_cast<AudioChannelMask>(capabilityHal.channelMasks);
    capInternal->channelCount = capabilityHal.channelCount;
    capInternal->subPortsNum = 0;
    capInternal->subPorts = nullptr;
    return DH_SUCCESS;
}

static int32_t GetPortCapabilityInternal(struct AudioAdapter *adapter, const struct AudioPort *port,
    struct AudioPortCapability *capability)
{
    if (adapter == nullptr || port == nullptr || port->portName == nullptr || capability == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    {
        std::lock_guard<std::mutex> lock(context->mtx_);
        auto iter = context->caps_.find(port->portId);
        if (iter != context->caps_.end()) {
            *capability = *(iter->second);
            return DH_SUCCESS;
        }
    }
    AudioPortHAL portHal = {
        .dir = port->dir,
        .portId = port->portId,
        .portName = port->portName,
    };

    AudioPortCapabilityHAl capabilityHal {};
    int32_t ret = context->proxy_->GetPortCapability(portHal, capabilityHal);
    if (ret != DH_SUCCESS) {
        return ret;
    }

    std::unique_ptr<AudioPortCapability> capInternal = std::make_unique<AudioPortCapability>();
    ret = InitAudioPortCapability(capInternal, capabilityHal);
    if (ret != DH_SUCCESS) {
        return ret;
    }
    *capability = *capInternal;
    {
        std::lock_guard<std::mutex> lock(context->mtx_);
        context->caps_[port->portId] = std::move(capInternal);
    }
    return DH_SUCCESS;
}

static int32_t ReleaseAudioRouteInternal(struct AudioAdapter *adapter, int32_t routeHandle)
{
    if (adapter == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    return context->proxy_->ReleaseAudioRoute(routeHandle);
}

static int32_t SetPassthroughModeInternal(struct AudioAdapter *adapter, const struct AudioPort *port,
    enum AudioPortPassthroughMode mode)
{
    if (adapter == nullptr || port == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    AudioPortHAL portHal = {
        .dir = port->dir,
        .portId = port->portId,
        .portName = port->portName,
    };
    return context->proxy_->SetPassthroughMode(portHal, static_cast<AudioPortPassthroughModeHAL>(mode));
}

static void ConvertAudioRouteNodeToHAL(const AudioRouteNode &node, AudioRouteNodeHAL &halNode)
{
    halNode.portId = node.portId;
    halNode.role = static_cast<uint32_t>(node.role);
    halNode.type = static_cast<uint32_t>(node.type);
    size_t descLength = 32;
    DHLOGI("%s: ConvertAudioRouteNodeToHAL portId: %d role: %d type: %d.", AUDIO_LOG, halNode.portId, halNode.role,
        halNode.type);

    switch (node.type) {
        case AUDIO_PORT_UNASSIGNED_TYPE:
            break;
        case AUDIO_PORT_DEVICE_TYPE: {
            halNode.device.moduleId = node.ext.device.moduleId;
            halNode.device.type = static_cast<uint32_t>(node.ext.device.type);
            if (node.ext.device.desc != nullptr) {
                size_t length = strlen(node.ext.device.desc);
                length = length < descLength ? length : descLength;
                halNode.device.desc = std::vector<uint8_t>(node.ext.device.desc, node.ext.device.desc + length);
            }
            break;
        }
        case AUDIO_PORT_MIX_TYPE: {
            halNode.mix.moduleId = node.ext.mix.moduleId;
            halNode.mix.streamId = node.ext.mix.streamId;

            DHLOGI("%s: ConvertAudioRouteNodeToHAL [Mix] moduleId: %d streamId: %d.", AUDIO_LOG,
                halNode.mix.moduleId, halNode.mix.streamId);
            break;
        }
        case AUDIO_PORT_SESSION_TYPE: {
            halNode.session.sessionType = static_cast<uint32_t>(node.ext.session.sessionType);
            DHLOGI("%s: ConvertAudioRouteNodeToHAL [Session] sessionType: %d.", AUDIO_LOG, halNode.session.sessionType);
            break;
        }
    }
}
static int32_t UpdateAudioRouteInternal(struct AudioAdapter *adapter, const struct AudioRoute *route,
    int32_t *routeHandle)
{
    if (adapter == nullptr || route == nullptr || routeHandle == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioRouteHAL audioRouteHal;
    for (uint32_t i = 0; i < route->sourcesNum; ++i) {
        AudioRouteNodeHAL halNode = {0};
        ConvertAudioRouteNodeToHAL(route->sources[i], halNode);
        audioRouteHal.sources.push_back(halNode);
    }

    for (uint32_t i = 0; i < route->sinksNum; ++i) {
        AudioRouteNodeHAL halNode = {0};
        ConvertAudioRouteNodeToHAL(route->sinks[i], halNode);
        audioRouteHal.sinks.push_back(halNode);
    }

    int32_t handle = -1;
    AudioAdapterContext *context = reinterpret_cast<AudioAdapterContext *>(adapter);
    int32_t ret = context->proxy_->UpdateAudioRoute(audioRouteHal, handle);
    *routeHandle = handle;
    return ret;
}

AudioAdapterContext::AudioAdapterContext()
{
    instance_.InitAllPorts = InitAllPortsInternal;
    instance_.CreateRender = CreateRenderInternal;
    instance_.DestroyRender = DestroyRenderInternal;
    instance_.CreateCapture = CreateCaptureInternal;
    instance_.DestroyCapture = DestroyCaptureInternal;
    instance_.GetPassthroughMode = GetPassthroughModeInternal;
    instance_.GetPortCapability = GetPortCapabilityInternal;
    instance_.ReleaseAudioRoute = ReleaseAudioRouteInternal;
    instance_.SetPassthroughMode = SetPassthroughModeInternal;
    instance_.UpdateAudioRoute = UpdateAudioRouteInternal;
}

AudioAdapterContext::~AudioAdapterContext()
{
    captures_.clear();
    renders_.clear();
    for (auto &cap : caps_) {
        if (cap.second->formats != nullptr) {
            free(cap.second->formats);
        }
    }
    caps_.clear();
}
} // namespace DistributedHardware
} // namespace OHOS
