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

#include "audio_capture_interface_impl_base.h"

#include <hdf_base.h>
#include <unistd.h>
#include "sys/time.h"
#include <securec.h>

#include "daudio_constants.h"
#include "daudio_log.h"
#include "daudio_utils.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioCaptureInterfaceImplBase"

using namespace OHOS::DistributedHardware;
namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
AudioCaptureInterfaceImplBase::AudioCaptureInterfaceImplBase(const AudioDeviceDescriptor &desc)
    : baseDevDesc_(desc)
{
    DHLOGD("Distributed audio capture base constructed, id(%d).", baseDevDesc_.pins);
}

AudioCaptureInterfaceImplBase::~AudioCaptureInterfaceImplBase()
{
    DHLOGD("Distributed audio capture base destructed, id(%d).", baseDevDesc_.pins);
}

const AudioDeviceDescriptor &AudioCaptureInterfaceImplBase::GetCaptureDesc()
{
    return baseDevDesc_;
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
