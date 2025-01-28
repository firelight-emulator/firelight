#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <vector>

namespace firelight::av {

class VideoEncoder {
public:
  VideoEncoder(int width, int height, int fps);
  std::vector<uint8_t> encode(const uint8_t* rawData);
private:
  int m_fps = 60;
  int m_width = 0;
  int m_height = 0;
  int64_t m_frameCount = 0;

  const AVCodec* m_codec = nullptr;
  AVCodecContext* m_codecCtx = nullptr;
  AVFrame* m_frame = nullptr;
  AVPacket* m_packet = nullptr;
};

}