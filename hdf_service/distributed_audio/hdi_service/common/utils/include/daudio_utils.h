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

#ifndef OHOS_DAUDIO_UTILS_H
#define OHOS_DAUDIO_UTILS_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
std::string GetAnonyString(const std::string &value);

int32_t GetAudioParamStr(const std::string &params, const std::string &key, std::string &value);

int32_t GetAudioParamInt(const std::string &params, const std::string &key, int32_t &value);

int32_t GetAudioParamUInt(const std::string &params, const std::string &key, uint32_t &value);

int32_t GetAudioParamBool(const std::string &params, const std::string &key, bool &value);

int32_t SetAudioParamStr(std::string &params, const std::string &key, const std::string &value);

int32_t GetDevTypeByDHId(int32_t dhId);
} // DistributedHardware
} // OHOS
#endif