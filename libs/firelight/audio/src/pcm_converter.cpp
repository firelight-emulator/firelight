#include <firelight/audio/pcm_converter.hpp>

extern "C" {
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

namespace firelight::audio {

namespace {
AVSampleFormat avFormatFor(const PcmSampleType type) {
  switch (type) {
  case PcmSampleType::UInt8:
    return AV_SAMPLE_FMT_U8;
  case PcmSampleType::Int16:
    return AV_SAMPLE_FMT_S16;
  case PcmSampleType::Int32:
    return AV_SAMPLE_FMT_S32;
  case PcmSampleType::Float32:
    return AV_SAMPLE_FMT_FLT;
  }

  return AV_SAMPLE_FMT_NONE;
}
} // namespace

int PcmData::getBytesPerSample() const {
  switch (sampleType) {
  case PcmSampleType::UInt8:
    return 1;
  case PcmSampleType::Int16:
    return 2;
  case PcmSampleType::Int32:
  case PcmSampleType::Float32:
    return 4;
  }

  return 0;
}

size_t PcmData::getFrameCount() const {
  const auto frameSize = static_cast<size_t>(getBytesPerSample()) * static_cast<size_t>(channelCount);

  if (frameSize == 0) {
    return 0;
  }

  return bytes.size() / frameSize;
}

std::optional<std::vector<float>> PcmConverter::convert(const PcmData &input, const int targetSampleRate,
                                                        const int targetChannelCount) {
  if (input.sampleRate <= 0 || input.channelCount <= 0 || targetSampleRate <= 0 || targetChannelCount <= 0) {
    return std::nullopt;
  }

  const auto inputFrames = input.getFrameCount();

  if (inputFrames == 0) {
    return std::vector<float>{};
  }

  const auto inputFormat = avFormatFor(input.sampleType);

  if (inputFormat == AV_SAMPLE_FMT_NONE) {
    return std::nullopt;
  }

  AVChannelLayout inputLayout{};
  AVChannelLayout outputLayout{};
  av_channel_layout_default(&inputLayout, input.channelCount);
  av_channel_layout_default(&outputLayout, targetChannelCount);

  SwrContext *context = swr_alloc();

  if (context == nullptr) {
    av_channel_layout_uninit(&inputLayout);
    av_channel_layout_uninit(&outputLayout);
    return std::nullopt;
  }

  av_opt_set_chlayout(context, "ichl", &inputLayout, 0);
  av_opt_set_int(context, "in_sample_rate", input.sampleRate, 0);
  av_opt_set_sample_fmt(context, "in_sample_fmt", inputFormat, 0);

  av_opt_set_chlayout(context, "ochl", &outputLayout, 0);
  av_opt_set_int(context, "out_sample_rate", targetSampleRate, 0);
  av_opt_set_sample_fmt(context, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

  av_channel_layout_uninit(&inputLayout);
  av_channel_layout_uninit(&outputLayout);

  if (swr_init(context) < 0) {
    swr_free(&context);
    return std::nullopt;
  }

  const auto capacity = av_rescale_rnd(swr_get_delay(context, input.sampleRate) + static_cast<int64_t>(inputFrames),
                                       targetSampleRate, input.sampleRate, AV_ROUND_UP);

  std::vector<float> output(static_cast<size_t>(capacity) * targetChannelCount);
  auto *outputBytes = reinterpret_cast<uint8_t *>(output.data());
  const auto *inputBytes = input.bytes.data();

  const auto converted =
      swr_convert(context, &outputBytes, static_cast<int>(capacity), &inputBytes, static_cast<int>(inputFrames));

  if (converted < 0) {
    swr_free(&context);
    return std::nullopt;
  }

  auto totalFrames = static_cast<size_t>(converted);

  // TODO
  // Flush whatever the resampler is still holding; on a clip only a few
  // milliseconds long the tail is a meaningful share of the sound
  while (totalFrames < static_cast<size_t>(capacity)) {
    auto *tail = reinterpret_cast<uint8_t *>(output.data() + totalFrames * targetChannelCount);
    const auto drained = swr_convert(context, &tail, static_cast<int>(capacity - totalFrames), nullptr, 0);

    if (drained <= 0) {
      break;
    }

    totalFrames += static_cast<size_t>(drained);
  }

  swr_free(&context);

  output.resize(totalFrames * targetChannelCount);

  return output;
}

} // namespace firelight::audio
