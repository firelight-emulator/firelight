#include <QCoreApplication>
#include <gtest/gtest.h>

// A QCoreApplication backs the Qt types the input module still uses (no GUI is
// needed by these tests)
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
