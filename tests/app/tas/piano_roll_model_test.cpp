// Headless unit test for the piano-roll's data model — the editable-grid substance
// (toggle a cell, drag-paint a column, playhead + greenzone-keyframe roles) bound to
// a real TasSession. No QML/Vulkan: a QAbstractListModel is exercised with the
// QApplication fl_test already provides, exactly like the other gui/models tests.
#include "gui/models/piano_roll_model.hpp"

#include <firelight/input/input_frame.hpp>
#include <firelight/tas/tas_emulator.hpp>
#include <firelight/tas/tas_session.hpp>

#include <QAbstractItemModel>
#include <QObject>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <vector>

namespace firelight::gui {
namespace {

using input::InputFrame;

// A trivial deterministic emulator: state is a frame counter, enough for the session
// to serialize/restore greenzone keyframes. Input is irrelevant to this model test.
class CounterEmu final : public tas::ITasEmulator {
public:
  uint64_t counter = 0;
  void setRetropadProvider(libretro::IRetropadProvider *) override {}
  void runFrame() override { ++counter; }
  std::vector<uint8_t> serializeState() override {
    std::vector<uint8_t> s(8);
    std::memcpy(s.data(), &counter, 8);
    return s;
  }
  bool deserializeState(const std::vector<uint8_t> &s) override {
    if (s.size() != 8) {
      return false;
    }
    std::memcpy(&counter, s.data(), 8);
    return true;
  }
};

std::vector<InputFrame> makeMovie(int n) {
  std::vector<InputFrame> m;
  m.reserve(static_cast<size_t>(n));
  for (int k = 0; k < n; ++k) {
    InputFrame f;
    f.setButton(static_cast<unsigned>(k % 12), true);
    m.push_back(f);
  }
  return m;
}

int roleFor(const PianoRollModel &model, const QByteArray &name) {
  const auto roles = model.roleNames();
  for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
    if (it.value() == name) {
      return it.key();
    }
  }
  return -1;
}

} // namespace

TEST(PianoRollModelTest, ExposesMovieRows) {
  CounterEmu emu;
  tas::TasSession session(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  const auto movie = makeMovie(10);
  session.loadMovie(movie);

  PianoRollModel model;
  model.setSession(&session);

  EXPECT_EQ(model.rowCount(), 10);
  EXPECT_EQ(model.frameCount(), 10);

  const int buttonsRole = roleFor(model, "buttons");
  const int isCurrentRole = roleFor(model, "isCurrent");
  const int frameIndexRole = roleFor(model, "frameIndex");
  ASSERT_NE(buttonsRole, -1);
  ASSERT_NE(isCurrentRole, -1);

  EXPECT_EQ(model.data(model.index(3), frameIndexRole).toInt(), 3);
  EXPECT_EQ(model.data(model.index(3), buttonsRole).toUInt(), movie[3].buttons);
  EXPECT_TRUE(model.data(model.index(0), isCurrentRole).toBool()); // playhead @0
  EXPECT_FALSE(model.data(model.index(1), isCurrentRole).toBool());
}

TEST(PianoRollModelTest, ToggleButtonEditsAndSignals) {
  CounterEmu emu;
  tas::TasSession session(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  session.loadMovie(makeMovie(8));

  PianoRollModel model;
  model.setSession(&session);

  int dataChanges = 0, movieChanges = 0;
  QObject::connect(&model, &QAbstractItemModel::dataChanged,
                   [&](const QModelIndex &, const QModelIndex &,
                       const QVector<int> &) { ++dataChanges; });
  QObject::connect(&model, &PianoRollModel::movieChanged, [&] { ++movieChanges; });

  ASSERT_FALSE(model.isButtonPressed(5, 3));
  EXPECT_TRUE(model.toggleButton(5, 3));
  EXPECT_TRUE(model.isButtonPressed(5, 3));
  EXPECT_EQ(session.rerecordCount(), 1u);
  EXPECT_GE(dataChanges, 1);
  EXPECT_GE(movieChanges, 1);

  // Toggling again clears it and is another rerecord.
  EXPECT_FALSE(model.toggleButton(5, 3));
  EXPECT_FALSE(model.isButtonPressed(5, 3));
  EXPECT_EQ(session.rerecordCount(), 2u);

  // A no-op toggle target (already in the desired state) still flips; but toggling a
  // button that is set back off leaves the frame unchanged from its original.
}

TEST(PianoRollModelTest, PaintButtonSetsRangeOrderIndependent) {
  CounterEmu emu;
  tas::TasSession session(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  session.loadMovie(makeMovie(20));

  PianoRollModel model;
  model.setSession(&session);

  // Passed high..low on purpose — must paint [3,6].
  model.paintButton(6, 3, /*buttonId=*/9, /*pressed=*/true);
  for (int f = 3; f <= 6; ++f) {
    EXPECT_TRUE(model.isButtonPressed(f, 9)) << "frame " << f;
  }
  EXPECT_FALSE(model.isButtonPressed(2, 9));
  EXPECT_FALSE(model.isButtonPressed(7, 9));

  model.paintButton(3, 6, 9, /*pressed=*/false);
  for (int f = 3; f <= 6; ++f) {
    EXPECT_FALSE(model.isButtonPressed(f, 9)) << "frame " << f;
  }
}

TEST(PianoRollModelTest, EditRepositionsPlayheadAndInvalidatesKeyframes) {
  CounterEmu emu;
  tas::TasSession session(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  session.loadMovie(makeMovie(20));
  for (int i = 0; i < 20; ++i) {
    session.stepForward();
  }

  PianoRollModel model;
  model.setSession(&session);
  model.refresh();
  ASSERT_EQ(model.currentFrame(), 20);

  const int isKeyframeRole = roleFor(model, "isKeyframe");
  ASSERT_NE(isKeyframeRole, -1);
  EXPECT_TRUE(model.data(model.index(8), isKeyframeRole).toBool()); // keyframe @8

  model.toggleButton(6, 2); // edit at frame 6 (button not already set there)

  EXPECT_EQ(model.currentFrame(), 6); // playhead pulled back to the edit point
  EXPECT_FALSE(model.data(model.index(8), isKeyframeRole).toBool()); // @8 invalidated
  EXPECT_TRUE(model.data(model.index(4), isKeyframeRole).toBool());  // @4 (<=6) survives
}

TEST(PianoRollModelTest, NullSessionIsEmptyAndSafe) {
  PianoRollModel model;
  EXPECT_EQ(model.rowCount(), 0);
  EXPECT_EQ(model.frameCount(), 0);
  EXPECT_EQ(model.currentFrame(), 0);
  EXPECT_FALSE(model.isButtonPressed(0, 0));
  EXPECT_FALSE(model.toggleButton(0, 0)); // no-op, no crash
  EXPECT_FALSE(model.data(model.index(0), PianoRollModel::ButtonsRole).isValid());
}

} // namespace firelight::gui
