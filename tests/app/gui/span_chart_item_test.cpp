// TODO: NEEDS REVIEW
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTest>
#include <chrono>
#include <gtest/gtest.h>
#include <gui/qt_performance_monitor_proxy.hpp>
#include <gui/span_chart_item.hpp>
#include <thread>

namespace firelight::gui {

namespace {
// Presents with a frame of work between each, spaced so the intervals are measurable. The frame each
// present puts on the display is marked too, which is what the tally counts frames from
void recordFrames(monitoring::Monitor &monitor, const int presents) {
  const auto present = monitor.marker("present", "");
  const auto shown = monitor.marker("frame_shown", "");
  const auto frame = monitor.span("run_frame", "");

  for (auto index = 0; index < presents; ++index) {
    present.mark();
    shown.mark();
    {
      const monitoring::ScopedSpan scoped(frame);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}
} // namespace

TEST(SpanChartItemTest, HistorySeesTheMarkersAndSpansOnceVisible) {
  auto &monitor = monitoring::Monitor::instance();
  QtPerformanceMonitorProxy proxy;
  proxy.setVisible(true);
  recordFrames(monitor, 5);
  QTest::qWait(300);

  const auto &history = proxy.getHistory();
  EXPECT_NE(history.getKind("present"), 0);
  EXPECT_NE(history.getKind("run_frame"), 0);
  EXPECT_GE(history.getSpans().size(), 10u);
  EXPECT_GT(proxy.getFramesRun(), 0);
  proxy.setVisible(false);
}

TEST(SpanChartItemTest, SlicesOneBarPerPresentInterval) {
  auto &monitor = monitoring::Monitor::instance();
  QtPerformanceMonitorProxy proxy;
  SpanChartItem chart;
  chart.setSize(QSizeF(300, 100));
  chart.setSource(&proxy);

  proxy.setVisible(true);
  recordFrames(monitor, 5);
  QTest::qWait(300);

  // The monitor keeps earlier tests' markers too, so at least the four intervals recorded here
  EXPECT_GE(chart.getBarCount(), 4);
  proxy.setVisible(false);
}

// The chart mounts before any data exists, so its node first meets the renderer empty; this is
// the case that once drew nothing forever
TEST(SpanChartItemTest, DrawsLitPixelsForBarsThatArriveAfterMounting) {
  auto &monitor = monitoring::Monitor::instance();
  QtPerformanceMonitorProxy proxy;
  QQuickWindow window;
  window.setColor(Qt::black);
  window.resize(320, 120);
  auto *chart = new SpanChartItem(window.contentItem());
  chart->setPosition(QPointF(10, 10));
  chart->setSize(QSizeF(300, 100));
  chart->setBarColor(QColor(255, 255, 255, 255));
  chart->setSource(&proxy);
  window.show();
  ASSERT_TRUE(QTest::qWaitForWindowExposed(&window));
  QTest::qWait(50);

  proxy.setVisible(true);
  recordFrames(monitor, 5);
  QTest::qWait(300);
  ASSERT_GT(chart->getBarCount(), 0);

  const auto image = window.grabWindow();
  ASSERT_FALSE(image.isNull());
  auto lit = 0;

  for (auto y = 0; y < image.height(); ++y) {
    for (auto x = 0; x < image.width(); ++x) {
      if (qGray(image.pixel(x, y)) > 40) {
        lit++;
      }
    }
  }

  EXPECT_GT(lit, 0) << "bars: " << chart->getBarCount();
  proxy.setVisible(false);
}

// The item reached through QML, with the proxy as a context property, the way the panel mounts it
TEST(SpanChartItemTest, TakesTheProxyFromAContextProperty) {
  auto &monitor = monitoring::Monitor::instance();
  QtPerformanceMonitorProxy proxy;
  qmlRegisterType<SpanChartItem>("Firelight", 1, 0, "SpanChartItem");
  QQmlEngine engine;
  engine.rootContext()->setContextProperty("PerformanceMonitor", &proxy);
  QQmlComponent component(&engine);
  component.setData(R"(
import QtQuick
import Firelight 1.0
SpanChartItem {
    width: 300
    height: 100
    source: PerformanceMonitor
    boundaryKind: "present"
    barWidth: 6
    barGap: 1
}
)",
                    QUrl());
  auto *item = qobject_cast<SpanChartItem *>(component.create());
  ASSERT_NE(item, nullptr) << component.errorString().toStdString();
  EXPECT_EQ(item->getSource(), &proxy);

  proxy.setVisible(true);
  recordFrames(monitor, 5);
  QTest::qWait(300);

  EXPECT_GE(item->getBarCount(), 4);
  proxy.setVisible(false);
  delete item;
}

} // namespace firelight::gui
