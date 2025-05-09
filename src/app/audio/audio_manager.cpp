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

  m_numSamples += numFrames;
  if (m_elapsedTimer.elapsed() > 1000) {
    m_elapsedTimer.restart();
    // spdlog::info("Last second average samples: {}", m_numSamples);
    m_numSamples = 0;
  }

  // spdlog::info("Writing {} bytes", numFrames * 4);
  // m_audioDevice->write((char *)data, numFrames * 4);
  if (!m_isMuted && m_audioDevice) {

    const auto used = m_audioSink->bufferSize() - m_audioSink->bytesFree();
    m_currentBufferLevel = static_cast<float>(used) / m_audioSink->bufferSize();
    if (m_onAudioBufferLevelChanged) {
      m_onAudioBufferLevelChanged();
    }

    static constexpr int numSamples = 10; // Number of frames to average over
    static int buffer_avg[numSamples] =
        {}; // Circular buffer for past buffer usages
    static int buffer_index = 0;
    int sum = 0;

    // Insert the current used value into the circular buffer
    buffer_avg[buffer_index] = used;
    buffer_index = (buffer_index + 1) % numSamples;

    // Calculate the moving average for buffer usage
    for (const int i : buffer_avg) {
      sum += i;
    }
    const int average_used = sum / numSamples;

    double targetBufferFill =
        m_audioSink->bufferSize() * 0.5; // Aim for half the buffer size as the
    double bufferDeviation =
        (average_used - targetBufferFill) / targetBufferFill;

    int delta = 0;
    if (bufferDeviation > 0.6) {
      delta = -7;
    } else if (bufferDeviation > 0.3) {
      delta = -6;
    } else if (bufferDeviation > 0.1) {
      delta = -5;
    } else if (bufferDeviation > -0.1) {
      delta = 0;
    } else if (bufferDeviation > -0.3) {
      delta = 3;
    } else if (bufferDeviation > -0.6) {
      delta = 4;
    } else {
      delta = 5;
    }

    if (numFrames > 300 || delta > 0) {
      swr_set_compensation(m_swrContext, delta, numFrames);
    }

    const int max_output_samples = swr_get_out_samples(m_swrContext, numFrames);
    if (max_output_samples < 0) {
      spdlog::error("Error calculating maximum output samples");
      return 0;
    }

    uint8_t *outputBuffer[2];
    av_samples_alloc(outputBuffer, NULL, 2, max_output_samples,
                     AV_SAMPLE_FMT_S16, 0); // 2 channels, 16-bit samples

    // Resample the input data and apply the dynamic compensation using delta
    // samples
    const int output_samples =
        swr_convert(m_swrContext, outputBuffer, max_output_samples,
                    (const uint8_t **)&data, numFrames);

    auto written =
        m_audioDevice->write((char *)outputBuffer[0], output_samples * 4);
    // spdlog::info("Wanting to write {} bytes, wrote {} bytes",
    //              output_samples * 4, written);

    av_freep(&outputBuffer[0]);
  }

  return numFrames;
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
  m_audioSink->stop();
  m_audioDevice->close();
  av_free(m_swrContext);
}
