#include <firelight/media/clip_recorder.hpp>

#include <QImage>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace firelight::media {

namespace {
// Backpressure: if the encoder falls behind, drop the oldest queued frame
// rather than let the hand-off queue grow. A handful of small retro frames.
constexpr std::size_t MAX_QUEUED_FRAMES = 8;

struct RawFrame {
  std::vector<uint8_t> rgba;
  int width = 0;
  int height = 0;
  int stride = 0;
  int64_t ptsMs = 0;
};
} // namespace

struct ClipRecorder::Impl {
  explicit Impl(const int windowSeconds)
      : windowUs(static_cast<int64_t>(windowSeconds > 0 ? windowSeconds : 15) *
                 1'000'000) {}
  ~Impl() { stop(); }

  const int64_t windowUs;
  int fps = 60;
  int sampleRate = 48000;
  int channels = 2;
  int encWidth = 0;
  int encHeight = 0;

  // ffmpeg state — created on start(), used only by the worker while running,
  // freed by stop() after the worker joins.
  AVCodecContext *enc = nullptr;
  SwsContext *sws = nullptr;
  int swsSrcW = 0;
  int swsSrcH = 0;
  AVFrame *yuv = nullptr;
  AVPacket *pkt = nullptr;
  std::vector<uint8_t> extradata;
  int64_t lastPts = -1;

  // Hand-off queue: render thread -> worker.
  std::deque<RawFrame> queue;
  std::mutex queueMutex;
  std::condition_variable queueCv;
  std::condition_variable idleCv; // signalled when the worker goes idle
  bool encoding = false;          // true while a frame is mid-encode
  std::thread worker;
  std::atomic<bool> running{false};

  // Rolling compressed window + PCM ring.
  mutable std::mutex ringMutex;
  std::deque<EncodedPacket> videoRing;
  std::deque<int16_t> pcmRing;

  bool start(int w, int h, int f, int sr, int ch);
  void stop();
  void reset();
  void flush();
  void pushVideo(const QImage &frame, int64_t ptsMs);
  void pushAudio(const int16_t *data, std::size_t numFrames);
  [[nodiscard]] ClipSnapshot snapshot() const;

  void workerLoop();
  void encodeFrame(const RawFrame &f);
  void drainEncoder();
  void appendPacket(const AVPacket *p);
  void trimVideoRing(); // caller holds ringMutex
};

bool ClipRecorder::Impl::start(const int w, const int h, const int f,
                               const int sr, const int ch) {
  stop(); // clean restart

  fps = f > 0 ? f : 60;
  sampleRate = sr > 0 ? sr : 48000;
  channels = ch > 0 ? ch : 2;
  if (w <= 0 || h <= 0) {
    return false;
  }
  // Retro frames are tiny; a raw native-res encode looks soft once a player
  // upscales it. Integer-upscale (nearest-neighbor, done in encodeFrame) to
  // roughly 720 lines so the clip stays pixel-sharp on playback.
  constexpr int targetHeight = 720;
  const int scale = std::clamp(targetHeight / h, 1, 6);
  encWidth = (w * scale) & ~1; // even dims for YUV420
  encHeight = (h * scale) & ~1;

  const AVCodec *codec = avcodec_find_encoder_by_name("libx264");
  if (!codec) {
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  }
  if (!codec) {
    spdlog::warn("[ClipRecorder] no H.264 encoder available");
    return false;
  }

  enc = avcodec_alloc_context3(codec);
  if (!enc) {
    return false;
  }
  enc->width = encWidth;
  enc->height = encHeight;
  enc->pix_fmt = AV_PIX_FMT_YUV420P;
  enc->time_base = AVRational{1, fps};
  enc->framerate = AVRational{fps, 1};
  enc->gop_size = fps;    // ~1s keyframe interval -> tight rolling trim
  enc->max_b_frames = 0;  // no reordering: pts==dts, simplest mux
  enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; // SPS/PPS in extradata for mp4
  av_opt_set(enc->priv_data, "preset", "veryfast", 0);
  av_opt_set(enc->priv_data, "tune", "zerolatency", 0);
  av_opt_set(enc->priv_data, "crf", "18", 0);

  if (avcodec_open2(enc, codec, nullptr) < 0) {
    spdlog::warn("[ClipRecorder] failed to open H.264 encoder");
    avcodec_free_context(&enc);
    return false;
  }
  extradata.assign(enc->extradata, enc->extradata + enc->extradata_size);

  yuv = av_frame_alloc();
  yuv->format = AV_PIX_FMT_YUV420P;
  yuv->width = encWidth;
  yuv->height = encHeight;
  if (!yuv || av_frame_get_buffer(yuv, 0) < 0) {
    stop();
    return false;
  }
  pkt = av_packet_alloc();
  lastPts = -1;

  running = true;
  worker = std::thread([this] { workerLoop(); });
  return true;
}

void ClipRecorder::Impl::stop() {
  if (running.exchange(false)) {
    queueCv.notify_all();
    if (worker.joinable()) {
      worker.join();
    }
  }
  // Safe to free now: the worker has exited.
  if (enc) {
    avcodec_free_context(&enc);
  }
  if (sws) {
    sws_freeContext(sws);
    sws = nullptr;
  }
  if (yuv) {
    av_frame_free(&yuv);
  }
  if (pkt) {
    av_packet_free(&pkt);
  }
  swsSrcW = swsSrcH = 0;
  {
    std::lock_guard<std::mutex> lk(queueMutex);
    queue.clear();
  }
  {
    std::lock_guard<std::mutex> lk(ringMutex);
    videoRing.clear();
    pcmRing.clear();
  }
}

void ClipRecorder::Impl::reset() {
  std::lock_guard<std::mutex> lk(ringMutex);
  videoRing.clear();
  pcmRing.clear();
}

void ClipRecorder::Impl::pushVideo(const QImage &frame, const int64_t ptsMs) {
  if (!running.load()) {
    return;
  }
  // Premultiplied-opaque bytes are identical to straight RGBA (alpha == 255),
  // so accept both without a conversion copy; convert anything else.
  const bool rgbaBytes =
      frame.format() == QImage::Format_RGBA8888 ||
      frame.format() == QImage::Format_RGBA8888_Premultiplied;
  const QImage img =
      rgbaBytes ? frame : frame.convertToFormat(QImage::Format_RGBA8888);

  RawFrame rf;
  rf.width = img.width();
  rf.height = img.height();
  rf.stride = static_cast<int>(img.bytesPerLine());
  rf.ptsMs = ptsMs;
  rf.rgba.assign(img.constBits(), img.constBits() + img.sizeInBytes());
  {
    std::lock_guard<std::mutex> lk(queueMutex);
    if (queue.size() >= MAX_QUEUED_FRAMES) {
      queue.pop_front();
    }
    queue.push_back(std::move(rf));
  }
  queueCv.notify_one();
}

void ClipRecorder::Impl::pushAudio(const int16_t *data,
                                   const std::size_t numFrames) {
  if (!running.load() || !data || numFrames == 0) {
    return;
  }
  const std::size_t samples = numFrames * static_cast<std::size_t>(channels);
  std::lock_guard<std::mutex> lk(ringMutex);
  pcmRing.insert(pcmRing.end(), data, data + samples);
  // Keep ~window + 0.5s of audio so it comfortably covers the video window.
  const auto windowSamples =
      static_cast<std::size_t>(windowUs / 1'000'000) *
          static_cast<std::size_t>(sampleRate) *
          static_cast<std::size_t>(channels) +
      static_cast<std::size_t>(sampleRate / 2) * channels;
  while (pcmRing.size() > windowSamples) {
    pcmRing.pop_front();
  }
}

void ClipRecorder::Impl::workerLoop() {
  while (true) {
    RawFrame f;
    {
      std::unique_lock<std::mutex> lk(queueMutex);
      queueCv.wait(lk, [this] { return !running.load() || !queue.empty(); });
      if (!running.load() && queue.empty()) {
        break;
      }
      f = std::move(queue.front());
      queue.pop_front();
      encoding = true;
    }
    encodeFrame(f);
    {
      std::lock_guard<std::mutex> lk(queueMutex);
      encoding = false;
      if (queue.empty()) {
        idleCv.notify_all();
      }
    }
  }
}

void ClipRecorder::Impl::flush() {
  std::unique_lock<std::mutex> lk(queueMutex);
  idleCv.wait(lk, [this] {
    return !running.load() || (queue.empty() && !encoding);
  });
}

void ClipRecorder::Impl::encodeFrame(const RawFrame &f) {
  if (!sws || swsSrcW != f.width || swsSrcH != f.height) {
    // SWS_POINT = nearest-neighbor: keeps upscaled pixels sharp (no blur).
    sws = sws_getCachedContext(sws, f.width, f.height, AV_PIX_FMT_RGBA, encWidth,
                               encHeight, AV_PIX_FMT_YUV420P, SWS_POINT,
                               nullptr, nullptr, nullptr);
    swsSrcW = f.width;
    swsSrcH = f.height;
  }
  if (!sws || av_frame_make_writable(yuv) < 0) {
    return;
  }
  const uint8_t *srcData[4] = {f.rgba.data(), nullptr, nullptr, nullptr};
  const int srcStride[4] = {f.stride, 0, 0, 0};
  sws_scale(sws, srcData, srcStride, 0, f.height, yuv->data, yuv->linesize);

  int64_t pts = av_rescale(f.ptsMs, fps, 1000); // ms -> frame index (CFR)
  if (pts <= lastPts) {
    pts = lastPts + 1; // keep strictly monotonic across dropped frames
  }
  lastPts = pts;
  yuv->pts = pts;

  if (avcodec_send_frame(enc, yuv) < 0) {
    return;
  }
  drainEncoder();
}

void ClipRecorder::Impl::drainEncoder() {
  while (true) {
    const int ret = avcodec_receive_packet(enc, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
      break;
    }
    appendPacket(pkt);
    av_packet_unref(pkt);
  }
}

void ClipRecorder::Impl::appendPacket(const AVPacket *p) {
  EncodedPacket ep;
  ep.data.assign(p->data, p->data + p->size);
  ep.ptsUs = av_rescale_q(p->pts, enc->time_base, AVRational{1, 1'000'000});
  ep.dtsUs = av_rescale_q(p->dts, enc->time_base, AVRational{1, 1'000'000});
  ep.keyframe = (p->flags & AV_PKT_FLAG_KEY) != 0;

  std::lock_guard<std::mutex> lk(ringMutex);
  videoRing.push_back(std::move(ep));
  trimVideoRing();
}

void ClipRecorder::Impl::trimVideoRing() {
  // Drop whole leading GOPs while the buffer would still cover the window
  // without them, so the front is always a keyframe.
  if (videoRing.empty()) {
    return;
  }
  const int64_t latest = videoRing.back().ptsUs;
  while (videoRing.size() > 1) {
    std::size_t secondKey = 0;
    for (std::size_t i = 1; i < videoRing.size(); ++i) {
      if (videoRing[i].keyframe) {
        secondKey = i;
        break;
      }
    }
    if (secondKey == 0) {
      break; // only one GOP buffered
    }
    if (latest - videoRing[secondKey].ptsUs >= windowUs) {
      videoRing.erase(videoRing.begin(),
                      videoRing.begin() + static_cast<long>(secondKey));
    } else {
      break;
    }
  }
}

ClipSnapshot ClipRecorder::Impl::snapshot() const {
  ClipSnapshot snap;
  std::lock_guard<std::mutex> lk(ringMutex);
  if (videoRing.empty()) {
    return snap;
  }
  std::size_t start = 0;
  while (start < videoRing.size() && !videoRing[start].keyframe) {
    ++start;
  }
  if (start >= videoRing.size()) {
    return snap; // no keyframe buffered yet
  }
  snap.video.assign(videoRing.begin() + static_cast<long>(start),
                    videoRing.end());
  snap.extradata = extradata;
  snap.pcm.assign(pcmRing.begin(), pcmRing.end());
  snap.fps = fps;
  snap.sampleRate = sampleRate;
  snap.channels = channels;
  snap.width = encWidth;
  snap.height = encHeight;
  return snap;
}

// --- public forwarding ---

ClipRecorder::ClipRecorder(const int windowSeconds)
    : m_impl(std::make_unique<Impl>(windowSeconds)) {}
ClipRecorder::~ClipRecorder() = default;

bool ClipRecorder::start(const int width, const int height, const int fps,
                         const int sampleRate, const int channels) {
  return m_impl->start(width, height, fps, sampleRate, channels);
}
void ClipRecorder::stop() { m_impl->stop(); }
void ClipRecorder::reset() { m_impl->reset(); }
void ClipRecorder::flush() { m_impl->flush(); }
bool ClipRecorder::isRecording() const { return m_impl->running.load(); }
void ClipRecorder::pushVideoFrame(const QImage &frame, const int64_t ptsMs) {
  m_impl->pushVideo(frame, ptsMs);
}
void ClipRecorder::pushAudio(const int16_t *data, const std::size_t numFrames) {
  m_impl->pushAudio(data, numFrames);
}
ClipSnapshot ClipRecorder::snapshot() const { return m_impl->snapshot(); }

} // namespace firelight::media
