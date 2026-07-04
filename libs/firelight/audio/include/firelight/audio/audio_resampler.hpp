#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

extern "C" {
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

// Owns the libswresample context that converts core audio (interleaved stereo
// S16) to the output device's sample rate. It also applies a feed-forward
// playback-rate bias (sync-to-monitor resamples audio to the display refresh
// rate) and a per-call drift-compensation delta (dynamic rate control, decided
// by the caller from the output buffer's occupancy).
//
// The swr context is only ever touched on the thread that calls
// initialize()/process(); setPlaybackRateRatio() may be called from any thread
// and is applied lazily on the next process().
class AudioResampler {
public:
  AudioResampler() = default;
  ~AudioResampler();

  AudioResampler(const AudioResampler &) = delete;
  AudioResampler &operator=(const AudioResampler &) = delete;

  // (Re)build the context for the given core sample rate (Hz).
  void initialize(int sampleRate);

  // 1.0 = native; > 1 plays back faster (ratio = refreshHz / coreFps).
  void setPlaybackRateRatio(double ratio);

  // Resample `numFrames` interleaved stereo S16 frames, nudging the rate by
  // `compensationDelta` samples over the call. Returns interleaved stereo S16
  // output (empty on error or when nothing was produced).
  std::vector<int16_t> process(const int16_t *data, size_t numFrames,
                               int compensationDelta);

  [[nodiscard]] bool isInitialized() const { return m_swrContext != nullptr; }

private:
  void rebuild();

  SwrContext *m_swrContext = nullptr;
  AVChannelLayout m_channelLayout{};
  int m_sampleRate = 0;

  // Requested from any thread; the active value is only changed on the
  // process()/initialize() thread.
  std::atomic<double> m_requestedRatio = 1.0;
  double m_activeRatio = 1.0;
};
