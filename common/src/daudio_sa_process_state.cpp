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

#include "daudio_sa_process_state.h"

#include <cstdint>
#include <cstdlib>
#include <mutex>

#include "daudio_constants.h"
#include "daudio_hisysevent.h"
#include "daudio_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioSAProcessState"

namespace OHOS {
namespace DistributedHardware {
enum DAudioSAState : uint32_t {
    DAUDIO_SA_EXIT_STATE_STOP = 0,
    DAUDIO_SA_EXIT_STATE_START = 1,
};
DAudioSAState g_sinkSAState = DAUDIO_SA_EXIT_STATE_START;
DAudioSAState g_sourceSAState = DAUDIO_SA_EXIT_STATE_START;
std::mutex g_saProcessState;

void SetSinkProcessExit()
{
    DHLOGI("Set sink process exit.");
    std::lock_guard<std::mutex> autoLock(g_saProcessState);
    g_sinkSAState = DAUDIO_SA_EXIT_STATE_STOP;
    DHLOGI("Source SA state [%d], sink SA state [%d].", g_sourceSAState, g_sinkSAState);
    if (g_sourceSAState == DAUDIO_SA_EXIT_STATE_START || g_sinkSAState == DAUDIO_SA_EXIT_STATE_START) {
        return;
    }
    DHLOGI("Exit SA process success.");
    _Exit(0);
}

void SetSourceProcessExit()
{
    DHLOGI("Set sources process exit.");
    std::lock_guard<std::mutex> autoLock(g_saProcessState);
    g_sourceSAState = DAUDIO_SA_EXIT_STATE_STOP;
    DHLOGI("Source SA state [%d], sink SA state [%d].", g_sourceSAState, g_sinkSAState);
    if (g_sourceSAState == DAUDIO_SA_EXIT_STATE_START || g_sinkSAState == DAUDIO_SA_EXIT_STATE_START) {
        return;
    }
    DHLOGI("Exit SA process success.");
    _Exit(0);
}
} // namespace DistributedHardware
} // namespace OHOS
