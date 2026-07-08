#include <firelight/media/stream_encoder.hpp>

#include <QImage>
#include <spdlog/spdlog.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

namespace firelight::media {

namespace {
constexpr std::size_t MAX_QUEUED_FRAMES = 8;
constexpr int OPUS_SAMPLE_RATE = 48000;

struct RawFrame {
  std::vector<uint8_t> rgba;
  int width = 0;
  int height = 0;
  int stride = 0;
  int64_t ptsMs = 0;
};
} // namespace

struct StreamEncoder::Impl {
  ~Impl() { stop(); }

  VideoPacketCallback videoCallback;
  AudioPacketCallback audioCallback;

  StreamEncoderConfig config;

  // Video: worker-thread encode, hand-off queue fed by the render thread.
  AVCodecContext *videoCodec = nullptr;
  SwsContext *sws = nullptr;
  int swsSrcW = 0;
  int swsSrcH = 0;
  AVFrame *yuv = nullptr;
  AVPacket *videoPacket = nullptr;
  int64_t lastVideoPts = -1;
  std::vector<uint8_t> videoExtradata;
  std::atomic<bool> keyframeRequested{false};

  std::deque<RawFrame> queue;
  std::mutex queueMutex;
  std::condition_variable queueCv;
  std::condition_variable idleCv;
  bool encoding = false;
  std::thread worker;
  std::atomic<bool> running{false};

  // Audio: inline encode on the calling thread behind its own mutex.
  std::mutex audioMutex;
  AVCodecContext *audioCodec = nullptr;
  SwrContext *swr = nullptr;
  AVFrame *audioFrame = nullptr;
  AVPacket *audioPacket = nullptr;
  std::vector<int16_t> pcmPending; // resampled 48k stereo, interleaved
  int64_t audioSamplesSent = 0;

  bool start(const StreamEncoderConfig &encoderConfig) {
    stop();
    config = encoderConfig;
    config.width &= ~1;
    config.height &= ~1;
    if (config.width <= 0 || config.height <= 0 || config.fps <= 0) {
      return false;
    }
    if (!startVideo() || !startAudio()) {
      stop();
      return false;
    }
    running = true;
    worker = std::thread([this] { workerLoop(); });
    return true;
  }

  bool startVideo() {
    const AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
      codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    }
    if (!codec) {
      spdlog::warn("[StreamEncoder] no H.264 encoder available");
      return false;
    }
    videoCodec = avcodec_alloc_context3(codec);
    videoCodec->width = config.width;
    videoCodec->height = config.height;
    videoCodec->pix_fmt = AV_PIX_FMT_YUV420P;
    videoCodec->time_base = AVRational{1, 1000}; // pts already in ms
    videoCodec->framerate = AVRational{config.fps, 1};
    videoCodec->gop_size = config.fps * 4;
    videoCodec->max_b_frames = 0;
    videoCodec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    const int64_t bitrate = int64_t{1000} * std::max(config.bitrateKbps, 250);
    videoCodec->bit_rate = bitrate;
    videoCodec->rc_max_rate = bitrate;
    videoCodec->rc_buffer_size = static_cast<int>(bitrate / 2);
    av_opt_set(videoCodec->priv_data, "preset", "veryfast", 0);
    av_opt_set(videoCodec->priv_data, "tune", "zerolatency", 0);
    // Self-contained IDRs so a receiver can join mid-stream.
    av_opt_set(videoCodec->priv_data, "forced-idr", "1", 0);
    av_opt_set(videoCodec->priv_data, "x264-params", "repeat-headers=1", 0);

    if (avcodec_open2(videoCodec, codec, nullptr) < 0) {
      spdlog::warn("[StreamEncoder] failed to open H.264 encoder");
      return false;
    }
    videoExtradata.assign(videoCodec->extradata,
                          videoCodec->extradata + videoCodec->extradata_size);

    yuv = av_frame_alloc();
    yuv->format = AV_PIX_FMT_YUV420P;
    yuv->width = config.width;
    yuv->height = config.height;
    if (av_frame_get_buffer(yuv, 0) < 0) {
      return false;
    }
    videoPacket = av_packet_alloc();
    lastVideoPts = -1;
    return true;
  }

  bool startAudio() {
    const AVCodec *codec = avcodec_find_encoder_by_name("libopus");
    if (!codec) {
      codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);
    }
    if (!codec) {
      spdlog::warn("[StreamEncoder] no Opus encoder available");
      return false;
    }
    audioCodec = avcodec_alloc_context3(codec);
    audioCodec->sample_rate = OPUS_SAMPLE_RATE;
    audioCodec->sample_fmt = AV_SAMPLE_FMT_S16;
    av_channel_layout_default(&audioCodec->ch_layout, 2);
    audioCodec->bit_rate = 128000;
    audioCodec->time_base = AVRational{1, OPUS_SAMPLE_RATE};

    if (avcodec_open2(audioCodec, codec, nullptr) < 0) {
      spdlog::warn("[StreamEncoder] failed to open Opus encoder");
      return false;
    }

    swr = swr_alloc();
    AVChannelLayout stereo;
    av_channel_layout_default(&stereo, 2);
    av_opt_set_chlayout(swr, "in_chlayout", &stereo, 0);
    av_opt_set_chlayout(swr, "out_chlayout", &stereo, 0);
    av_opt_set_int(swr, "in_sample_rate", config.inputSampleRate, 0);
    av_opt_set_int(swr, "out_sample_rate", OPUS_SAMPLE_RATE, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    if (swr_init(swr) < 0) {
      spdlog::warn("[StreamEncoder] failed to set up audio resampler");
      return false;
    }

    audioFrame = av_frame_alloc();
    audioFrame->format = AV_SAMPLE_FMT_S16;
    audioFrame->sample_rate = OPUS_SAMPLE_RATE;
    av_channel_layout_default(&audioFrame->ch_layout, 2);
    audioFrame->nb_samples = audioCodec->frame_size; // 960 = 20 ms
    if (av_frame_get_buffer(audioFrame, 0) < 0) {
      return false;
    }
    audioPacket = av_packet_alloc();
    pcmPending.clear();
    audioSamplesSent = 0;
    return true;
  }

  void stop() {
    if (running.exchange(false)) {
      queueCv.notify_all();
      if (worker.joinable()) {
        worker.join();
      }
    }
    if (videoCodec) {
      avcodec_free_context(&videoCodec);
    }
    if (sws) {
      sws_freeContext(sws);
      sws = nullptr;
    }
    swsSrcW = swsSrcH = 0;
    if (yuv) {
      av_frame_free(&yuv);
    }
    if (videoPacket) {
      av_packet_free(&videoPacket);
    }
    {
      std::lock_guard lock(queueMutex);
      queue.clear();
    }
    std::lock_guard audioLock(audioMutex);
    if (audioCodec) {
      avcodec_free_context(&audioCodec);
    }
    if (swr) {
      swr_free(&swr);
    }
    if (audioFrame) {
      av_frame_free(&audioFrame);
    }
    if (audioPacket) {
      av_packet_free(&audioPacket);
    }
    pcmPending.clear();
  }

  void pushVideo(const QImage &frame, const int64_t ptsMs) {
    if (!running.load()) {
      return;
    }
    const bool rgbaBytes =
        frame.format() == QImage::Format_RGBA8888 ||
        frame.format() == QImage::Format_RGBA8888_Premultiplied;
    const QImage img =
        rgbaBytes ? frame : frame.convertToFormat(QImage::Format_RGBA8888);

    RawFrame raw;
    raw.width = img.width();
    raw.height = img.height();
    raw.stride = static_cast<int>(img.bytesPerLine());
    raw.ptsMs = ptsMs;
    raw.rgba.assign(img.constBits(), img.constBits() + img.sizeInBytes());
    {
      std::lock_guard lock(queueMutex);
      if (queue.size() >= MAX_QUEUED_FRAMES) {
        queue.pop_front();
      }
      queue.push_back(std::move(raw));
    }
    queueCv.notify_one();
  }

  void workerLoop() {
    while (true) {
      RawFrame frame;
      {
        std::unique_lock lock(queueMutex);
        queueCv.wait(lock, [this] { return !running.load() || !queue.empty(); });
        if (!running.load() && queue.empty()) {
          return;
        }
        frame = std::move(queue.front());
        queue.pop_front();
        encoding = true;
      }
      encodeVideoFrame(frame);
      {
        std::lock_guard lock(queueMutex);
        encoding = false;
        if (queue.empty()) {
          idleCv.notify_all();
        }
      }
    }
  }

  void encodeVideoFrame(const RawFrame &frame) {
    if (!sws || swsSrcW != frame.width || swsSrcH != frame.height) {
      sws = sws_getCachedContext(sws, frame.width, frame.height,
                                 AV_PIX_FMT_RGBA, config.width, config.height,
                                 AV_PIX_FMT_YUV420P, SWS_POINT, nullptr,
                                 nullptr, nullptr);
      swsSrcW = frame.width;
      swsSrcH = frame.height;
    }
    if (!sws || av_frame_make_writable(yuv) < 0) {
      return;
    }
    const uint8_t *srcData[4] = {frame.rgba.data(), nullptr, nullptr, nullptr};
    const int srcStride[4] = {frame.stride, 0, 0, 0};
    sws_scale(sws, srcData, srcStride, 0, frame.height, yuv->data,
              yuv->linesize);

    int64_t pts = frame.ptsMs;
    if (pts <= lastVideoPts) {
      pts = lastVideoPts + 1;
    }
    lastVideoPts = pts;
    yuv->pts = pts;
    yuv->pict_type = keyframeRequested.exchange(false) ? AV_PICTURE_TYPE_I
                                                       : AV_PICTURE_TYPE_NONE;

    if (avcodec_send_frame(videoCodec, yuv) < 0) {
      return;
    }
    while (avcodec_receive_packet(videoCodec, videoPacket) >= 0) {
      if (videoCallback) {
        videoCallback(
            std::vector<uint8_t>(videoPacket->data,
                                 videoPacket->data + videoPacket->size),
            videoPacket->pts, (videoPacket->flags & AV_PKT_FLAG_KEY) != 0);
      }
      av_packet_unref(videoPacket);
    }
  }

  void pushAudioSamples(const int16_t *data, const std::size_t numFrames) {
    std::lock_guard lock(audioMutex);
    if (!running.load() || !audioCodec || !swr || numFrames == 0) {
      return;
    }

    // Resample the core-rate input to 48 kHz and append to the pending PCM.
    const int maxOut = swr_get_out_samples(swr, static_cast<int>(numFrames));
    std::vector<int16_t> resampled(static_cast<size_t>(maxOut) * 2);
    auto *outPlane = reinterpret_cast<uint8_t *>(resampled.data());
    const auto *inPlane = reinterpret_cast<const uint8_t *>(data);
    const int converted = swr_convert(swr, &outPlane, maxOut, &inPlane,
                                      static_cast<int>(numFrames));
    if (converted <= 0) {
      return;
    }
    pcmPending.insert(pcmPending.end(), resampled.begin(),
                      resampled.begin() + static_cast<size_t>(converted) * 2);

    const auto frameSamples = static_cast<size_t>(audioFrame->nb_samples);
    while (pcmPending.size() >= frameSamples * 2) {
      if (av_frame_make_writable(audioFrame) < 0) {
        return;
      }
      std::memcpy(audioFrame->data[0], pcmPending.data(),
                  frameSamples * 2 * sizeof(int16_t));
      pcmPending.erase(pcmPending.begin(),
                       pcmPending.begin() +
                           static_cast<long long>(frameSamples * 2));
      audioFrame->pts = audioSamplesSent;
      audioSamplesSent += static_cast<int64_t>(frameSamples);

      if (avcodec_send_frame(audioCodec, audioFrame) < 0) {
        return;
      }
      while (avcodec_receive_packet(audioCodec, audioPacket) >= 0) {
        if (audioCallback) {
          const int64_t ptsMs = audioPacket->pts * 1000 / OPUS_SAMPLE_RATE;
          audioCallback(
              std::vector<uint8_t>(audioPacket->data,
                                   audioPacket->data + audioPacket->size),
              ptsMs);
        }
        av_packet_unref(audioPacket);
      }
    }
  }

  void flushVideo() {
    std::unique_lock lock(queueMutex);
    idleCv.wait(lock,
                [this] { return !running.load() || (queue.empty() && !encoding); });
  }
};

StreamEncoder::StreamEncoder() : m_impl(std::make_unique<Impl>()) {}
StreamEncoder::~StreamEncoder() = default;

void StreamEncoder::setVideoPacketCallback(VideoPacketCallback callback) {
  m_impl->videoCallback = std::move(callback);
}
void StreamEncoder::setAudioPacketCallback(AudioPacketCallback callback) {
  m_impl->audioCallback = std::move(callback);
}
bool StreamEncoder::start(const StreamEncoderConfig &config) {
  return m_impl->start(config);
}
void StreamEncoder::stop() { m_impl->stop(); }
bool StreamEncoder::isRunning() const { return m_impl->running.load(); }
void StreamEncoder::requestKeyframe() { m_impl->keyframeRequested = true; }
std::vector<uint8_t> StreamEncoder::extradata() const {
  return m_impl->videoExtradata;
}
void StreamEncoder::flush() { m_impl->flushVideo(); }
void StreamEncoder::pushVideoFrame(const QImage &frame, const int64_t ptsMs) {
  m_impl->pushVideo(frame, ptsMs);
}
void StreamEncoder::pushAudio(const int16_t *data, const std::size_t numFrames) {
  m_impl->pushAudioSamples(data, numFrames);
}

} // namespace firelight::media
