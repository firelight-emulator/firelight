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

int AudioManager::resampleAudio(uint8_t **in_data, int in_nb_samples, uint8_t **out_data, int out_nb_samples,
                                int delta_samples) {
  // Compensate for buffer underflow/overflow by adjusting sample rate
  if (delta_samples != 0) {
    swr_set_compensation(m_swrContext, delta_samples, in_nb_samples);
  }

  // Perform resampling
  int ret = swr_convert(m_swrContext, out_data, out_nb_samples, (const uint8_t **) in_data, in_nb_samples);
  if (ret < 0) {
    fprintf(stderr, "Error while resampling audio\n");
    return ret;
  }

  return ret;
}

size_t AudioManager::receive(const int16_t *data, const size_t numFrames) {
  if (!m_isMuted && m_audioDevice) {
    // Calculate the maximum number of output samples based on input
    int max_output_samples = av_rescale_rnd(numFrames, m_sampleRate, m_sampleRate, AV_ROUND_UP);

    // Allocate output buffer for resampled audio
    uint8_t *outputBuffer[2];
    av_samples_alloc(outputBuffer, NULL, 2, max_output_samples, AV_SAMPLE_FMT_S16, 0); // 2 channels, 16-bit samples

    // Resample the input data and apply dynamic rate control
    int output_samples = resampleAudio((uint8_t **) &data, numFrames, outputBuffer, max_output_samples, -5);


    // printf("output samples: %d\n", output_samples);
    // Write the resampled audio data to the audio device
    m_audioDevice->write((char *) outputBuffer[0], output_samples * 4); // 4 bytes per frame (2 channels, 16-bit)

    // Free the output buffer
    av_freep(&outputBuffer[0]);

    // Logging buffer information
    printf("Size: %d, Bytes free: %lld\n", m_audioSink->bufferSize(), m_audioSink->bytesFree());
  }

  return numFrames;
}


void MyAudioCallback(void *userdata, Uint8 *stream, int len) {
  auto audioManager = static_cast<AudioManager *>(userdata);
  printf("Callback Requesting %d (available: %lld)!\n", len, audioManager->m_buffer.size());
  // if (audioManager->m_buffer.size() == 0) {
  //   return;
  // }

  if (audioManager->m_buffer.size() < 4) {
    return;
  }

  audioManager->m_buffer.read((char *) stream, len);
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

  m_audioDevice = m_audioSink->start();

  // m_buffer.open(QIODeviceBase::ReadWrite);
  // SDL_AudioSpec want, have;
  //
  // SDL_memset(&want, 0, sizeof(want));
  // want.freq = new_freq; // Sample rate (e.g., 44.1 kHz)
  // want.format = AUDIO_S16; // Audio format (16-bit signed)
  // want.channels = 2; // Number of audio channels (stereo)
  // want.samples = 1024; // Audio buffer size (samples)
  // want.callback = MyAudioCallback;
  // want.userdata = this;
  //
  // this->audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  // if (audioDevice == 0) {
  //   printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
  // }
  //
  // SDL_PauseAudioDevice(audioDevice, false); // Start audio playback
}

void AudioManager::setMuted(bool muted) {
  m_isMuted = muted;
  SDL_PauseAudioDevice(audioDevice, m_isMuted);
}

AudioManager::~AudioManager() {
  m_buffer.close();
  SDL_CloseAudioDevice(this->audioDevice);
}
