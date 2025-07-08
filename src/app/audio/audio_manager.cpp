#include "audio_manager.hpp"

#include <spdlog/spdlog.h>

extern "C" {
#include <libswresample/swresample.h>
}

void AudioManager::initializeResampler(int64_t in_channel_layout,
                                       int in_sample_rate,
                                       enum AVSampleFormat in_sample_fmt,
                                       int64_t out_channel_layout,
                                       int out_sample_rate,
                                       AVSampleFormat out_sample_fmt) {
  m_swrContext = swr_alloc();

  av_channel_layout_default(m_channelLayout, 2);
  char thing[256];

  av_channel_layout_describe(m_channelLayout, thing, sizeof(thing));

  // Set options for input and output
  // av_opt_set_int(m_swrContext, "in_channel_layout", in_channel_layout, 0);
  av_opt_set_chlayout(m_swrContext, "ichl", m_channelLayout, 0);
  av_opt_set_int(m_swrContext, "in_sample_rate", in_sample_rate, 0);
  av_opt_set_sample_fmt(m_swrContext, "in_sample_fmt", in_sample_fmt, 0);

  av_opt_set_chlayout(m_swrContext, "ochl", m_channelLayout, 0);
  av_opt_set_int(m_swrContext, "out_sample_rate", out_sample_rate, 0);
  av_opt_set_sample_fmt(m_swrContext, "out_sample_fmt", out_sample_fmt, 0);

  if (const int returnCode = swr_init(m_swrContext) < 0) {
    spdlog::error("Failed to initialize the resampling context: {}",
                  av_err2str(returnCode));
    swr_free(&m_swrContext);
  }
}

AudioManager::AudioManager(std::function<void()> onAudioBufferLevelChanged)
    : m_onAudioBufferLevelChanged(std::move(onAudioBufferLevelChanged)) {
  m_elapsedTimer.start();
}

size_t AudioManager::receive(const int16_t *data, const size_t numFrames) {
  // TODO: REALLY BAD SOLUTION for mupen sometimes sending very small number of
  // frames
  if (numFrames < 30) {
    return numFrames;
  }

  m_numSamples += numFrames;
  if (m_elapsedTimer.elapsed() > 1000) {
    m_elapsedTimer.restart();
    m_numSamples = 0;
  }

  if (!m_isMuted && m_audioDevice && m_audioSink) { // Added m_audioSink check
    const auto bufferTotalCapacity = m_audioSink->bufferSize();
    if (bufferTotalCapacity == 0)
      return numFrames; // Avoid division by zero

    const auto usedBytes = bufferTotalCapacity - m_audioSink->bytesFree();
    m_currentBufferLevel = static_cast<float>(usedBytes) / bufferTotalCapacity;
    if (m_onAudioBufferLevelChanged) {
      m_onAudioBufferLevelChanged();
    }

    // --- Moving Average Calculation for Buffer Usage ---
    static constexpr int AVG_WINDOW_SIZE =
        10; // Number of observations to average over
    static int avg_buffer_usage_bytes[AVG_WINDOW_SIZE] =
        {}; // Circular buffer for past buffer usages (bytes)
    static int avg_buffer_idx = 0;
    static int avg_buffer_populated_count =
        0; // To correctly average when buffer isn't full yet
    static double previous_avg_fill_ratio =
        -1.0; // Previous cycle's average fill ratio, -1.0 indicates
              // uninitialized

    // Insert the current usedBytes into the circular buffer
    avg_buffer_usage_bytes[avg_buffer_idx] = usedBytes;
    avg_buffer_idx = (avg_buffer_idx + 1) % AVG_WINDOW_SIZE;

    if (avg_buffer_populated_count < AVG_WINDOW_SIZE) {
      avg_buffer_populated_count++;
    }

    long long sum_used_bytes =
        0; // Use long long for sum to avoid overflow if usedBytes can be large
    for (int i = 0; i < avg_buffer_populated_count; ++i) {
      sum_used_bytes += avg_buffer_usage_bytes[i];
    }
    const double current_average_used_bytes =
        static_cast<double>(sum_used_bytes) / avg_buffer_populated_count;
    const double current_avg_fill_ratio =
        current_average_used_bytes / bufferTotalCapacity;

    // --- Trend Analysis & Resampling Decision ---
    const double TARGET_FILL_RATIO = 0.5;
    const double TARGET_FILL_BYTES = bufferTotalCapacity * TARGET_FILL_RATIO;

    // Deviation of current average from target
    double bufferDeviation =
        (current_average_used_bytes - TARGET_FILL_BYTES) / TARGET_FILL_BYTES;

    int delta = 0; // Default to no compensation
    bool allowResamplingAdjustment = true;

    // Only perform trend analysis if we have a stable previous average
    if (previous_avg_fill_ratio >= 0.0 &&
        avg_buffer_populated_count == AVG_WINDOW_SIZE) {
      const double current_error_ratio =
          current_avg_fill_ratio - TARGET_FILL_RATIO;
      const double previous_error_ratio =
          previous_avg_fill_ratio - TARGET_FILL_RATIO;

      const double WITHIN_TARGET_TOLERANCE_RATIO = 0.05; // e.g., 45%-55% fill
      const double EXTREME_DEVIATION_THRESHOLD_RATIO =
          0.25; // e.g., <25% or >75% fill

      bool isTrendingWell =
          (std::abs(current_error_ratio) < std::abs(previous_error_ratio));
      bool isNearTarget =
          (std::abs(current_error_ratio) <= WITHIN_TARGET_TOLERANCE_RATIO);
      bool isExtremelyDeviated =
          (std::abs(current_error_ratio) > EXTREME_DEVIATION_THRESHOLD_RATIO);

      if (isNearTarget) {
        allowResamplingAdjustment =
            false; // Already close to target, no need to intervene
        // spdlog::debug("Buffer near target ({:.2f}%), no resampling.",
        // current_avg_fill_ratio * 100);
      } else if (isTrendingWell && !isExtremelyDeviated) {
        allowResamplingAdjustment =
            false; // Trending correctly and not in an extreme state
        // spdlog::debug("Buffer trending well ({:.2f}%), error reducing, no
        // resampling.", current_avg_fill_ratio * 100);
      }
      // Else, allowResamplingAdjustment remains true (either not trending well,
      // or trending well but still extremely deviated)
    }

    // Update previous average fill ratio for the next call,
    // only after the circular buffer is fully populated for consistent average
    // comparison.
    if (avg_buffer_populated_count == AVG_WINDOW_SIZE) {
      previous_avg_fill_ratio = current_avg_fill_ratio;
    }

    if (allowResamplingAdjustment) {
      // Calculate delta based on current buffer deviation
      // This is your existing logic for determining delta
      if (bufferDeviation > 0.6) { // >80% full (target is 50%)
        delta = -5;
      } else if (bufferDeviation > 0.3) { // >65%
        delta = -4;
      } else if (bufferDeviation > 0.1) { // >55%
        delta = -3;
      } else if (bufferDeviation > -0.1) { // 45%-55% (This will be overridden
                                           // by isNearTarget usually)
        delta = 0;
      } else if (bufferDeviation > -0.3) { // 35%-45%
        // Original logic had delta = 0 here. If buffer is consistently too low
        // and not trending up, we might want to add samples.
        // Consider if this should be delta = 1 or if deadband is intentional.
        // For now, keeping original logic unless overridden by trend:
        delta = 0; // Maintained original logic for this band if adjustment is
                   // allowed but if it's stuck here and not trending well, this
                   // might need to be positive. However, the
                   // EXTREME_DEVIATION_THRESHOLD_RATIO for allowing adjustment
                   // helps.
      } else if (bufferDeviation > -0.6) { // 20%-35%
        delta = 1;
      } else { // <20%
        delta = 2;
      }
      // spdlog::debug("Resampling allowed. BufferDev: {:.2f}, Delta: {}",
      // bufferDeviation, delta);
    } else {
      delta = 0; // Trend is good or near target, so force no compensation
    }

    // Apply resampling if delta is non-zero OR if numFrames is large (original
    // condition)
    if (numFrames > 300 || delta != 0) {
      swr_set_compensation(m_swrContext, delta, numFrames);
    }

    const int max_output_samples = swr_get_out_samples(m_swrContext, numFrames);
    if (max_output_samples < 0) {
      spdlog::error("Error calculating maximum output samples: {}",
                    av_err2str(max_output_samples));
      // av_freep(&outputBuffer[0]); // This would be an error, outputBuffer not
      // allocated yet
      return 0; // Or handle error appropriately
    }
    if (max_output_samples == 0 && numFrames > 0) {
      // This can happen if delta causes all samples to be dropped.
      // Or if numFrames is very small and context cannot produce output.
      // No data to write, but not necessarily an error if intended.
      // spdlog::debug("Max output samples is 0 for numFrames: {}", numFrames);
      return numFrames; // Input consumed, no output produced.
    }

    uint8_t *outputBuffer[2] = {nullptr, nullptr}; // Initialize to nullptr
    // Assuming stereo (2 channels), AV_SAMPLE_FMT_S16
    if (av_samples_alloc(outputBuffer, NULL, 2, max_output_samples,
                         AV_SAMPLE_FMT_S16, 0) < 0) {
      spdlog::error("Failed to allocate output buffer for resampling.");
      return 0;
    }

    const int output_samples =
        swr_convert(m_swrContext, outputBuffer, max_output_samples,
                    (const uint8_t **)&data, numFrames);

    if (output_samples < 0) {
      spdlog::error("Error during swr_convert: {}", av_err2str(output_samples));
      av_freep(&outputBuffer[0]);
      return 0; // Or handle error
    }

    if (output_samples > 0) {
      m_audioDevice->write(reinterpret_cast<char *>(outputBuffer[0]),
                           output_samples * 2 * sizeof(int16_t)); // stereo, s16
    }

    av_freep(&outputBuffer[0]); // Frees the entire buffer allocated by
                                // av_samples_alloc
  }

  return numFrames; // Return original number of frames consumed
}

void AudioManager::initialize(const double new_freq) {
  m_sampleRate = new_freq;
  initializeResampler(AV_CH_LAYOUT_STEREO, m_sampleRate, AV_SAMPLE_FMT_S16,
                      AV_CH_LAYOUT_STEREO, m_sampleRate, AV_SAMPLE_FMT_S16);

  QAudioFormat format;
  format.setChannelCount(2);
  format.setSampleFormat(QAudioFormat::Int16);
  format.setSampleRate(m_sampleRate);

  m_audioSink = new QAudioSink(format);

  auto mult = 2;
  if (m_sampleRate > 44000) {
    mult = 4;
  }
  m_audioSink->setBufferSize(8192 * mult);
  m_audioDevice = m_audioSink->start();
}

void AudioManager::setMuted(bool muted) { m_isMuted = muted; }

float AudioManager::getBufferLevel() const { return m_currentBufferLevel; }

AudioManager::~AudioManager() {
  if (m_audioDevice) {
    m_audioDevice->close();
  }
  if (m_audioSink) {
    m_audioSink->stop();
  }
  if (m_swrContext) {
    av_free(m_swrContext);
  }
}
