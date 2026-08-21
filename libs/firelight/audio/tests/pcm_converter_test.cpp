#include <firelight/audio/pcm_converter.hpp>

#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <vector>

namespace firelight::audio {

namespace {
constexpr int DEVICE_RATE = 48000;

// A quarter-amplitude sine, so a rate conversion has something with structure to
// preserve rather than a constant
double sineAt(const size_t frame, const size_t total) {
  return 0.25 * std::sin(2.0 * 3.14159265358979 * 8.0 * static_cast<double>(frame) / static_cast<double>(total));
}

PcmData makeInt16(const size_t frames, const int channels, const int rate) {
  PcmData data;
  data.sampleType = PcmSampleType::Int16;
  data.sampleRate = rate;
  data.channelCount = channels;
  data.bytes.resize(frames * channels * 2);

  for (size_t frame = 0; frame < frames; ++frame) {
    const auto value = static_cast<int16_t>(sineAt(frame, frames) * 32767.0);

    for (auto channel = 0; channel < channels; ++channel) {
      const auto offset = (frame * channels + channel) * 2;
      data.bytes[offset] = static_cast<uint8_t>(value & 0xFF);
      data.bytes[offset + 1] = static_cast<uint8_t>(value >> 8 & 0xFF);
    }
  }

  return data;
}

// The same waveform as makeInt16, widened to 32-bit — the shape SDL hands back
// for the 24-bit assets
PcmData makeInt32(const size_t frames, const int channels, const int rate) {
  PcmData data;
  data.sampleType = PcmSampleType::Int32;
  data.sampleRate = rate;
  data.channelCount = channels;
  data.bytes.resize(frames * channels * 4);

  for (size_t frame = 0; frame < frames; ++frame) {
    const auto value = static_cast<int32_t>(sineAt(frame, frames) * 32767.0) << 16;

    for (auto channel = 0; channel < channels; ++channel) {
      const auto offset = (frame * channels + channel) * 4;
      data.bytes[offset] = static_cast<uint8_t>(value & 0xFF);
      data.bytes[offset + 1] = static_cast<uint8_t>(value >> 8 & 0xFF);
      data.bytes[offset + 2] = static_cast<uint8_t>(value >> 16 & 0xFF);
      data.bytes[offset + 3] = static_cast<uint8_t>(value >> 24 & 0xFF);
    }
  }

  return data;
}

PcmData makeUInt8(const size_t frames, const int channels, const int rate) {
  PcmData data;
  data.sampleType = PcmSampleType::UInt8;
  data.sampleRate = rate;
  data.channelCount = channels;
  data.bytes.assign(frames * channels, 128);

  return data;
}

float peak(const std::vector<float> &samples) {
  auto highest = 0.0F;

  for (const auto sample : samples) {
    highest = std::max(highest, std::abs(sample));
  }

  return highest;
}
} // namespace

TEST(PcmConverterTest, RejectsZeroChannels) {
  auto input = makeInt16(100, 2, DEVICE_RATE);
  input.channelCount = 0;

  EXPECT_FALSE(PcmConverter::convert(input, DEVICE_RATE, 2).has_value());
}

TEST(PcmConverterTest, RejectsZeroTargetRate) {
  const auto input = makeInt16(100, 2, DEVICE_RATE);
  EXPECT_FALSE(PcmConverter::convert(input, 0, 2).has_value());
}

TEST(PcmConverterTest, RejectsZeroSourceRate) {
  auto input = makeInt16(100, 2, DEVICE_RATE);
  input.sampleRate = 0;

  EXPECT_FALSE(PcmConverter::convert(input, DEVICE_RATE, 2).has_value());
}

TEST(PcmConverterTest, EmptyInputProducesEmptyOutput) {
  PcmData input;
  input.sampleRate = DEVICE_RATE;
  input.channelCount = 2;

  const auto result = PcmConverter::convert(input, DEVICE_RATE, 2);

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST(PcmConverterTest, PreservesFrameCountAtMatchingRate) {
  const auto input = makeInt16(1000, 2, DEVICE_RATE);
  const auto result = PcmConverter::convert(input, DEVICE_RATE, 2);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->size() / 2, 1000u);
}

TEST(PcmConverterTest, MonoBecomesStereoWithBothChannelsEqual) {
  const auto input = makeInt16(500, 1, DEVICE_RATE);
  const auto result = PcmConverter::convert(input, DEVICE_RATE, 2);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size() / 2, 500u);

  for (size_t frame = 0; frame < 500; ++frame) {
    ASSERT_FLOAT_EQ((*result)[frame * 2], (*result)[frame * 2 + 1]) << "frame " << frame;
  }
}

TEST(PcmConverterTest, HalvesFrameCountDownsampling96kTo48k) {
  const auto input = makeInt16(9600, 1, 96000);
  const auto result = PcmConverter::convert(input, DEVICE_RATE, 2);

  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(static_cast<double>(result->size() / 2), 4800.0, 4.0);
}

TEST(PcmConverterTest, UpsamplesCdRateToDeviceRate) {
  const auto input = makeInt16(4410, 2, 44100);
  const auto result = PcmConverter::convert(input, DEVICE_RATE, 2);

  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(static_cast<double>(result->size() / 2), 4800.0, 4.0);
}

TEST(PcmConverterTest, Int16FullScaleMapsToUnitFloat) {
  PcmData input;
  input.sampleType = PcmSampleType::Int16;
  input.sampleRate = DEVICE_RATE;
  input.channelCount = 1;
  input.bytes = {0xFF, 0x7F, 0x00, 0x80};

  const auto result = PcmConverter::convert(input, DEVICE_RATE, 1);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 2u);
  EXPECT_NEAR((*result)[0], 1.0F, 0.001F);
  EXPECT_NEAR((*result)[1], -1.0F, 0.001F);
}

TEST(PcmConverterTest, UInt8IsCenteredAroundZero) {
  const auto input = makeUInt8(64, 1, DEVICE_RATE);
  const auto result = PcmConverter::convert(input, DEVICE_RATE, 1);

  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(peak(*result), 0.0F, 0.01F);
}

// The path button_nav.wav and game_view_nav.wav take: SDL widens their 24-bit
// samples to 32-bit, and that has to land at the same amplitude as 16-bit
TEST(PcmConverterTest, Int32MatchesInt16Amplitude) {
  const auto from16 = PcmConverter::convert(makeInt16(500, 1, DEVICE_RATE), DEVICE_RATE, 1);
  const auto from32 = PcmConverter::convert(makeInt32(500, 1, DEVICE_RATE), DEVICE_RATE, 1);

  ASSERT_TRUE(from16.has_value());
  ASSERT_TRUE(from32.has_value());
  ASSERT_EQ(from16->size(), from32->size());

  for (size_t i = 0; i < from16->size(); ++i) {
    ASSERT_NEAR((*from16)[i], (*from32)[i], 1e-4F) << "sample " << i;
  }
}

// Without a drain pass the resampler keeps its filter tail, which is a large
// share of a clip only a few milliseconds long
TEST(PcmConverterTest, DrainsTheResamplerTail) {
  const auto input = makeInt16(9600, 1, 96000);
  const auto result = PcmConverter::convert(input, DEVICE_RATE, 2);

  ASSERT_TRUE(result.has_value());

  const auto expected = 9600.0 * DEVICE_RATE / 96000.0;
  EXPECT_GE(static_cast<double>(result->size() / 2), expected - 2.0);
}

// bubble.wav is 916 frames at 96kHz, shorter than the resampler's own delay
TEST(PcmConverterTest, HandlesAClipShorterThanTheFilterDelay) {
  const auto input = makeInt16(916, 1, 96000);
  const auto result = PcmConverter::convert(input, DEVICE_RATE, 2);

  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(static_cast<double>(result->size() / 2), 458.0, 4.0);
  EXPECT_GT(peak(*result), 0.1F);
}

} // namespace firelight::audio
