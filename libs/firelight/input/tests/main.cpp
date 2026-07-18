#include <QCoreApplication>
#include <gtest/gtest.h>

// A QCoreApplication is required so Qt can load the QSQLITE driver used by
// SqliteControllerRepository. No GUI is needed by the input module tests
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
