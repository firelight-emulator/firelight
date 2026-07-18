#include <firelight/media/clip_thumbnailer.hpp>

#include <QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <spdlog/spdlog.h>

namespace firelight::media {

bool ClipThumbnailer::writePoster(const QString &mp4Path,
                                  const QString &pngPath) {
  AVFormatContext *fmt = nullptr;
  if (avformat_open_input(&fmt, mp4Path.toUtf8().constData(), nullptr,
                          nullptr) < 0) {
    spdlog::warn("Clip poster: could not open {}", mp4Path.toStdString());
    return false;
  }

  bool ok = false;
  AVCodecContext *codecCtx = nullptr;
  AVFrame *frame = nullptr;
  AVPacket *packet = nullptr;
  SwsContext *sws = nullptr;

  do {
    if (avformat_find_stream_info(fmt, nullptr) < 0) {
      break;
    }

    int videoStream = -1;
    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
      if (fmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        videoStream = static_cast<int>(i);
        break;
      }
    }
    if (videoStream < 0) {
      break;
    }

    AVCodecParameters *par = fmt->streams[videoStream]->codecpar;
    const AVCodec *decoder = avcodec_find_decoder(par->codec_id);
    if (!decoder) {
      break;
    }

    codecCtx = avcodec_alloc_context3(decoder);
    if (!codecCtx || avcodec_parameters_to_context(codecCtx, par) < 0 ||
        avcodec_open2(codecCtx, decoder, nullptr) < 0) {
      break;
    }

    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet) {
      break;
    }

    // Read + decode until the first video frame emerges
    bool gotFrame = false;
    while (!gotFrame && av_read_frame(fmt, packet) >= 0) {
      if (packet->stream_index == videoStream &&
          avcodec_send_packet(codecCtx, packet) >= 0 &&
          avcodec_receive_frame(codecCtx, frame) >= 0) {
        gotFrame = true;
      }
      av_packet_unref(packet);
    }
    if (!gotFrame) {
      break;
    }

    const int w = frame->width;
    const int h = frame->height;
    QImage image(w, h, QImage::Format_RGBA8888);
    uint8_t *dst[1] = {image.bits()};
    int dstStride[1] = {static_cast<int>(image.bytesPerLine())};

    sws = sws_getContext(w, h, static_cast<AVPixelFormat>(frame->format), w, h,
                         AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr,
                         nullptr);
    if (!sws) {
      break;
    }
    sws_scale(sws, frame->data, frame->linesize, 0, h, dst, dstStride);

    ok = image.save(pngPath, "PNG");
    if (!ok) {
      spdlog::warn("Clip poster: could not save {}", pngPath.toStdString());
    }
  } while (false);

  if (sws) {
    sws_freeContext(sws);
  }
  if (packet) {
    av_packet_free(&packet);
  }
  if (frame) {
    av_frame_free(&frame);
  }
  if (codecCtx) {
    avcodec_free_context(&codecCtx);
  }
  avformat_close_input(&fmt);
  return ok;
}

} // namespace firelight::media
