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

#include "audio_render_interface_impl_test.h"

#include <hdf_base.h>
#include <unistd.h>
#include "sys/time.h"

#include "daudio_constants.h"
#include "daudio_log.h"

using namespace testing::ext;

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
void AudioRenderInterfaceImplTest::SetUpTestCase(void) {}

void AudioRenderInterfaceImplTest::TearDownTestCase(void) {}

void AudioRenderInterfaceImplTest::SetUp(void)
{
    audioRenderInterfaceImpl_ = std::make_shared<AudioRenderInterfaceImpl>(adpName_, desc_, attrs_, callback_);
}

void AudioRenderInterfaceImplTest::TearDown(void)
{
    audioRenderInterfaceImpl_ = nullptr;
}

/**
 * @tc.name: GetLatency_001
 * @tc.desc: Verify the GetLatency function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetLatency_001, TestSize.Level1)
{
    uint32_t ms = 12;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetLatency(ms));
}

/**
 * @tc.name: RenderFrame_001
 * @tc.desc: Verify the RenderFrame function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, RenderFrame_001, TestSize.Level1)
{
    std::vector<int8_t> frame;
    uint64_t replyBytes = 0;
    EXPECT_EQ(HDF_FAILURE, audioRenderInterfaceImpl_->RenderFrame(frame, replyBytes));
}

/**
 * @tc.name: GetRenderPosition_001
 * @tc.desc: Verify the GetRenderPosition function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetRenderPosition_001, TestSize.Level1)
{
    uint64_t frames = 0;
    AudioTimeStamp time;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetRenderPosition(frames, time));
}

/**
 * @tc.name: SetRenderSpeedr_001
 * @tc.desc: Verify the SetRenderSpeed function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SetRenderSpeed_001, TestSize.Level1)
{
    float speed = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SetRenderSpeed(speed));
}

/**
 * @tc.name: GetRenderSpeed_001
 * @tc.desc: Verify the GetRenderSpeed function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetRenderSpeed_001, TestSize.Level1)
{
    float speed;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetRenderSpeed(speed));
}

/**
 * @tc.name: SetChannelMode_001
 * @tc.desc: Verify the SetChannelMode function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SetChannelMode_001, TestSize.Level1)
{
    AudioChannelMode mode = AudioChannelMode::AUDIO_CHANNEL_NORMAL;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SetChannelMode(mode));
}

/**
 * @tc.name: GetChannelMode_001
 * @tc.desc: Verify the GetChannelMode function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetChannelMode_001, TestSize.Level1)
{
    AudioChannelMode mode = AudioChannelMode::AUDIO_CHANNEL_MIX;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetChannelMode(mode));
}

/**
 * @tc.name: RegCallback_001
 * @tc.desc: Verify the RegCallback function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, RegCallback_001, TestSize.Level1)
{
    sptr<IAudioCallback> cbObj = nullptr;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->RegCallback(cbObj, 0));
}

/**
 * @tc.name: DrainBuffer_001
 * @tc.desc: Verify the DrainBuffer function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, DrainBuffer_001, TestSize.Level1)
{
    AudioDrainNotifyType type = AudioDrainNotifyType::AUDIO_DRAIN_EARLY_MODE;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->DrainBuffer(type));
}

/**
 * @tc.name: Start_001
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, Start_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->Start());
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Verify the Stop function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, Stop_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->Stop());
}

/**
 * @tc.name: Pause_001
 * @tc.desc: Verify the Pause function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, Pause_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->Pause());
}

/**
 * @tc.name: Resume_001
 * @tc.desc: Verify the Resume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, Resume_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->Resume());
}

/**
 * @tc.name: Flush_001
 * @tc.desc: Verify the Flush function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, Flush_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->Flush());
}

/**
 * @tc.name: TurnStandbyMode_001
 * @tc.desc: Verify the TurnStandbyMode function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, TurnStandbyMode_001, TestSize.Level1)
{
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->TurnStandbyMode());
}

/**
 * @tc.name: AudioDevDump_001
 * @tc.desc: Verify the AudioDevDump function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, AudioDevDump_001, TestSize.Level1)
{
    int32_t range = 0;
    int32_t fd = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->AudioDevDump(range, fd));
}

/**
 * @tc.name: CheckSceneCapability_001
 * @tc.desc: Verify the CheckSceneCapability function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, CheckSceneCapability_001, TestSize.Level1)
{
    AudioSceneDescriptor scene;
    bool support = true;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->CheckSceneCapability(scene, support));
}

/**
 * @tc.name: SelectScene_001
 * @tc.desc: Verify the SelectScene function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SelectScene_001, TestSize.Level1)
{
    AudioSceneDescriptor scene;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SelectScene(scene));
}

/**
 * @tc.name: SetMute_001
 * @tc.desc: Verify the SetMute function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SetMute_001, TestSize.Level1)
{
    bool mute = true;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SetMute(mute));
}

/**
 * @tc.name: GetMute_001
 * @tc.desc: Verify the GetMute function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetMute_001, TestSize.Level1)
{
    bool mute = true;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetMute(mute));
}

/**
 * @tc.name: SetVolume_001
 * @tc.desc: Verify the SetVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SetVolume_001, TestSize.Level1)
{
    float volume = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SetVolume(volume));
}

/**
 * @tc.name: GetVolume_001
 * @tc.desc: Verify the GetVolume function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetVolume_001, TestSize.Level1)
{
    float volume = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetVolume(volume));
}

/**
 * @tc.name: GetGainThreshold_001
 * @tc.desc: Verify the GetGainThreshold function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetGainThreshold_001, TestSize.Level1)
{
    float min = 0;
    float max = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetGainThreshold(min, max));
}

/**
 * @tc.name: SetGain_001
 * @tc.desc: Verify the SetGain function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SetGain_001, TestSize.Level1)
{
    float gain = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SetGain(gain));
}

/**
 * @tc.name: GetGain_001
 * @tc.desc: Verify the GetGain function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetGain_001, TestSize.Level1)
{
    float gain = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetGain(gain));
}

/**
 * @tc.name: GetFrameSize_001
 * @tc.desc: Verify the GetFrameSize function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetFrameSize_001, TestSize.Level1)
{
    uint64_t size = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetFrameSize(size));
}

/**
 * @tc.name: GetFrameCount_001
 * @tc.desc: Verify the GetFrameCount function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetFrameCount_001, TestSize.Level1)
{
    uint64_t count = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetFrameCount(count));
}

/**
 * @tc.name: SetSampleAttributes_001
 * @tc.desc: Verify the SetSampleAttributes function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SetSampleAttributes_001, TestSize.Level1)
{
    AudioSampleAttributes attrs = {
        .type = AUDIO_IN_MEDIA,
        .interleaved = true,
        .format = AUDIO_FORMAT_PCM_16_BIT,
        .sampleRate = 44100,
        .channelCount = 2,
        .period = 1,
        .frameSize = 1,
        .isBigEndian = true,
        .isSignedData = true,
        .startThreshold = 1,
        .stopThreshold = 1,
        .silenceThreshold = 1,
        .streamId = 1,
    };
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SetSampleAttributes(attrs));
}

/**
 * @tc.name: GetSampleAttributes_001
 * @tc.desc: Verify the GetSampleAttributes function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetSampleAttributes_001, TestSize.Level1)
{
    AudioSampleAttributes attrs;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetSampleAttributes(attrs));
}

/**
 * @tc.name: GetCurrentChannelId_001
 * @tc.desc: Verify the GetCurrentChannelId function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetCurrentChannelId_001, TestSize.Level1)
{
    uint32_t channelId = 0;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetCurrentChannelId(channelId));
}

/**
 * @tc.name: SetExtraParams_001
 * @tc.desc: Verify the SetExtraParams function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, SetExtraParams_001, TestSize.Level1)
{
    std::string keyValueList = "hello";
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->SetExtraParams(keyValueList));
}

/**
 * @tc.name: GetExtraParams_001
 * @tc.desc: Verify the GetExtraParams function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetExtraParams_001, TestSize.Level1)
{
    std::string keyValueList;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetExtraParams(keyValueList));
}

/**
 * @tc.name: ReqMmapBuffer_001
 * @tc.desc: Verify the ReqMmapBuffer function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, ReqMmapBuffer_001, TestSize.Level1)
{
    int32_t reqSize = 1;
    AudioMmapBufferDescripter desc;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->ReqMmapBuffer(reqSize, desc));
}

/**
 * @tc.name: GetMmapPosition_001
 * @tc.desc: Verify the GetMmapPosition function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetMmapPosition_001, TestSize.Level1)
{
    uint64_t frames = 0;
    AudioTimeStamp time;
    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->GetMmapPosition(frames, time));
}

/**
 * @tc.name: GetVolumeInner_001
 * @tc.desc: Verify the GetVolumeInner function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetVolumeInner_001, TestSize.Level1)
{
    EXPECT_EQ(audioRenderInterfaceImpl_->vol_, audioRenderInterfaceImpl_->GetVolumeInner());
}

/**
 * @tc.name: GetFadeRate_001
 * @tc.desc: Verify the GetFadeRate function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetFadeRate_001, TestSize.Level1)
{
    uint32_t currentIndex = 2;
    const uint32_t durationIndex = 5;

    float fadeRate = audioRenderInterfaceImpl_->GetFadeRate(currentIndex, durationIndex);
    EXPECT_LE(0, fadeRate);
    EXPECT_GE(0.5f, fadeRate);
}

/**
 * @tc.name: GetFadeRate_002
 * @tc.desc: Verify the GetFadeRate function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetFadeRate_002, TestSize.Level1)
{
    uint32_t currentIndex = 3;
    const uint32_t durationIndex = 5;

    float fadeRate = audioRenderInterfaceImpl_->GetFadeRate(currentIndex, durationIndex);
    EXPECT_LE(0.5f, fadeRate);
    EXPECT_GE(1.0f, fadeRate);
}

/**
 * @tc.name: GetFadeRate_002
 * @tc.desc: Verify the GetFadeRate function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, GetFadeRate_003, TestSize.Level1)
{
    uint32_t currentIndex = 6;
    const uint32_t durationIndex = 5;

    float fadeRate = audioRenderInterfaceImpl_->GetFadeRate(currentIndex, durationIndex);
    EXPECT_EQ(1.0f, fadeRate);
}

/**
 * @tc.name: FadeInProcess_001
 * @tc.desc: Verify the FadeInProcess function.
 * @tc.type: FUNC
 * @tc.require: AR000H0E6H
 */
HWTEST_F(AudioRenderInterfaceImplTest, FadeInProcess_001, TestSize.Level1)
{
    const uint32_t durationFrame = 10;
    const size_t frameLength = 4096;
    int8_t* frameData = new int8_t[frameLength];

    EXPECT_EQ(HDF_SUCCESS, audioRenderInterfaceImpl_->FadeInProcess(durationFrame, frameData, frameLength));
}
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
