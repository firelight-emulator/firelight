#include "audio_manager.hpp"
#include <libswresample/swresample.h>

void AudioManager::initializeResampler(int64_t in_channel_layout, int in_sample_rate, enum AVSampleFormat in_sample_fmt,
                                       int64_t out_channel_layout, int out_sample_rate,
                                       AVSampleFormat out_sample_fmt) {
  m_swrContext = swr_alloc();

  // Set options for input and output
  av_opt_set_int(m_swrContext, "in_channel_layout", in_channel_layout, 0);
  av_opt_set_int(m_swrContext, "in_sample_rate", in_sample_rate, 0);
  av_opt_set_sample_fmt(m_swrContext, "in_sample_fmt", in_sample_fmt, 0);

  av_opt_set_int(m_swrContext, "out_channel_layout", out_channel_layout, 0);
  av_opt_set_int(m_swrContext, "out_sample_rate", out_sample_rate, 0);
  av_opt_set_sample_fmt(m_swrContext, "out_sample_fmt", out_sample_fmt, 0);

  if (swr_init(m_swrContext) < 0) {
    fprintf(stderr, "Failed to initialize the resampling context\n");
    swr_free(&m_swrContext);
  }
}

AudioManager::AudioManager(std::function<void()> onAudioBufferLevelChanged) : m_onAudioBufferLevelChanged(
  std::move(onAudioBufferLevelChanged)) {
}

size_t AudioManager::receive(const int16_t *data, const size_t numFrames) {
  if (!m_isMuted && m_audioDevice) {
    m_frameNumber++;

    const auto used = m_audioSink->bufferSize() - m_audioSink->bytesFree();
    m_currentBufferLevel = static_cast<float>(used) / m_audioSink->bufferSize();
    if (m_onAudioBufferLevelChanged) {
      m_onAudioBufferLevelChanged();
    }
    static constexpr int numSamples = 10; // Number of frames to average over
    static int buffer_avg[numSamples] = {}; // Circular buffer for past buffer usages
    static int buffer_index = 0;
    int sum = 0;

    // Insert the current used value into the circular buffer
    buffer_avg[buffer_index] = used;
    buffer_index = (buffer_index + 1) % numSamples;

    // Calculate the moving average for buffer usage
    for (const int i: buffer_avg) {
      sum += i;
    }
    const int average_used = sum / numSamples;

    m_deltaFrames -= average_used; // Use the average value for smoother buffer adjustment


    double targetBufferFill = m_audioSink->bufferSize() * 0.5; // Aim for half the buffer size as the target
    double bufferDeviation = (average_used - targetBufferFill) / targetBufferFill;

    // // Gradual adjustment of m_changeThing using smoothing factor
    // const double smoothing_factor = 0.1; // Adjust this for more or less smoothing
    // const double desired_change = (m_deltaFrames < 0) ? -1 : (m_deltaFrames > 0) ? 1 : 0;
    //
    // m_changeThing += smoothing_factor * (desired_change - bufferDeviation);


    int delta = 0;
    if (bufferDeviation > 0.8) {
      delta = -4;
    } else if (bufferDeviation > 0.4) {
      delta = -3;
    } else if (bufferDeviation > 0.2) {
      delta = -2;
    } else if (bufferDeviation > 0.1) {
      delta = -1;
    } else if (bufferDeviation > -0.1) {
      delta = 0;
    } else if (bufferDeviation > -0.2) {
      delta = 1;
    } else if (bufferDeviation > -0.4) {
      delta = 2;
    } else if (bufferDeviation > -0.8) {
      delta = 3;
    } else {
      delta = 4;
    }

    if (numFrames > 300 || delta > 0) {
      swr_set_compensation(m_swrContext, delta, numFrames);
    }

    const int max_output_samples = swr_get_out_samples(m_swrContext, numFrames);
    if (max_output_samples < 0) {
      fprintf(stderr, "Error calculating maximum output samples\n");
      return 0;
    }

    uint8_t *outputBuffer[2];
    av_samples_alloc(outputBuffer, NULL, 2, max_output_samples, AV_SAMPLE_FMT_S16, 0); // 2 channels, 16-bit samples

    // Resample the input data and apply the dynamic compensation using delta samples
    const int output_samples = swr_convert(m_swrContext, outputBuffer, max_output_samples, (const uint8_t **) &data,
                                           numFrames);

    m_deltaFrames += m_audioDevice->write((char *) outputBuffer[0], output_samples * 4);

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

  m_audioSink->setBufferSize(16384);
  m_audioDevice = m_audioSink->start();
}

void AudioManager::setMuted(bool muted) {
  m_isMuted = muted;
}

float AudioManager::getBufferLevel() const {
  return m_currentBufferLevel;
}

AudioManager::~AudioManager() {
  m_audioSink->stop();
  m_audioDevice->close();
  av_free(m_swrContext);
}
