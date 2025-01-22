// #pragma once
// #include "stream_boy.hpp"
// extern "C" {
// #include <libavcodec/avcodec.h>
// #include <libavcodec/packet.h>
// #include <libavformat/avformat.h>
// #include <libavutil/imgutils.h>
// #include <libavutil/opt.h>
// #include <libavutil/time.h>
// #include <libswscale/swscale.h>
// }
//
// namespace firelight::netplay {
//
// class StreamTest final : IStreamBoy {
//
// public:
//   StreamTest();
//   ~StreamTest();
//   void takeImage(QImage image) override;
//   void doIt() override;
//
// private:
//   AVFormatContext *m_outputFormatContext;
//   AVCodecContext *m_codecContext;
//   AVStream *m_videoStream;
// };
//
// } // namespace firelight::netplay