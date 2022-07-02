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

#ifndef HDI_DAUDIO_ATTRIBUTE_INTERNAL_H
#define HDI_DAUDIO_ATTRIBUTE_INTERNAL_H

#include <securec.h>
#include <stdint.h>
#include <sys/mman.h>
#include <v1_0/types.h>

#include "audio_types.h"
#include "daudio_errcode.h"
#include "daudio_log.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;
template<typename T>
class AudioAttributeInternal final {
public:
    static int32_t GetFrameSize(AudioHandle handle, uint64_t *size);
    static int32_t GetFrameCount(AudioHandle handle, uint64_t *count);
    static int32_t SetSampleAttributes(AudioHandle handle, const struct AudioSampleAttributes *attrs);
    static int32_t GetSampleAttributes(AudioHandle handle, struct AudioSampleAttributes *attrs);
    static int32_t GetCurrentChannelId(AudioHandle handle, uint32_t *channelId);
    static int32_t SetExtraParams(AudioHandle handle, const char *keyValueList);
    static int32_t GetExtraParams(AudioHandle handle, char *keyValueList, int32_t listLenth);
    static int32_t ReqMmapBuffer(AudioHandle handle, int32_t reqSize, struct AudioMmapBufferDescripter *desc);
    static int32_t GetMmapPosition(AudioHandle handle, uint64_t *frames, struct AudioTimeStamp *time);

public:
    static const char *AUDIO_LOG;
};

template<typename T>
const char *AudioAttributeInternal<T>::AUDIO_LOG = "AudioAttributeInternal";

template<typename T>
int32_t AudioAttributeInternal<T>::GetFrameSize(AudioHandle handle, uint64_t *size)
{
    if (handle == nullptr || size == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    return (context == nullptr) ? ERR_DH_AUDIO_HDF_INVALID_PARAM : context->proxy_->GetFrameSize(*size);
}

template<typename T>
int32_t AudioAttributeInternal<T>::GetFrameCount(AudioHandle handle, uint64_t *count)
{
    if (handle == nullptr || count == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    return (context == nullptr) ? ERR_DH_AUDIO_HDF_INVALID_PARAM : context->proxy_->GetFrameCount(*count);
}

template<typename T>
int32_t AudioAttributeInternal<T>::SetSampleAttributes(AudioHandle handle, const struct AudioSampleAttributes *attrs)
{
    if (handle == nullptr || attrs == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    AudioSampleAttributesHAL attrsHal = {
        .format = static_cast<uint32_t>(attrs->format),
        .sampleRate = attrs->sampleRate,
        .channelCount = attrs->channelCount,
    };
    DHLOGD("%s: AttrsHal.format = %u", AUDIO_LOG, attrsHal.format);
    return (context == nullptr) ? ERR_DH_AUDIO_HDF_INVALID_PARAM : context->proxy_->SetSampleAttributes(attrsHal);
}

template<typename T>
int32_t AudioAttributeInternal<T>::GetSampleAttributes(AudioHandle handle, struct AudioSampleAttributes *attrs)
{
    if (handle == nullptr || attrs == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    DHLOGD("%s: Enter to  GetSampleAttributes.", AUDIO_LOG);
    T *context = reinterpret_cast<T *>(handle);
    if (context == nullptr) {
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioSampleAttributesHAL attrsHal;
    DHLOGD("%s: GetSampleAttributes call.", AUDIO_LOG);
    int32_t ret = context->proxy_->GetSampleAttributes(attrsHal);
    if (ret != DH_SUCCESS) {
        return ret;
    }
    DHLOGD("%s: GetSampleAttributes call sucess.", AUDIO_LOG);

    attrs->type = static_cast<AudioCategory>(attrsHal.type);
    attrs->interleaved = static_cast<bool>(attrsHal.interleaved);
    attrs->format = static_cast<AudioFormat>(attrsHal.format);
    attrs->sampleRate = attrsHal.sampleRate;
    attrs->channelCount = attrsHal.channelCount;
    attrs->streamId = attrsHal.streamId;
    return DH_SUCCESS;
}

template<typename T>
int32_t AudioAttributeInternal<T>::GetCurrentChannelId(AudioHandle handle, uint32_t *channelId)
{
    if (handle == nullptr || channelId == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    return (context == nullptr) ? ERR_DH_AUDIO_HDF_INVALID_PARAM : context->proxy_->GetCurrentChannelId(*channelId);
}

template<typename T>
int32_t AudioAttributeInternal<T>::SetExtraParams(AudioHandle handle, const char *keyValueList)
{
    if (handle == nullptr || keyValueList == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    std::string keyValueListHal(keyValueList);
    return (context == nullptr) ? ERR_DH_AUDIO_HDF_INVALID_PARAM : context->proxy_->SetExtraParams(keyValueListHal);
}

template<typename T>
int32_t AudioAttributeInternal<T>::GetExtraParams(AudioHandle handle, char *keyValueList, int32_t listLenth)
{
    if (handle == nullptr || keyValueList == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    if (listLenth <= 0) {
        DHLOGE("%s:The parameter is invalid.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    if (context == nullptr) {
        DHLOGE("%s:The context is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    std::string keyValueListHal(keyValueList);
    int32_t ret = context->proxy_->GetExtraParams(keyValueListHal);
    if (ret != DH_SUCCESS) {
        return ret;
    }
    if (listLenth - 1 < (int)keyValueListHal.length()) {
        keyValueListHal = keyValueListHal.substr(0, listLenth - 1);
    }
    if (strcpy_s(keyValueList, listLenth, keyValueListHal.c_str()) != EOK) {
        DHLOGE("%s:Strcpy_s keyValueList failed.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    return DH_SUCCESS;
}

template<typename T>
int32_t AudioAttributeInternal<T>::ReqMmapBuffer(AudioHandle handle, int32_t reqSize,
    struct AudioMmapBufferDescripter *desc)
{
    if (handle == nullptr || desc == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    T *context = reinterpret_cast<T *>(handle);
    if (context == nullptr) {
        DHLOGE("%s:The context is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }

    AudioMmapBufferDescripterHAL descHal;
    int32_t ret = context->proxy_->ReqMmapBuffer(reqSize, descHal);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:The ReqMmapBuffer is failed.", AUDIO_LOG);
        return ret;
    }

    desc->memoryAddress = mmap(0, descHal.totalBufferFrames, PROT_READ | PROT_WRITE, MAP_SHARED, descHal.memoryFd, 0);
    if (desc->memoryAddress == MAP_FAILED) {
        DHLOGD("%s: ReqMmapBuffer mmap error!", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_FAILURE;
    }
    desc->memoryFd = descHal.memoryFd;
    desc->totalBufferFrames = descHal.totalBufferFrames;
    desc->transferFrameSize = descHal.transferFrameSize;
    desc->isShareable = descHal.isShareable;
    return DH_SUCCESS;
}

template<typename T>
int32_t AudioAttributeInternal<T>::GetMmapPosition(AudioHandle handle, uint64_t *frames, struct AudioTimeStamp *time)
{
    if (handle == nullptr || frames == nullptr || time == nullptr) {
        DHLOGE("%s:The parameter is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }
    DHLOGD("%s: Enter to getMmapPosition.", AUDIO_LOG);

    T *context = reinterpret_cast<T *>(handle);
    if (context == nullptr) {
        DHLOGE("%s:The context is empty.", AUDIO_LOG);
        return ERR_DH_AUDIO_HDF_INVALID_PARAM;
    }
    DHLOGD("%s: Call to getMmapPosition.", AUDIO_LOG);

    AudioTimeStampHAL timeHal;
    int32_t ret = context->proxy_->GetMmapPosition(*frames, timeHal);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s:The GetMmapPosition is failed.", AUDIO_LOG);
        return ret;
    }
    DHLOGD("%s: Call to getMmapPosition sucess.", AUDIO_LOG);

    time->tvSec = timeHal.tvSec;
    time->tvNSec = timeHal.tvNSec;
    return DH_SUCCESS;
}
} // DistributedHardware
} // OHOS
#endif // HDI_DAUDIO_ATTRIBUTE_INTERNAL_H
