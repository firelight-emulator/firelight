#include <firelight/media/clip_muxer.hpp>

#include <spdlog/spdlog.h>

#include <cstring>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mem.h>
}

namespace firelight::media {

bool ClipMuxer::writeMp4(const ClipSnapshot &snap, const QString &path) {
  if (snap.video.empty() || snap.width <= 0 || snap.height <= 0) {
    return false;
  }
  const std::string p = path.toStdString();

  AVFormatContext *oc = nullptr;
  if (avformat_alloc_output_context2(&oc, nullptr, "mp4", p.c_str()) < 0 ||
      !oc) {
    spdlog::warn("[ClipMuxer] could not allocate mp4 output for {}", p);
    return false;
  }

  bool ok = false;
  do {
    AVStream *vst = avformat_new_stream(oc, nullptr);
    if (!vst) {
      break;
    }
    vst->time_base = AVRational{1, 1'000'000}; // request microsecond timescale
    AVCodecParameters *par = vst->codecpar;
    par->codec_type = AVMEDIA_TYPE_VIDEO;
    par->codec_id = AV_CODEC_ID_H264;
    par->width = snap.width;
    par->height = snap.height;
    par->format = AV_PIX_FMT_YUV420P;
    if (!snap.extradata.empty()) {
      par->extradata = static_cast<uint8_t *>(
          av_mallocz(snap.extradata.size() + AV_INPUT_BUFFER_PADDING_SIZE));
      if (!par->extradata) {
        break;
      }
      std::memcpy(par->extradata, snap.extradata.data(), snap.extradata.size());
      par->extradata_size = static_cast<int>(snap.extradata.size());
    }

    if (!(oc->oformat->flags & AVFMT_NOFILE)) {
      if (avio_open(&oc->pb, p.c_str(), AVIO_FLAG_WRITE) < 0) {
        spdlog::warn("[ClipMuxer] could not open {} for writing", p);
        break;
      }
    }
    if (avformat_write_header(oc, nullptr) < 0) {
      spdlog::warn("[ClipMuxer] could not write mp4 header for {}", p);
      break;
    }

    // Rebase so the first packet is t=0; convert from microseconds to whatever
    // timescale the muxer settled on for the stream after the header.
    const int64_t base = snap.video.front().ptsUs;
    const AVRational us{1, 1'000'000};
    AVPacket *pkt = av_packet_alloc();
    bool wrote = true;
    for (const auto &ep : snap.video) {
      if (av_new_packet(pkt, static_cast<int>(ep.data.size())) < 0) {
        wrote = false;
        break;
      }
      std::memcpy(pkt->data, ep.data.data(), ep.data.size());
      pkt->stream_index = vst->index;
      pkt->pts = av_rescale_q(ep.ptsUs - base, us, vst->time_base);
      pkt->dts = av_rescale_q(ep.dtsUs - base, us, vst->time_base);
      if (pkt->dts > pkt->pts) {
        pkt->dts = pkt->pts;
      }
      if (ep.keyframe) {
        pkt->flags |= AV_PKT_FLAG_KEY;
      }
      if (av_interleaved_write_frame(oc, pkt) < 0) {
        av_packet_unref(pkt);
        wrote = false;
        break;
      }
      av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    if (!wrote) {
      break;
    }

    if (av_write_trailer(oc) < 0) {
      break;
    }
    ok = true;
  } while (false);

  if (oc && !(oc->oformat->flags & AVFMT_NOFILE) && oc->pb) {
    avio_closep(&oc->pb);
  }
  avformat_free_context(oc);
  if (ok) {
    spdlog::info("[ClipMuxer] wrote clip {}", p);
  }
  return ok;
}

} // namespace firelight::media
