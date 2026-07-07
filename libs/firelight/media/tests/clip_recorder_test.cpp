#include <firelight/media/clip_muxer.hpp>
#include <firelight/media/clip_recorder.hpp>

#include <QColor>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace firelight::media {

namespace {
QImage makeFrame(int w, int h, int i) {
  QImage img(w, h, QImage::Format_RGBA8888);
  // Varying content so the encoder has real data to compress.
  img.fill(QColor((i * 5) % 256, (i * 3) % 256, (i * 7) % 256));
  return img;
}
} // namespace

// The core promise of the design: continuous encoding keeps only a bounded,
// compressed, keyframe-aligned window — not a raw-frame backlog that grows with
// play time.
TEST(ClipRecorderTest, RollingWindowStaysBoundedAndStartsOnKeyframe) {
  ClipRecorder recorder(/*windowSeconds=*/2);
  ASSERT_TRUE(recorder.start(64, 64, 30, 48000, 2));

  constexpr int fps = 30;
  constexpr int seconds = 5;
  std::vector<int16_t> audio(static_cast<std::size_t>(48000 / fps) * 2, 0);
  for (int i = 0; i < fps * seconds; ++i) {
    recorder.pushVideoFrame(makeFrame(64, 64, i),
                            static_cast<int64_t>(i) * 1000 / fps);
    for (auto &s : audio) {
      s = static_cast<int16_t>((i * 137) & 0x7fff);
    }
    recorder.pushAudio(audio.data(), audio.size() / 2);
    // Pace the feed in small batches so the test never trips the backpressure
    // valve (real frames arrive at ~fps, far slower than encode).
    if (i % 4 == 0) {
      recorder.flush();
    }
  }
  recorder.flush();

  const auto snap = recorder.snapshot();
  ASSERT_FALSE(snap.empty());
  EXPECT_TRUE(snap.video.front().keyframe); // window begins on a keyframe
  // Small sources are integer-upscaled toward ~720 lines for a crisp clip:
  // a whole multiple of the source, aspect preserved (square stays square).
  EXPECT_EQ(snap.width, snap.height);
  EXPECT_GT(snap.height, 64);
  EXPECT_EQ(snap.width % 64, 0);

  // Fed 5 seconds but configured for 2 -> the retained span is bounded to the
  // window (plus at most one GOP), never the full runtime.
  const int64_t spanUs = snap.video.back().ptsUs - snap.video.front().ptsUs;
  EXPECT_GE(spanUs, 1'000'000);
  EXPECT_LE(spanUs, 3'500'000);

  recorder.stop();
}

TEST(ClipRecorderTest, MuxesSnapshotToValidMp4) {
  ClipRecorder recorder(3);
  ASSERT_TRUE(recorder.start(80, 64, 30, 48000, 2));

  constexpr int fps = 30;
  for (int i = 0; i < fps * 2; ++i) {
    recorder.pushVideoFrame(makeFrame(80, 64, i),
                            static_cast<int64_t>(i) * 1000 / fps);
  }
  recorder.flush();
  const auto snap = recorder.snapshot();
  ASSERT_FALSE(snap.empty());
  recorder.stop();

  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  const QString path = dir.path() + "/clip.mp4";
  ASSERT_TRUE(ClipMuxer::writeMp4(snap, path));

  QFile f(path);
  ASSERT_TRUE(f.open(QIODevice::ReadOnly));
  const QByteArray bytes = f.readAll();
  EXPECT_GT(bytes.size(), 1000);                // real encoded content
  EXPECT_TRUE(bytes.left(64).contains("ftyp")); // mp4 signature box
}

TEST(ClipRecorderTest, EmptySnapshotMuxesToNothing) {
  const ClipSnapshot empty;
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  EXPECT_FALSE(ClipMuxer::writeMp4(empty, dir.path() + "/none.mp4"));
}

} // namespace firelight::media
