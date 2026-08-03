#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace firelight::audio {

/**
 * The interleaved sample encodings PcmConverter accepts
 */
enum class PcmSampleType { UInt8, Int16, Int32, Float32 };

/**
 * Interleaved PCM with no rate, layout, or format conversion applied yet
 */
struct PcmData {
  std::vector<uint8_t> bytes;
  PcmSampleType sampleType = PcmSampleType::Int16;
  int sampleRate = 0;
  int channelCount = 0;

  /**
   * @return The size in bytes of a single sample of sampleType
   */
  [[nodiscard]] int getBytesPerSample() const;

  /**
   * @return How many interleaved frames the byte buffer holds
   */
  [[nodiscard]] size_t getFrameCount() const;
};

/**
 * Converts interleaved PCM of any supported rate, channel count, and sample type into interleaved
 * float32 at a target rate and channel count
 */
class PcmConverter {
public:
  /**
   * Runs the whole buffer through libswresample and then drains the resampler, so the filter tail
   * is not lost off the end of a short clip
   *
   * @param input The source PCM
   * @param targetSampleRate The rate to convert to, in Hz
   * @param targetChannelCount The channel count to convert to
   * @return Interleaved float32 samples, or nullopt if the conversion could not be set up
   */
  static std::optional<std::vector<float>> convert(const PcmData &input, int targetSampleRate,
                                                   int targetChannelCount);
};

} // namespace firelight::audio
