#include <firelight/audio/audio_resampler.hpp>

#include <cmath>
#include <spdlog/spdlog.h>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
}

AudioResampler::~AudioResampler() {
  if (m_swrContext) {
    swr_free(&m_swrContext);
  }
  av_channel_layout_uninit(&m_channelLayout);
}

void AudioResampler::initialize(const int sampleRate) {
  initialize(sampleRate, sampleRate);
}

void AudioResampler::initialize(const int inputRate, const int outputRate) {
  m_sampleRate = inputRate;
  m_outputSampleRate = outputRate;
  m_activeRatio = m_requestedRatio.load();
  rebuild();
}

void AudioResampler::setPlaybackRateRatio(double ratio) {
  if (ratio <= 0.0) {
    ratio = 1.0;
  }
  m_requestedRatio = ratio;
}

void AudioResampler::rebuild() {
  if (m_sampleRate <= 0 || m_outputSampleRate <= 0) {
    return;
  }

  if (m_swrContext) {
    swr_free(&m_swrContext);
  }
  m_swrContext = swr_alloc();

  av_channel_layout_default(&m_channelLayout, 2);

  // Output rate is the device rate divided by the playback-rate ratio. Running
  // the game faster (ratio > 1) yields more input samples per real second, so we
  // resample to fewer output samples to keep the device buffer balanced (which
  // raises pitch proportionally to match the faster video)
  const int outputRate =
      static_cast<int>(std::lround(m_outputSampleRate / m_activeRatio));

  av_opt_set_chlayout(m_swrContext, "ichl", &m_channelLayout, 0);
  av_opt_set_int(m_swrContext, "in_sample_rate", m_sampleRate, 0);
  av_opt_set_sample_fmt(m_swrContext, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);

  av_opt_set_chlayout(m_swrContext, "ochl", &m_channelLayout, 0);
  av_opt_set_int(m_swrContext, "out_sample_rate", outputRate, 0);
  av_opt_set_sample_fmt(m_swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

  if (const int rc = swr_init(m_swrContext); rc < 0) {
    spdlog::error("Failed to initialize the resampling context: {}",
                  av_err2str(rc));
    swr_free(&m_swrContext);
  }
}

std::vector<int16_t> AudioResampler::process(const int16_t *data,
                                             const size_t numFrames,
                                             const int compensationDelta) {
  if (const double req = m_requestedRatio.load();
    std::abs(req - m_activeRatio) > 1e-6 && m_sampleRate > 0) {
    m_activeRatio = req;
    rebuild();
  }

  if (!m_swrContext) {
    return {};
  }

  if (numFrames > 300 || compensationDelta != 0) {
    swr_set_compensation(m_swrContext, compensationDelta,
                         static_cast<int>(numFrames));
  }

  const int maxOut =
      swr_get_out_samples(m_swrContext, static_cast<int>(numFrames));
  if (maxOut < 0) {
    spdlog::error("Error calculating maximum output samples: {}",
                  av_err2str(maxOut));
    return {};
  }
  if (maxOut == 0) {
    return {};
  }

  std::vector<int16_t> output(static_cast<size_t>(maxOut) * 2); // stereo
  auto *outPtr = reinterpret_cast<uint8_t *>(output.data());
  const auto *inPtr = reinterpret_cast<const uint8_t *>(data);

  const int outSamples = swr_convert(m_swrContext, &outPtr, maxOut, &inPtr,
                                     static_cast<int>(numFrames));
  if (outSamples < 0) {
    spdlog::error("Error during swr_convert: {}", av_err2str(outSamples));
    return {};
  }

  output.resize(static_cast<size_t>(outSamples) * 2);
  return output;
}
