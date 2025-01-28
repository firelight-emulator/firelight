#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <vector>

namespace firelight::av {

class VideoDecoder {
public:
  VideoDecoder();
  bool decode(const uint8_t* encodedData, size_t dataSize, std::vector<uint8_t>& decodedFrame);
private:

  const AVCodec* m_codec = nullptr;
  AVCodecContext* m_codecCtx = nullptr;
  AVFrame* m_frame = nullptr;
  AVPacket* m_packet = nullptr;
};

} // namespace firelight::av
