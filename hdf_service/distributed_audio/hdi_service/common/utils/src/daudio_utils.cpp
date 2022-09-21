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

#include "daudio_utils.h"

#include "daudio_constants.h"
#include "daudio_errcode.h"

namespace OHOS {
namespace DistributedHardware {
std::string GetAnonyString(const std::string &value)
{
    constexpr size_t INT32_SHORT_ID_LENGTH = 20;
    constexpr size_t INT32_PLAINTEXT_LENGTH = 4;
    constexpr size_t INT32_MIN_ID_LENGTH = 3;
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

int32_t GetAudioParamStr(const std::string &params, const std::string &key, std::string &value)
{
    size_t step = key.size();
    if (step >= params.size()) {
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    size_t pos = params.find(key);
    if (pos == params.npos || params.at(pos + step) != '=') {
        return ERR_DH_AUDIO_COMMON_NOT_FOUND_KEY;
    }
    size_t splitPosEnd = params.find(';', pos);
    if (splitPosEnd != params.npos) {
        value = params.substr(pos + step + 1, splitPosEnd - pos - step - 1);
    } else {
        value = params.substr(pos + step + 1);
    }
    return DH_SUCCESS;
}

int32_t GetAudioParamInt(const std::string &params, const std::string &key, int32_t &value)
{
    std::string val = "0";
    int32_t ret = GetAudioParamStr(params, key, val);
    value = std::stoi(val);
    return ret;
}

int32_t GetAudioParamUInt(const std::string &params, const std::string &key, uint32_t &value)
{
    value = 0;
    return DH_SUCCESS;
}

int32_t GetAudioParamBool(const std::string &params, const std::string &key, bool &value)
{
    std::string val;
    GetAudioParamStr(params, key, val);
    value = (val != "0");
    return DH_SUCCESS;
}

int32_t SetAudioParamStr(std::string &params, const std::string &key, const std::string &value)
{
    params = params + key + '=' + value + ';';
    return DH_SUCCESS;
}

int32_t GetDevTypeByDHId(int32_t dhId)
{
    if ((uint32_t)dhId & 0x8000000) {
        return AUDIO_DEVICE_TYPE_MIC;
    } else if ((uint32_t)dhId & 0x7ffffff) {
        return AUDIO_DEVICE_TYPE_SPEAKER;
    }
    return AUDIO_DEVICE_TYPE_UNKNOWN;
}
} // namespace DistributedHardware
} // namespace OHOS