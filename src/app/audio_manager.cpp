#include "audio_manager.hpp"
#include <libswresample/swresample.h>
#include <spdlog/spdlog.h>

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

size_t AudioManager::receive(const int16_t *data, const size_t numFrames) {
  if (!m_isMuted && m_audioDevice) {
    m_frameNumber++;

    auto used = m_audioSink->bufferSize() - m_audioSink->bytesFree();
    static const int numSamples = 10; // Number of frames to average over
    static int buffer_avg[numSamples] = {0}; // Circular buffer for past buffer usages
    static int buffer_index = 0;
    int sum = 0;

    // Insert the current used value into the circular buffer
    buffer_avg[buffer_index] = used;
    buffer_index = (buffer_index + 1) % numSamples;

    // Calculate the moving average for buffer usage
    for (int i = 0; i < numSamples; i++) {
      sum += buffer_avg[i];
    }
    int average_used = sum / numSamples;

    m_deltaFrames -= average_used; // Use the average value for smoother buffer adjustment


    double targetBufferFill = m_audioSink->bufferSize() * 0.5; // Aim for half the buffer size as the target
    double bufferDeviation = (average_used - targetBufferFill) / targetBufferFill;

    // Gradual adjustment of m_changeThing using smoothing factor
    double smoothing_factor = 0.1; // Adjust this for more or less smoothing
    double desired_change = (m_deltaFrames < 0) ? -1 : (m_deltaFrames > 0) ? 1 : 0;

    m_changeThing += smoothing_factor * (desired_change - bufferDeviation);

    int delta = 0;
    if (bufferDeviation > 0.7) {
      delta = -5;
    } else if (bufferDeviation > 0.4) {
      delta = -4;
    } else if (bufferDeviation > 0.2) {
      delta = -2;
    } else if (bufferDeviation > 0.1) {
      delta = 1;
    } else if (bufferDeviation > -0.1) {
      delta = 0;
    } else if (bufferDeviation > -0.2) {
      delta = 1;
    } else if (bufferDeviation > -0.4) {
      delta = 2;
    } else if (bufferDeviation > -0.7) {
      delta = 4;
    } else {
      delta = 5;
    }

    // Clamp m_changeThing to avoid extreme values
    // if (m_changeThing > 3) {
    //   m_changeThing = 3;
    // } else if (m_changeThing < -3) {
    //   m_changeThing = -3;
    // }
    //
    // int delta = 0;
    // if (m_changeThing > 0.9) {
    //   delta = 5;
    // } else if (m_changeThing > 0.5) {
    //   delta = 1;
    // } else if (m_changeThing > 0.1) {
    //   delta = 0;
    // } else if (m_changeThing > -0.1) {
    //   delta = 0;
    // } else if (m_changeThing > -0.5) {
    //   delta = -1;
    // } else {
    //   delta = -3;
    // }
    // printf("delta: %d, changeThing: %f, buffer deviation: %f\n", delta, m_changeThing, bufferDeviation);

    if (numFrames > 300 || delta > 0) {
      swr_set_compensation(m_swrContext, delta, numFrames);
    }

    int max_output_samples = swr_get_out_samples(m_swrContext, numFrames);
    if (max_output_samples < 0) {
      fprintf(stderr, "Error calculating maximum output samples\n");
      return 0;
    }

    // printf("max output samples: %d\n", max_output_samples);
    uint8_t *outputBuffer[2];

    av_samples_alloc(outputBuffer, NULL, 2, max_output_samples, AV_SAMPLE_FMT_S16, 0); // 2 channels, 16-bit samples

    // Resample the input data and apply the dynamic compensation using delta samples
    int output_samples = swr_convert(m_swrContext, outputBuffer, max_output_samples, (const uint8_t **) &data,
                                     numFrames);
    // int output_samples = resampleAudio((uint8_t **) &data, numFrames, outputBuffer, max_output_samples, delta_samples);

    // Write the resampled audio data to the audio device
    int wrote = m_audioDevice->write((char *) outputBuffer[0], output_samples * 4);
    // 4 bytes per frame (2 channels, 16-bit)

    // printf("avgUsed: %d, changeThing: %f, sent: %llu, wrote: %d, buffer filled: %lld\n", average_used, m_changeThing,
    //        numFrames * 4, wrote, used);
    m_deltaFrames += wrote;
    // Free the output buffer
    av_freep(&outputBuffer[0]);
    //
    //   if (m_deltaFrames > 5) {
    //     m_deltaFrames = 5;
    //   } else if (m_deltaFrames < -5) {
    //     m_deltaFrames = -5;
    //   }

    // }

    // m_avgSamplesInOneFrame[avgSamplesOneFrameIndex] = used;
    // if (avgSamplesOneFrameIndex++ == 10) {
    //   avgSamplesOneFrameIndex = 0;
    //   for (auto &i: m_avgSamplesInOneFrame) {
    //     printf("thing: %d\n", i);
    //   }
    //   printf("**********\n");
    // }

    // int delta = buffer_occupancy - previous_buffer_occupancy;

    // if (delta > 0) {
    //   printf("Buffer is filling by %d bytes (free: %llu)\n", delta, m_audioSink->bytesFree());
    // } else if (delta < 0) {
    //   printf("Buffer is emptying by %d bytes (free: %llu)\n", -delta, m_audioSink->bytesFree());
    // } else {
    //   printf("Buffer is stable\n");
    // }

    // update_buffer_history();
    // if (m_frameNumber == 60) {
    //   calculate_average_rate_of_change();
    //   m_frameNumber = 0;
    // }

    // Ensure buffer_occupancy stays within bounds
    // if (buffer_occupancy > buffer_size) {
    //   buffer_occupancy = buffer_size;
    // } else if (buffer_occupancy < 0) {
    //   buffer_occupancy = 0;
    // }


    // m_bufferSize += m_audioSink->bufferSize() - m_audioSink->bytesFree();
    // m_numSamples += numFrames;
    // //
    // if (m_frameNumber == 10) {
    //   auto used = m_audioSink->bufferSize() - m_audioSink->bytesFree();
    //   printf("Used: %lld, last: %d, diff: %lld\n", used, m_lastBufferSize, used - m_lastBufferSize);
    //   m_lastBufferSize = used;
    //   // auto numWrittenLastTenFrames = m_bufferSize / 4.0 * 6.0;
    //   // printf("avg last 10 frames: %f\n", numWrittenLastTenFrames);
    //   // // spdlog::info("ratio: {}", (m_bufferSize / 4) * 6);
    //   m_frameNumber = 0;
    //   // m_bufferSize = 0;
    //   // m_numSamples = 0;
    // }
    // m_audioDevice->write((char *) data, numFrames * 4);


    // printf("buffer used: %lld\n", m_audioSink->bufferSize() - m_audioSink->bytesFree());
    // printf("wrote: %lld, free: %lld\n", m_audioDevice->write((char *) data, numFrames * 4),
    //        m_audioSink->bytesFree());


    // Timer: Measure elapsed time since the last callback
    // double actual_frame_time = m_timer.elapsed(); // Time since the last callback
    // double expected_frame_time = 1000.0 / 60.0; // Expected time between callbacks (16.67ms for 60Hz)
    // double frame_time_deviation = actual_frame_time - expected_frame_time; // Deviation from expected time
    //
    // m_timer.restart(); // Restart the timer for the next frame
    //
    // // Calculate delta samples to compensate for the timing difference
    // int delta_samples = static_cast<int>((frame_time_deviation / expected_frame_time) * numFrames);
    //
    // // Clamp the delta samples to avoid drastic changes (e.g., +/-5 samples)
    // const int max_delta_samples = 1; // Adjust based on how sensitive the timing correction needs to be
    // if (delta_samples > max_delta_samples) {
    //   delta_samples = max_delta_samples;
    // } else if (delta_samples < -max_delta_samples) {
    //   delta_samples = -max_delta_samples;
    // }
    //
    // swr_set_compensation(m_swrContext, delta_samples, numFrames);
    //
    // // Use swr_get_out_samples to get the maximum number of output samples that may be generated
    // int max_output_samples = swr_get_out_samples(m_swrContext, numFrames);
    // if (max_output_samples < 0) {
    //   fprintf(stderr, "Error calculating maximum output samples\n");
    //   return 0;
    // }
    //
    // // Allocate output buffer for resampled audio based on maximum possible output samples
    // uint8_t *outputBuffer[2];
    // av_samples_alloc(outputBuffer, NULL, 2, max_output_samples, AV_SAMPLE_FMT_S16, 0); // 2 channels, 16-bit samples
    //
    // // Resample the input data and apply the dynamic compensation using delta samples
    // int output_samples = swr_convert(m_swrContext, outputBuffer, max_output_samples, (const uint8_t **) &data,
    //                                  numFrames);
    // // int output_samples = resampleAudio((uint8_t **) &data, numFrames, outputBuffer, max_output_samples, delta_samples);
    //
    // // Write the resampled audio data to the audio device
    // m_audioDevice->write((char *) outputBuffer[0], output_samples * 4); // 4 bytes per frame (2 channels, 16-bit)
    //
    // // Free the output buffer
    // av_freep(&outputBuffer[0]);
    //
    // // Logging buffer information
    // printf("Size: %d, Bytes free: %lld\n", m_audioSink->bufferSize(), m_audioSink->bytesFree());
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

  m_audioSink->setBufferSize(8192);
  m_audioDevice = m_audioSink->start();
}

void AudioManager::setMuted(bool muted) {
  m_isMuted = muted;
  // SDL_PauseAudioDevice(audioDevice, m_isMuted);
}

AudioManager::~AudioManager() {
  m_audioSink->stop();
  m_audioDevice->close();
  av_free(m_swrContext);
  // SDL_CloseAudioDevice(this->audioDevice);
}
