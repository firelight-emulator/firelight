#include <gtest/gtest.h>

// firelight_tas has no Qt/SDL/DB dependency, so a plain GoogleTest main suffices
// (unlike firelight_input's tests, which need a QCoreApplication for QSQLITE).
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
