#include <firelight/audio/sfx_mixer.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <cmath>
#include <thread>
#include <vector>

namespace firelight::audio {

namespace {
constexpr int DEVICE_RATE = 48000;

// A clip whose every sample is the same value, so a gain or a sum is readable
// straight off the output
std::vector<float> flatClip(const size_t frames, const int channels, const float value) {
  return std::vector<float>(frames * channels, value);
}

// A clip whose frames ramp upward, so a read position can be identified from the
// value at it
std::vector<float> rampClip(const size_t frames, const int channels) {
  std::vector<float> samples(frames * channels);

  for (size_t frame = 0; frame < frames; ++frame) {
    for (auto channel = 0; channel < channels; ++channel) {
      samples[frame * channels + channel] = static_cast<float>(frame) / static_cast<float>(frames);
    }
  }

  return samples;
}

size_t countNonSilentFrames(const std::vector<float> &output, const int channels) {
  size_t count = 0;

  for (size_t frame = 0; frame < output.size() / channels; ++frame) {
    if (std::abs(output[frame * channels]) > 1e-6F) {
      count++;
    }
  }

  return count;
}
} // namespace

TEST(SfxMixerTest, RenderIsSilentWithNoVoices) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);
  mixer.addClip(flatClip(64, 2, 0.5F));

  std::vector<float> output(128, 1.0F);
  mixer.render(output.data(), 64);

  for (const auto sample : output) {
    EXPECT_FLOAT_EQ(sample, 0.0F);
  }

  EXPECT_EQ(mixer.getActiveVoiceCount(), 0);
}

TEST(SfxMixerTest, RenderZeroFramesIsSafe) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);
  const auto clip = mixer.addClip(flatClip(64, 2, 0.5F));
  mixer.play(clip, 1.0F, 4);

  std::vector<float> output(2, 1.0F);
  mixer.render(output.data(), 0);

  EXPECT_EQ(mixer.getActiveVoiceCount(), 0);
}

TEST(SfxMixerTest, AddClipReturnsSequentialIds) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);

  EXPECT_EQ(mixer.addClip(flatClip(4, 2, 0.1F)), 0);
  EXPECT_EQ(mixer.addClip(flatClip(4, 2, 0.2F)), 1);
  EXPECT_EQ(mixer.getClipCount(), 2);
}

TEST(SfxMixerTest, AddClipRefusesBeyondMaxClips) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);

  for (auto i = 0; i < SfxMixer::MAX_CLIPS; ++i) {
    ASSERT_GE(mixer.addClip(flatClip(4, 2, 0.1F)), 0);
  }

  EXPECT_EQ(mixer.addClip(flatClip(4, 2, 0.1F)), -1);
}

TEST(SfxMixerTest, PlaysAClipVerbatimAtUnityGain) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);
  const auto clip = mixer.addClip(flatClip(8, 2, 0.5F));
  mixer.play(clip, 1.0F, 4);

  std::vector<float> output(16, 0.0F);
  mixer.render(output.data(), 8);

  for (const auto sample : output) {
    EXPECT_FLOAT_EQ(sample, 0.5F);
  }
}

TEST(SfxMixerTest, AppliesGain) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);
  const auto clip = mixer.addClip(flatClip(8, 2, 0.8F));
  mixer.play(clip, 0.5F, 4);

  std::vector<float> output(16, 0.0F);
  mixer.render(output.data(), 8);

  for (const auto sample : output) {
    EXPECT_FLOAT_EQ(sample, 0.4F);
  }
}

TEST(SfxMixerTest, SumsOverlappingVoices) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);
  const auto clip = mixer.addClip(flatClip(8, 2, 0.2F));
  mixer.play(clip, 1.0F, 4);
  mixer.play(clip, 1.0F, 4);

  std::vector<float> output(16, 0.0F);
  mixer.render(output.data(), 8);

  EXPECT_FLOAT_EQ(output[0], 0.4F);
  EXPECT_EQ(mixer.getActiveVoiceCount(), 2);
}

TEST(SfxMixerTest, ClampsToUnitRange) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);
  const auto clip = mixer.addClip(flatClip(8, 2, 0.5F));

  for (auto i = 0; i < 10; ++i) {
    mixer.play(clip, 1.0F, SfxMixer::MAX_VOICES);
  }

  std::vector<float> output(16, 0.0F);
  mixer.render(output.data(), 8);

  for (const auto sample : output) {
    EXPECT_LE(sample, 1.0F);
    EXPECT_GE(sample, -1.0F);
  }

  EXPECT_FLOAT_EQ(output[0], 1.0F);
}

TEST(SfxMixerTest, RendersAcrossBufferBoundaries) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(rampClip(16, 1));
  mixer.play(clip, 1.0F, 4);

  std::vector<float> first(8, 0.0F);
  std::vector<float> second(8, 0.0F);
  mixer.render(first.data(), 8);
  mixer.render(second.data(), 8);

  const auto expected = rampClip(16, 1);

  for (size_t i = 0; i < 8; ++i) {
    EXPECT_FLOAT_EQ(first[i], expected[i]) << "first half, sample " << i;
    EXPECT_FLOAT_EQ(second[i], expected[i + 8]) << "second half, sample " << i;
  }
}

TEST(SfxMixerTest, VoiceEndsAfterClipLength) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(4, 1, 0.5F));
  mixer.play(clip, 1.0F, 4);

  std::vector<float> output(8, 0.0F);
  mixer.render(output.data(), 8);

  EXPECT_FLOAT_EQ(output[3], 0.5F);
  EXPECT_FLOAT_EQ(output[4], 0.0F);
  EXPECT_FLOAT_EQ(output[7], 0.0F);
  EXPECT_EQ(mixer.getActiveVoiceCount(), 0);
}

TEST(SfxMixerTest, RecyclesOldestWhenPerClipVoiceLimitReached) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(64, 1, 0.25F));

  mixer.play(clip, 1.0F, 2);
  mixer.play(clip, 1.0F, 2);
  mixer.play(clip, 1.0F, 2);

  std::vector<float> output(8, 0.0F);
  mixer.render(output.data(), 8);

  EXPECT_EQ(mixer.getActiveVoiceCount(), 2);
}

TEST(SfxMixerTest, RespectsGlobalVoiceCap) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(64, 1, 0.01F));

  for (auto i = 0; i < SfxMixer::MAX_VOICES + 8; ++i) {
    mixer.play(clip, 1.0F, SfxMixer::MAX_VOICES);
  }

  std::vector<float> output(8, 0.0F);
  mixer.render(output.data(), 8);

  EXPECT_EQ(mixer.getActiveVoiceCount(), SfxMixer::MAX_VOICES);
}

TEST(SfxMixerTest, PerClipLimitDoesNotStarveOtherClips) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto quiet = mixer.addClip(flatClip(64, 1, 0.1F));
  const auto loud = mixer.addClip(flatClip(64, 1, 0.4F));

  for (auto i = 0; i < 5; ++i) {
    mixer.play(quiet, 1.0F, 2);
  }

  mixer.play(loud, 1.0F, 2);

  std::vector<float> output(4, 0.0F);
  mixer.render(output.data(), 4);

  EXPECT_EQ(mixer.getActiveVoiceCount(), 3);
  EXPECT_FLOAT_EQ(output[0], 0.6F);
}

TEST(SfxMixerTest, StopFadesOutRatherThanCutting) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(512, 1, 0.5F));
  mixer.play(clip, 1.0F, 4);

  std::vector<float> before(8, 0.0F);
  mixer.render(before.data(), 8);
  ASSERT_FLOAT_EQ(before[0], 0.5F);

  mixer.stop(clip);

  std::vector<float> fading(SfxMixer::STOP_FADE_FRAMES, 0.0F);
  mixer.render(fading.data(), SfxMixer::STOP_FADE_FRAMES);

  for (size_t i = 1; i < fading.size(); ++i) {
    ASSERT_LE(fading[i], fading[i - 1]) << "sample " << i;
  }

  EXPECT_FLOAT_EQ(fading.back(), 0.0F);
  EXPECT_EQ(mixer.getActiveVoiceCount(), 0);
}

TEST(SfxMixerTest, StopOnlyAffectsTheGivenClip) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto first = mixer.addClip(flatClip(512, 1, 0.25F));
  const auto second = mixer.addClip(flatClip(512, 1, 0.25F));
  mixer.play(first, 1.0F, 4);
  mixer.play(second, 1.0F, 4);

  std::vector<float> output(4, 0.0F);
  mixer.render(output.data(), 4);
  ASSERT_FLOAT_EQ(output[0], 0.5F);

  mixer.stop(first);
  mixer.render(output.data(), 4);

  EXPECT_EQ(mixer.getActiveVoiceCount(), 2);

  std::vector<float> settled(SfxMixer::STOP_FADE_FRAMES * 2, 0.0F);
  mixer.render(settled.data(), SfxMixer::STOP_FADE_FRAMES * 2);

  EXPECT_FLOAT_EQ(settled.back(), 0.25F);
  EXPECT_EQ(mixer.getActiveVoiceCount(), 1);
}

// Requests that piled up while nothing was rendering must not all land in the
// first buffer afterwards, which sums copies of the same sound in phase
TEST(SfxMixerTest, DiscardPendingCommandsDropsQueuedPlays) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(64, 1, 0.3F));

  mixer.play(clip, 1.0F, 4);
  mixer.play(clip, 1.0F, 4);
  mixer.play(clip, 1.0F, 4);
  mixer.discardPendingCommands();

  std::vector<float> output(8, 0.0F);
  mixer.render(output.data(), 8);

  EXPECT_EQ(mixer.getActiveVoiceCount(), 0);
  EXPECT_FLOAT_EQ(output[0], 0.0F);
}

// The shape of the bug the discard exists for: without it, three queued plays
// start together at the same position and treble the amplitude
TEST(SfxMixerTest, QueuedPlaysWouldOtherwiseSumInPhase) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(64, 1, 0.3F));

  mixer.play(clip, 1.0F, 4);
  mixer.play(clip, 1.0F, 4);
  mixer.play(clip, 1.0F, 4);

  std::vector<float> output(8, 0.0F);
  mixer.render(output.data(), 8);

  EXPECT_FLOAT_EQ(output[0], 0.9F);
  EXPECT_EQ(mixer.getActiveVoiceCount(), 3);
}

TEST(SfxMixerTest, PlayWithInvalidClipIdIsIgnored) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 2);
  mixer.addClip(flatClip(8, 2, 0.5F));

  EXPECT_FALSE(mixer.play(-1, 1.0F, 4));
  EXPECT_FALSE(mixer.play(99, 1.0F, 4));

  std::vector<float> output(16, 0.0F);
  mixer.render(output.data(), 8);

  EXPECT_EQ(mixer.getActiveVoiceCount(), 0);
}

TEST(SfxMixerTest, PitchAboveOnePlaysTheClipInLessTime) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(100, 1, 0.5F));
  mixer.play(clip, 1.0F, 4, 2.0F);

  std::vector<float> output(100, 0.0F);
  mixer.render(output.data(), 100);

  EXPECT_NEAR(static_cast<double>(countNonSilentFrames(output, 1)), 50.0, 1.0);
}

TEST(SfxMixerTest, PitchBelowOnePlaysTheClipOverMoreTime) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(50, 1, 0.5F));
  mixer.play(clip, 1.0F, 4, 0.5F);

  std::vector<float> output(120, 0.0F);
  mixer.render(output.data(), 120);

  EXPECT_NEAR(static_cast<double>(countNonSilentFrames(output, 1)), 100.0, 2.0);
}

TEST(SfxMixerTest, NonPositivePitchFallsBackToUnity) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(20, 1, 0.5F));
  mixer.play(clip, 1.0F, 4, 0.0F);

  std::vector<float> output(40, 0.0F);
  mixer.render(output.data(), 40);

  EXPECT_NEAR(static_cast<double>(countNonSilentFrames(output, 1)), 20.0, 1.0);
}

TEST(SfxMixerTest, ClearClipsSilencesActiveVoices) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(64, 1, 0.5F));
  mixer.play(clip, 1.0F, 4);

  std::vector<float> output(4, 0.0F);
  mixer.render(output.data(), 4);
  ASSERT_EQ(mixer.getActiveVoiceCount(), 1);

  mixer.clearClips();
  mixer.render(output.data(), 4);

  EXPECT_EQ(mixer.getClipCount(), 0);
  EXPECT_EQ(mixer.getActiveVoiceCount(), 0);
  EXPECT_FLOAT_EQ(output[0], 0.0F);
}

TEST(SfxMixerTest, DropsCommandsWhenRingIsFullWithoutBlocking) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(64, 1, 0.01F));

  for (auto i = 0; i < SfxMixer::COMMAND_CAPACITY + 32; ++i) {
    mixer.play(clip, 1.0F, SfxMixer::MAX_VOICES);
  }

  EXPECT_GT(mixer.getDroppedCommandCount(), 0u);

  std::vector<float> output(4, 0.0F);
  mixer.render(output.data(), 4);

  EXPECT_LE(mixer.getActiveVoiceCount(), SfxMixer::MAX_VOICES);
}

// Hammers the ring from another thread while rendering, which is the arrangement
// the audio callback actually runs in
TEST(SfxMixerTest, SurvivesConcurrentPlayAndRender) {
  SfxMixer mixer;
  mixer.configure(DEVICE_RATE, 1);
  const auto clip = mixer.addClip(flatClip(64, 1, 0.05F));

  std::atomic<bool> done{false};
  std::thread producer([&mixer, clip, &done] {
    for (auto i = 0; i < 50000; ++i) {
      mixer.play(clip, 1.0F, 4);
    }

    done = true;
  });

  std::vector<float> output(64, 0.0F);

  while (!done) {
    mixer.render(output.data(), 64);

    for (const auto sample : output) {
      ASSERT_TRUE(std::isfinite(sample));
      ASSERT_LE(std::abs(sample), 1.0F);
    }
  }

  producer.join();
  mixer.render(output.data(), 64);

  EXPECT_LE(mixer.getActiveVoiceCount(), SfxMixer::MAX_VOICES);
  EXPECT_GE(mixer.getActiveVoiceCount(), 0);
}

} // namespace firelight::audio
