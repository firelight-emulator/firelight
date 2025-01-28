#include "video_decoder.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace firelight::av {
VideoDecoder::VideoDecoder() {
  // Find the H.264 decoder
  m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!m_codec) {
    throw std::runtime_error("H.264 codec not found");
  }

  // Allocate codec context
  m_codecCtx = avcodec_alloc_context3(m_codec);
  if (!m_codecCtx) {
    throw std::runtime_error("Failed to allocate codec context");
  }

  // Open codec
  if (avcodec_open2(m_codecCtx, m_codec, nullptr) < 0) {
    throw std::runtime_error("Failed to open codec");
  }

  // Allocate frame
  m_frame = av_frame_alloc();
  if (!m_frame) {
    throw std::runtime_error("Failed to allocate frame");
  }

  // Allocate packet
  m_packet = av_packet_alloc();
  if (!m_packet) {
    throw std::runtime_error("Failed to allocate packet");
  }
}
bool VideoDecoder::decode(const uint8_t *encodedData, size_t dataSize,
                          std::vector<uint8_t> &decodedFrame) {
  // Fill the packet with encoded data
  m_packet->data = const_cast<uint8_t*>(encodedData);
  m_packet->size = static_cast<int>(dataSize);

  // Send the packet to the decoder
  if (avcodec_send_packet(m_codecCtx, m_packet) < 0) {
    spdlog::error("Failed to send packet to decoder");
    return false;
  }

  // Receive the decoded frame
  const int ret = avcodec_receive_frame(m_codecCtx, m_frame);
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    // No frame available yet
    return false;
  }

  if (ret < 0) {
    spdlog::error("Failed to receive frame from decoder");
    return false;
  }

  // Convert decoded frame (YUV420P) to raw data
  int ySize = m_frame->width * m_frame->height;
  int uvSize = ySize / 4;

  decodedFrame.resize(ySize + 2 * uvSize); // Y + U + V
  memcpy(decodedFrame.data(), m_frame->data[0], ySize); // Copy Y plane
  memcpy(decodedFrame.data() + ySize, m_frame->data[1], uvSize); // Copy U plane
  memcpy(decodedFrame.data() + ySize + uvSize, m_frame->data[2], uvSize); // Copy V plane

  return true;
}
} // namespace firelight::av