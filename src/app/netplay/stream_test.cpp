// #include "stream_test.hpp"
//
// namespace firelight::netplay {
//
// StreamTest::StreamTest() {
//   avformat_alloc_output_context2(&m_outputFormatContext, nullptr, nullptr,
//                                  "video.mp4");
//   if (!m_outputFormatContext) {
//     printf("Error allocating output context\n");
//     return;
//   }
//
//   // Find the H.264 video encoder
//   auto codec = avcodec_find_encoder(AV_CODEC_ID_H264);
//   if (!codec) {
//     printf("Codec not found\n");
//     return;
//   }
//
//   // Create a new video stream in the output context
//   m_videoStream = avformat_new_stream(m_outputFormatContext, codec);
//   if (!m_videoStream) {
//     printf("Error creating video stream\n");
//     return;
//   }
//
//   m_codecContext = avcodec_alloc_context3(codec);
//   if (!m_codecContext) {
//     printf("Error allocating codec context\n");
//     return;
//   }
//
//   // Set codec parameters
//   m_codecContext->width = 640;
//   m_codecContext->height = 480;
//   m_codecContext->time_base = (AVRational){1, 60};
//   m_codecContext->pkt_timebase = (AVRational){1, 60};
//   m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
//
//   // Open the codec
//   if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
//     printf("Error opening codec\n");
//     return;
//   }
//
//   // Set stream parameters
//   avcodec_parameters_from_context(m_videoStream->codecpar, m_codecContext);
//   m_videoStream->time_base = (AVRational){1, 60};
//
//   // Open the output file
//   if (avio_open(&m_outputFormatContext->pb, "video.mp4", AVIO_FLAG_WRITE) <
//   0) {
//     printf("Error opening output file\n");
//     return;
//   }
//
//   // Write the stream header
//   if (avformat_write_header(m_outputFormatContext, NULL) < 0) {
//     printf("Error writing header\n");
//     return;
//   }
// }
//
// StreamTest::~StreamTest() {
//   printf("CLOSING STREAM\n");
//   if (int res = av_write_trailer(m_outputFormatContext) < 0) {
//     fprintf(stderr, "Error writing trailer: %d\n", res);
//   }
//
//   // Clean up resources
//   avcodec_free_context(&m_codecContext);
//   avio_closep(&m_outputFormatContext->pb); // Close the output file
//   avformat_free_context(m_outputFormatContext);
// }
//
// void StreamTest::takeImage(QImage image) {
//   static int frame_number = 0;
//   // Assuming you have a QImage named image in RGB888 format
//   if (image.format() != QImage::Format_RGB888) {
//     image = image.convertToFormat(QImage::Format_RGB888);
//   }
//
//   // Allocate an AVFrame for the input image
//   AVFrame *frame = av_frame_alloc();
//   if (!frame) {
//     printf("Error allocating an AVFrame\n");
//     return;
//   }
//
//   // Set up the AVFrame properties for the input image
//   av_image_fill_arrays(frame->data, frame->linesize, image.bits(),
//                        AV_PIX_FMT_RGB24, image.width(), image.height(), 1);
//
//   frame->pts = av_rescale_q(frame_number, m_codecContext->time_base,
//                             m_videoStream->time_base);
//   frame_number++;
//
//   // Allocate an AVFrame for the output image (YUV420P)
//   AVFrame *outputFrame = av_frame_alloc();
//   if (!outputFrame) {
//     printf("Error allocating output frame\n");
//     av_frame_free(&frame);
//     return;
//   }
//
//   outputFrame->width = image.width();
//   outputFrame->height = image.height();
//   outputFrame->format = AV_PIX_FMT_YUV420P;
//   outputFrame->pts = frame->pts;
//   outputFrame->pkt_dts = frame->pkt_dts;
//
//   if (av_frame_get_buffer(outputFrame, 32) < 0) {
//     printf("Error allocating output frame buffer\n");
//     av_frame_free(&frame);
//     av_frame_free(&outputFrame);
//     return;
//   }
//
//   // Create SwsContext for RGB to YUV conversion
//   SwsContext *sws_ctx =
//       sws_getContext(image.width(), image.height(), AV_PIX_FMT_RGB24,
//                      image.width(), image.height(), AV_PIX_FMT_YUV420P,
//                      SWS_BILINEAR, nullptr, nullptr, nullptr);
//
//   if (!sws_ctx) {
//     printf("Could not initialize the conversion context\n");
//     av_frame_free(&frame);
//     av_frame_free(&outputFrame);
//     return;
//   }
//
//   // Perform the color conversion
//   sws_scale(sws_ctx, frame->data, frame->linesize, 0, image.height(),
//             outputFrame->data, outputFrame->linesize);
//
//   // Now, outputFrame contains the YUV420P formatted data suitable for H.264
//   // encoding
//
//   uint8_t *dst_data[4];
//   int dst_linesize[4];
//
//   // Allocate a buffer for the converted image data
//   int ret = av_image_alloc(dst_data, dst_linesize, m_codecContext->width,
//                            m_codecContext->height, m_codecContext->pix_fmt,
//                            1);
//   if (ret < 0) {
//     printf("Could not allocate destination image\n");
//     sws_freeContext(sws_ctx);
//     av_frame_free(&frame);
//     av_frame_free(&outputFrame);
//     return;
//   }
//
//   // Fill the AVFrame with the converted image data
//   av_image_fill_arrays(outputFrame->data, outputFrame->linesize, dst_data[0],
//                        m_codecContext->pix_fmt, m_codecContext->width,
//                        m_codecContext->height, 1);
//
//   // Send the frame to the encoder
//   ret = avcodec_send_frame(m_codecContext, outputFrame);
//   if (ret < 0) {
//     fprintf(stderr, "Error sending frame for encoding\n");
//     av_frame_free(&frame);
//     av_frame_free(&outputFrame);
//     av_freep(&dst_data[0]);
//     sws_freeContext(sws_ctx);
//     return;
//   }
//
//   // For input frames
//   // printf("Input Frame PTS: %lld\n", frame->pts);
//
//   // Free the AVFrame and allocated buffers
//   av_frame_free(&frame);
//   av_frame_free(&outputFrame);
//   av_freep(&dst_data[0]);
//   sws_freeContext(sws_ctx);
//
//   AVPacket *packet = av_packet_alloc();
//   if (!packet) {
//     printf("Error allocating packet\n");
//     return;
//   }
//
//   // Receive the encoded packet
//   ret = avcodec_receive_packet(m_codecContext, packet);
//   if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
//     printf("Got one of the expected errors...: %d\n", ret);
//     return;
//     // Handle these specific cases if needed
//   } else if (ret < 0) {
//     printf("Error receiving encoded packet\n");
//     av_packet_free(&packet);
//     return;
//   }
//   // printf("Before Rescaling - PTS: %lld, DTS: %lld\n", packet->pts,
//   // packet->dts);
//
//   // packet->pts = av_rescale_q(packet->pts, m_codecContext->time_base,
//   //                            m_videoStream->time_base);
//   // packet->dts = av_rescale_q(packet->dts, m_codecContext->time_base,
//   //                            m_videoStream->time_base);
//
//   // printf("After Rescaling - PTS: %lld, DTS: %lld\n", packet->pts,
//   // packet->dts);
//
//   // Write the packet to the output file
//   if (av_write_frame(m_outputFormatContext, packet) < 0) {
//     printf("Error writing packet\n");
//     av_packet_unref(packet);
//     av_packet_free(&packet);
//     return;
//   }
//
//   av_packet_unref(packet);
//   av_packet_free(&packet);
// }
//
// void StreamTest::doIt() {
//
//   printf("CLOSING STREAM\n");
//   if (int res = av_write_trailer(m_outputFormatContext) < 0) {
//     fprintf(stderr, "Error writing trailer: %d\n", res);
//   }
//
//   // Clean up resources
//   avcodec_free_context(&m_codecContext);
//   avio_closep(&m_outputFormatContext->pb); // Close the output file
//   avformat_free_context(m_outputFormatContext);
// }
//
// } // namespace firelight::netplay
