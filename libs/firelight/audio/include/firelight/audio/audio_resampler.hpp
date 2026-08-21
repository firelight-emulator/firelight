#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

extern "C" {
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

/**
 * Resamples interleaved stereo S16 audio to a different sample rate, applying a compensation delta for
 * Dynamic Rate Control (DRC) to keep the audio sink buffer near 50% full
 */
class AudioResampler {
public:
  AudioResampler() = default;

  ~AudioResampler();

  AudioResampler(const AudioResampler &) = delete;

  AudioResampler &operator=(const AudioResampler &) = delete;

  /**
   * Initializes the resampler for a passthrough rate (input == output). Must be
   * called before processing audio
   * @param sampleRate The core's sample rate (in Hz)
   */
  void initialize(int sampleRate);

  /**
   * Initializes the resampler to convert from the core's input rate to a
   * (possibly different) output/device rate. Resampling to the audio device's
   * native rate here prevents the OS from needing to resample again
   *
   * @param inputRate The core's sample rate (in Hz) to resample from
   * @param outputRate The device's sample rate (in Hz) to resample to
   */
  void initialize(int inputRate, int outputRate);

  /**
   * Sets the playback rate ratio, which affects the output sample rate. A ratio greater than 1.0 speeds up
   * playback, while a ratio less than 1.0 slows it down. This is used to give a baseline for the resampling rate,
   * which is then adjusted by the compensation delta to keep the audio sink buffer near 50% full
   */
  void setPlaybackRateRatio(double ratio);

  /**
   * Resamples the given interleaved stereo S16 audio data to the output sample rate, applying the compensation delta
   * The compensation delta is typically computed by the AudioRateController based on the current fill level of the
   * audio sink buffer
   *
   * @param data Pointer to the input audio data (interleaved stereo S16)
   * @param numFrames Number of frames in the input data (each frame consists of 2 samples: left and right)
   * @param compensationDelta The number of samples to add to or remove from this call's output. Positive values
   *   raise the output sample count and negative values lower it; what the number should be is the caller's to
   *   decide (see AudioRateController)
   *
   * @return Vector containing the resampled audio data (interleaved stereo S16)
   */
  std::vector<int16_t> process(const int16_t *data, size_t numFrames, int compensationDelta);

  /**
   * @return true if the resampler has been initialized with a sample rate, false otherwise
   */
  [[nodiscard]] bool isInitialized() const { return m_swrContext != nullptr; }

private:
  void rebuild();

  SwrContext *m_swrContext = nullptr;
  AVChannelLayout m_channelLayout{};
  int m_sampleRate = 0;          // input (core) rate
  int m_outputSampleRate = 0;    // output (device) rate before the playback ratio
  int m_effectiveOutputRate = 0; // output rate the context was built with, after the playback ratio

  // Requested from any thread; the active value is only changed on the
  // process()/initialize() thread
  std::atomic<double> m_requestedRatio = 1.0;
  double m_activeRatio = 1.0;
};
