/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef DOWN_STREAM_ELEMENT_H
#define DOWN_STREAM_ELEMENT_H

#include <string>
#include <unordered_set>

#include "audio_data.h"
#include "ielement.h"

namespace OHOS {
namespace DistributedHardware {
const std::string SOURCE_REF = "source_ref";
const std::string SOURCE_MIC = "source_mic";
const std::string SINK_REF = "sink_ref";
const std::string SINK_MIC = "sink_mic";
const std::string RESAMPLER = "resampler";
const std::string AEC = "aec";
const std::string BUFFER_QUEUE = "buffer_queue";

class DownStreamElement : public IElement {
public:
    virtual ~DownStreamElement() {}
    int32_t AddDownStream(const std::shared_ptr<IElement> &element);
protected:
    const std::unordered_set<std::shared_ptr<IElement>>& GetDownStreams();
    virtual int32_t DoProcessData(std::shared_ptr<AudioData> &inData, std::shared_ptr<AudioData> &outData) = 0;

private:
    std::unordered_set<std::shared_ptr<IElement>> downStreams_;
};
} // DistributedHardware
} // OHOS
#endif // DOWN_STREAM_ELEMENT_H