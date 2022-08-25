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

#include "daudio_sink_proxy.h"

#include "daudio_errorcode.h"
#include "daudio_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSinkProxy"

namespace OHOS {
namespace DistributedHardware {
int32_t DAudioSinkProxy::InitSink(const std::string &params)
{
    DHLOGI("InitSink.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_DH_AUDIO_SA_WRITE_INTERFACE_TOKEN_FAILED;
    }

    if (!data.WriteString(params)) {
        return ERR_DH_AUDIO_SA_WRITE_PARAM_FAIED;
    }

    Remote()->SendRequest(INIT_SINK, data, reply, option);
    int32_t ret = reply.ReadInt32();
    return ret;
}

int32_t DAudioSinkProxy::ReleaseSink()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_DH_AUDIO_SA_WRITE_INTERFACE_TOKEN_FAILED;
    }

    Remote()->SendRequest(RELEASE_SINK, data, reply, option);
    int32_t ret = reply.ReadInt32();
    return ret;
}

int32_t DAudioSinkProxy::SubscribeLocalHardware(const std::string &dhId, const std::string
    &param)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_DH_AUDIO_SA_WRITE_INTERFACE_TOKEN_FAILED;
    }

    if (!data.WriteString(dhId) || !data.WriteString(param)) {
        return ERR_DH_AUDIO_SA_WRITE_PARAM_FAIED;
    }

    Remote()->SendRequest(SUBSCRIBE_LOCAL_HARDWARE, data, reply, option);
    int32_t ret = reply.ReadInt32();
    return ret;
}

int32_t DAudioSinkProxy::UnsubscribeLocalHardware(const std::string &dhId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_DH_AUDIO_SA_WRITE_INTERFACE_TOKEN_FAILED;
    }

    if (!data.WriteString(dhId)) {
        return ERR_DH_AUDIO_SA_WRITE_PARAM_FAIED;
    }

    Remote()->SendRequest(UNSUBSCRIBE_LOCAL_HARDWARE, data, reply, option);
    int32_t ret = reply.ReadInt32();
    return ret;
}

void DAudioSinkProxy::DAudioNotify(const std::string &devId, const std::string &dhId, const int32_t eventType,
    const std::string &eventContent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }

    if (!data.WriteString(devId) || !data.WriteString(dhId) || !data.WriteInt32(eventType) ||
        !data.WriteString(eventContent)) {
        return;
    }

    Remote()->SendRequest(DAUDIO_NOTIFY, data, reply, option);
}
} // namespace DistributedHardware
} // namespace OHOS