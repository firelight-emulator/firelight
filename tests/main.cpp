#include <QApplication>
#include <gtest/gtest.h>
#include <QtQuickTest>
#include <QTest>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto result = RUN_ALL_TESTS();
  //
  // QQuickView view;
  // view.setSource(QUrl("qrc:/tests.qml"));
  // view.show();

  // Enter the Qt event loop to run QML tests
  // QApplication::exec();
  // QUICK_TEST_MAIN(example);

  return result;
}
