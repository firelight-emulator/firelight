#include <firelight/media/clip_muxer.hpp>
#include <firelight/media/clip_recorder.hpp>
#include <firelight/media/clip_thumbnailer.hpp>

#include <QColor>
#include <QImage>
#include <QTemporaryDir>
#include <gtest/gtest.h>

#include <cstdint>

namespace firelight::media {
namespace {
QImage makeFrame(int w, int h, int i) {
  QImage img(w, h, QImage::Format_RGBA8888);
  img.fill(QColor((i * 5) % 256, (i * 3) % 256, (i * 7) % 256));
  return img;
}

// Encodes a short clip to `path` (same recorder→muxer pipeline the app uses)
bool writeSampleClip(const QString &path, int w, int h) {
  ClipRecorder recorder(3);
  if (!recorder.start(w, h, 30, 48000, 2)) {
    return false;
  }
  constexpr int fps = 30;
  for (int i = 0; i < fps * 2; ++i) {
    recorder.pushVideoFrame(makeFrame(w, h, i),
                            static_cast<int64_t>(i) * 1000 / fps);
  }
  recorder.flush();
  const auto snap = recorder.snapshot();
  recorder.stop();
  if (snap.empty()) {
    return false;
  }
  return ClipMuxer::writeMp4(snap, path);
}
} // namespace

TEST(ClipThumbnailerTest, WritesPosterFromValidMp4) {
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  const QString mp4 = dir.path() + "/clip.mp4";
  ASSERT_TRUE(writeSampleClip(mp4, 80, 64));

  const QString png = dir.path() + "/poster.png";
  ASSERT_TRUE(ClipThumbnailer::writePoster(mp4, png));

  QImage poster;
  ASSERT_TRUE(poster.load(png));
  // The clip is integer-upscaled toward ~720 lines, so the poster is a whole
  // multiple of the 80x64 source with its 5:4 aspect preserved
  EXPECT_GT(poster.height(), 64);
  EXPECT_EQ(poster.width() % 80, 0);
  EXPECT_EQ(poster.width() * 64, poster.height() * 80);
}

TEST(ClipThumbnailerTest, FailsOnMissingFile) {
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  EXPECT_FALSE(ClipThumbnailer::writePoster(dir.path() + "/nope.mp4",
                                            dir.path() + "/x.png"));
}

} // namespace firelight::media
