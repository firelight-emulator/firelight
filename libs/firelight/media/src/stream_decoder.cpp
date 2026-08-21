#include <firelight/media/stream_decoder.hpp>

#include <cstring>
#include <mutex>
#include <spdlog/spdlog.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

namespace firelight::media {

struct StreamDecoder::Impl {
  ~Impl() { stop(); }

  VideoFrameCallback videoCallback;
  AudioCallback audioCallback;

  std::mutex mutex;
  bool running = false;

  AVCodecContext *videoCodec = nullptr;
  AVFrame *videoFrame = nullptr;
  AVPacket *packet = nullptr;
  SwsContext *sws = nullptr;
  int swsW = 0;
  int swsH = 0;

  AVCodecContext *audioCodec = nullptr;
  AVFrame *audioFrame = nullptr;
  SwrContext *swr = nullptr;

  bool start(std::span<const uint8_t> extradata) {
    std::lock_guard lock(mutex);
    teardown();

    const AVCodec *video = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!video) {
      spdlog::warn("[StreamDecoder] no H.264 decoder available");
      return false;
    }
    videoCodec = avcodec_alloc_context3(video);
    if (!extradata.empty()) {
      videoCodec->extradata = static_cast<uint8_t *>(av_mallocz(extradata.size() + AV_INPUT_BUFFER_PADDING_SIZE));
      std::memcpy(videoCodec->extradata, extradata.data(), extradata.size());
      videoCodec->extradata_size = static_cast<int>(extradata.size());
    }
    if (avcodec_open2(videoCodec, video, nullptr) < 0) {
      spdlog::warn("[StreamDecoder] failed to open H.264 decoder");
      teardown();
      return false;
    }

    const AVCodec *audio = avcodec_find_decoder(AV_CODEC_ID_OPUS);
    if (!audio) {
      spdlog::warn("[StreamDecoder] no Opus decoder available");
      teardown();
      return false;
    }
    audioCodec = avcodec_alloc_context3(audio);
    audioCodec->sample_rate = 48000;
    av_channel_layout_default(&audioCodec->ch_layout, 2);
    if (avcodec_open2(audioCodec, audio, nullptr) < 0) {
      spdlog::warn("[StreamDecoder] failed to open Opus decoder");
      teardown();
      return false;
    }

    videoFrame = av_frame_alloc();
    audioFrame = av_frame_alloc();
    packet = av_packet_alloc();
    running = true;
    return true;
  }

  void stop() {
    std::lock_guard lock(mutex);
    teardown();
  }

  void teardown() {
    running = false;
    if (videoCodec) {
      avcodec_free_context(&videoCodec);
    }
    if (audioCodec) {
      avcodec_free_context(&audioCodec);
    }
    if (videoFrame) {
      av_frame_free(&videoFrame);
    }
    if (audioFrame) {
      av_frame_free(&audioFrame);
    }
    if (packet) {
      av_packet_free(&packet);
    }
    if (sws) {
      sws_freeContext(sws);
      sws = nullptr;
    }
    if (swr) {
      swr_free(&swr);
    }
    swsW = swsH = 0;
  }

  void pushVideo(std::span<const uint8_t> data, const int64_t ptsMs) {
    std::lock_guard lock(mutex);
    if (!running || data.empty()) {
      return;
    }
    if (av_new_packet(packet, static_cast<int>(data.size())) < 0) {
      return;
    }
    std::memcpy(packet->data, data.data(), data.size());
    packet->pts = ptsMs;
    const int sent = avcodec_send_packet(videoCodec, packet);
    av_packet_unref(packet);
    if (sent < 0) {
      return;
    }
    while (avcodec_receive_frame(videoCodec, videoFrame) >= 0) {
      emitVideoFrame(ptsMs);
      av_frame_unref(videoFrame);
    }
  }

  void emitVideoFrame(const int64_t fallbackPtsMs) {
    if (!videoCallback) {
      return;
    }
    const int width = videoFrame->width;
    const int height = videoFrame->height;
    if (width <= 0 || height <= 0) {
      return;
    }
    if (!sws || swsW != width || swsH != height) {
      sws = sws_getCachedContext(sws, width, height, static_cast<AVPixelFormat>(videoFrame->format), width, height,
                                 AV_PIX_FMT_RGBA, SWS_POINT, nullptr, nullptr, nullptr);
      swsW = width;
      swsH = height;
    }
    if (!sws) {
      return;
    }
    StreamVideoFrame out;
    out.width = width;
    out.height = height;
    out.rgba.resize(static_cast<size_t>(width) * height * 4);
    uint8_t *dstData[4] = {out.rgba.data(), nullptr, nullptr, nullptr};
    const int dstStride[4] = {width * 4, 0, 0, 0};
    sws_scale(sws, videoFrame->data, videoFrame->linesize, 0, height, dstData, dstStride);

    const int64_t pts = videoFrame->pts != AV_NOPTS_VALUE ? videoFrame->pts : fallbackPtsMs;
    videoCallback(std::move(out), pts);
  }

  void pushAudio(std::span<const uint8_t> data, const int64_t ptsMs) {
    std::lock_guard lock(mutex);
    if (!running || data.empty()) {
      return;
    }
    if (av_new_packet(packet, static_cast<int>(data.size())) < 0) {
      return;
    }
    std::memcpy(packet->data, data.data(), data.size());
    packet->pts = ptsMs;
    const int sent = avcodec_send_packet(audioCodec, packet);
    av_packet_unref(packet);
    if (sent < 0) {
      return;
    }
    while (avcodec_receive_frame(audioCodec, audioFrame) >= 0) {
      emitAudio(ptsMs);
      av_frame_unref(audioFrame);
    }
  }

  void emitAudio(const int64_t ptsMs) {
    if (!audioCallback || audioFrame->nb_samples <= 0) {
      return;
    }
    if (!swr) {
      swr = swr_alloc();
      AVChannelLayout stereo;
      av_channel_layout_default(&stereo, 2);
      av_opt_set_chlayout(swr, "in_chlayout", &audioFrame->ch_layout, 0);
      av_opt_set_chlayout(swr, "out_chlayout", &stereo, 0);
      av_opt_set_int(swr, "in_sample_rate", audioFrame->sample_rate, 0);
      av_opt_set_int(swr, "out_sample_rate", 48000, 0);
      av_opt_set_sample_fmt(swr, "in_sample_fmt", static_cast<AVSampleFormat>(audioFrame->format), 0);
      av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
      if (swr_init(swr) < 0) {
        swr_free(&swr);
        return;
      }
    }
    std::vector<int16_t> out(static_cast<size_t>(audioFrame->nb_samples) * 2);
    auto *outPlane = reinterpret_cast<uint8_t *>(out.data());
    const int converted = swr_convert(swr, &outPlane, audioFrame->nb_samples,
                                      const_cast<const uint8_t **>(audioFrame->data), audioFrame->nb_samples);
    if (converted <= 0) {
      return;
    }
    out.resize(static_cast<size_t>(converted) * 2);
    audioCallback(std::move(out), ptsMs);
  }
};

StreamDecoder::StreamDecoder() : m_impl(std::make_unique<Impl>()) {}

StreamDecoder::~StreamDecoder() = default;

void StreamDecoder::setVideoFrameCallback(VideoFrameCallback callback) { m_impl->videoCallback = std::move(callback); }

void StreamDecoder::setAudioCallback(AudioCallback callback) { m_impl->audioCallback = std::move(callback); }

bool StreamDecoder::start(std::span<const uint8_t> videoExtradata) { return m_impl->start(videoExtradata); }

void StreamDecoder::stop() { m_impl->stop(); }

bool StreamDecoder::isRunning() const {
  std::lock_guard lock(m_impl->mutex);
  return m_impl->running;
}

bool StreamDecoder::reset(std::span<const uint8_t> videoExtradata) { return m_impl->start(videoExtradata); }

void StreamDecoder::pushVideoPacket(std::span<const uint8_t> data, const int64_t ptsMs) {
  m_impl->pushVideo(data, ptsMs);
}

void StreamDecoder::pushAudioPacket(std::span<const uint8_t> data, const int64_t ptsMs) {
  m_impl->pushAudio(data, ptsMs);
}

} // namespace firelight::media
