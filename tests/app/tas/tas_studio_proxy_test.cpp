// Headless test for the TAS Studio's QML-facing facade. The proxy's transport, edit,
// and state logic is plain QObject glue over a TasSession, so it is exercised with the
// QApplication fl_test provides — no QML engine, no running game (loadDemo() supplies a
// self-contained session). The QML view over this proxy is verified separately (needs
// the GUI build).
#include "gui/qt_tas_studio_proxy.hpp"

#include <QObject>
#include <gtest/gtest.h>

namespace firelight::gui {
namespace {

TEST(TasStudioProxyTest, LoadDemoPopulatesModel) {
  QtTasStudioProxy proxy;
  EXPECT_FALSE(proxy.hasMovie());
  EXPECT_EQ(proxy.frameCount(), 0);

  proxy.loadDemo(50);
  EXPECT_TRUE(proxy.hasMovie());
  EXPECT_EQ(proxy.frameCount(), 50);
  ASSERT_NE(proxy.model(), nullptr);
  EXPECT_EQ(proxy.model()->rowCount(), 50);
  EXPECT_EQ(proxy.currentFrame(), 0);
}

TEST(TasStudioProxyTest, TransportStepsAndSeeks) {
  QtTasStudioProxy proxy;
  int playheadSignals = 0;
  QObject::connect(&proxy, &QtTasStudioProxy::playheadChanged,
                   [&] { ++playheadSignals; });

  proxy.loadDemo(40);
  proxy.stepForward();
  EXPECT_EQ(proxy.currentFrame(), 1);
  proxy.seekTo(30);
  EXPECT_EQ(proxy.currentFrame(), 30);
  proxy.stepBackward();
  EXPECT_EQ(proxy.currentFrame(), 29);
  EXPECT_GE(playheadSignals, 3);

  proxy.seekTo(1000); // clamps to frameCount
  EXPECT_EQ(proxy.currentFrame(), 40);
  proxy.stepForward(); // capped — never runs past the authored input
  EXPECT_EQ(proxy.currentFrame(), 40);
}

TEST(TasStudioProxyTest, PlayPauseAndTick) {
  QtTasStudioProxy proxy;
  int playingSignals = 0;
  QObject::connect(&proxy, &QtTasStudioProxy::playingChanged,
                   [&] { ++playingSignals; });

  proxy.loadDemo(10);
  EXPECT_FALSE(proxy.isPlaying());
  proxy.tick(); // not playing -> no-op
  EXPECT_EQ(proxy.currentFrame(), 0);

  proxy.play();
  EXPECT_TRUE(proxy.isPlaying());
  proxy.tick();
  EXPECT_EQ(proxy.currentFrame(), 1);
  proxy.togglePlay();
  EXPECT_FALSE(proxy.isPlaying());
  EXPECT_GE(playingSignals, 2);
}

TEST(TasStudioProxyTest, TickPlaysToEndThenAutoPauses) {
  QtTasStudioProxy proxy;
  proxy.loadDemo(5);
  proxy.play();
  for (int i = 0; i < 20; ++i) {
    proxy.tick(); // more ticks than frames
  }
  EXPECT_EQ(proxy.currentFrame(), 5);
  EXPECT_FALSE(proxy.isPlaying()); // auto-paused at the movie's end
}

TEST(TasStudioProxyTest, EditThroughProxy) {
  QtTasStudioProxy proxy;
  int movieSignals = 0;
  QObject::connect(&proxy, &QtTasStudioProxy::movieChanged,
                   [&] { ++movieSignals; });

  proxy.loadDemo(20);
  EXPECT_EQ(proxy.rerecordCount(), 0);

  proxy.toggleInput(5, 9); // button 9 is not in the demo pattern
  EXPECT_TRUE(proxy.model()->isButtonPressed(5, 9));
  EXPECT_EQ(proxy.rerecordCount(), 1);

  proxy.paintInput(2, 4, 11, /*pressed=*/true);
  for (int f = 2; f <= 4; ++f) {
    EXPECT_TRUE(proxy.model()->isButtonPressed(f, 11)) << "frame " << f;
  }
  EXPECT_GE(movieSignals, 2);
}

TEST(TasStudioProxyTest, NoMovieIsSafe) {
  QtTasStudioProxy proxy; // never loaded
  proxy.stepForward();
  proxy.seekTo(3);
  proxy.play();
  proxy.tick();
  proxy.toggleInput(0, 0);
  EXPECT_EQ(proxy.currentFrame(), 0);
  EXPECT_FALSE(proxy.isPlaying()); // play() no-ops without a movie
}

} // namespace
} // namespace firelight::gui
