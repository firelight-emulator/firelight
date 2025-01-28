#include "video_encoder.h"

#include <stdexcept>

namespace firelight::av {
VideoEncoder::VideoEncoder(int width, int height, int fps) : m_width(width), m_height(height), m_fps(fps) {
  // Find the H.264 encoder
  m_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!m_codec) {
    throw std::runtime_error("H.264 codec not found");
  }

  // Allocate codec context
  m_codecCtx = avcodec_alloc_context3(m_codec);
  if (!m_codecCtx) {
    throw std::runtime_error("Failed to allocate codec context");
  }

  // Set codec parameters
  m_codecCtx->bit_rate = 400000; // Adjust as needed
  m_codecCtx->width = m_width;
  m_codecCtx->height = m_height;
  m_codecCtx->time_base = AVRational{1, m_fps};
  m_codecCtx->framerate = AVRational{m_fps, 1};
  m_codecCtx->gop_size = 10; // Group of Pictures
  m_codecCtx->max_b_frames = 2;
  m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

  // Open codec
  if (avcodec_open2(m_codecCtx, m_codec, nullptr) < 0) {
    throw std::runtime_error("Failed to open codec");
  }

  // Allocate frame
  m_frame = av_frame_alloc();
  if (!m_frame) {
    throw std::runtime_error("Failed to allocate frame");
  }
  m_frame->format = m_codecCtx->pix_fmt;
  m_frame->width = m_width;
  m_frame->height = m_height;

  // Allocate frame buffer
  int ret = av_image_alloc(m_frame->data, m_frame->linesize, m_width, m_height,
                           m_codecCtx->pix_fmt, 32);
  if (ret < 0) {
    throw std::runtime_error("Failed to allocate frame buffer");
  }

  // Allocate packet
  m_packet = av_packet_alloc();
  if (!m_packet) {
    throw std::runtime_error("Failed to allocate packet");
  }
}
std::vector<uint8_t> VideoEncoder::encode(const uint8_t *rawData) {
  // Convert raw data to YUV420P (assuming rawData is in RGB)
  // Use libswscale or a custom conversion
  // Here we assume rawData is already in YUV420P

  // Fill frame with raw data
  memcpy(m_frame->data[0], rawData, m_width * m_height);               // Y plane
  memcpy(m_frame->data[1], rawData + m_width * m_height, m_width * m_height / 4); // U plane
  memcpy(m_frame->data[2], rawData + m_width * m_height * 5 / 4, m_width * m_height / 4); // V plane
  m_frame->pts = m_frameCount++;

  // Send frame to encoder
  if (avcodec_send_frame(m_codecCtx, m_frame) < 0) {
    throw std::runtime_error("Failed to send frame to encoder");
  }

  // Receive encoded packet
  std::vector<uint8_t> encodedData;
  auto result = avcodec_receive_packet(m_codecCtx, m_packet);
  printf("RESULT: %s\n", av_err2str(result));
  while (result == 0) {
    encodedData.insert(encodedData.end(), m_packet->data, m_packet->data + m_packet->size);
    av_packet_unref(m_packet);
    result = avcodec_receive_packet(m_codecCtx, m_packet);

  }

  return encodedData;
}
} // namespace firelight::av