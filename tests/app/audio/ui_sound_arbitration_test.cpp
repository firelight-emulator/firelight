// TODO: NEEDS REVIEW
#include "app/audio/ui_sound_player.hpp"

#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <gtest/gtest.h>

// Which sounds a user input is allowed to make: one, and only while the input is being handled
namespace firelight::audio {

class UiSoundArbitrationTest : public testing::Test {
protected:
  settings::SqliteSettingsRepository m_repo{":memory:"};
  settings::SettingsService m_service{m_repo};
  UiSoundPlayer m_player{m_service};

  // The arbitration answers before anything reaches the mixer, so no clip has to exist
  bool ask() { return m_player.request(0, 1.0, 1); }
};

// Nothing a person did is in flight, so a sound has nothing to attribute itself to
TEST_F(UiSoundArbitrationTest, ASoundOutsideAnInputIsNotHeard) { EXPECT_FALSE(ask()); }

// The first sound of an input is the one that stands for it
TEST_F(UiSoundArbitrationTest, TheFirstRequestInAnInputIsHeard) {
  UiSoundPlayer::GuiInputScope scope(&m_player);

  EXPECT_TRUE(ask());
}

// Everything the first sound's handler goes on to build is silent
TEST_F(UiSoundArbitrationTest, LaterRequestsInTheSameInputAreNotHeard) {
  UiSoundPlayer::GuiInputScope scope(&m_player);
  ASSERT_TRUE(ask());

  EXPECT_FALSE(ask());
  EXPECT_FALSE(ask());
}

// Each press is heard, which is what keeps a held direction sounding once per repeat
TEST_F(UiSoundArbitrationTest, EachInputIsHeardOnce) {
  for (auto i = 0; i < 3; ++i) {
    UiSoundPlayer::GuiInputScope scope(&m_player);

    EXPECT_TRUE(ask());
    EXPECT_FALSE(ask());
  }
}

// A caller that knows it answers the press takes the sound before any handler runs
TEST_F(UiSoundArbitrationTest, ClaimingTakesTheInputsSound) {
  UiSoundPlayer::GuiInputScope scope(&m_player);
  m_player.claim();

  EXPECT_FALSE(ask());
}

// A claim with no input behind it claims nothing, so it cannot silence the next press
TEST_F(UiSoundArbitrationTest, ClaimingOutsideAnInputDoesNotCarryOver) {
  m_player.claim();

  UiSoundPlayer::GuiInputScope scope(&m_player);

  EXPECT_TRUE(ask());
}

// A press handled inside another one keeps its own sound, and leaves the outer one spent
TEST_F(UiSoundArbitrationTest, ANestedInputArbitratesSeparately) {
  UiSoundPlayer::GuiInputScope outer(&m_player);
  ASSERT_TRUE(ask());

  {
    UiSoundPlayer::GuiInputScope inner(&m_player);
    EXPECT_TRUE(ask());
  }

  EXPECT_FALSE(ask());
}

// A sound that needs no input behind it is exempt both ways: it is never refused, and it leaves the
// input's own sound for whatever asks
TEST_F(UiSoundArbitrationTest, AnExemptSoundDoesNotSpendTheInput) {
  UiSoundPlayer::GuiInputScope scope(&m_player);
  m_player.play(0, 1.0, 1);

  EXPECT_TRUE(ask());
}

// Being claimed says nothing about an exempt sound, so both are heard in one input
TEST_F(UiSoundArbitrationTest, AnExemptSoundSurvivesAClaim) {
  UiSoundPlayer::GuiInputScope scope(&m_player);
  m_player.claim();
  m_player.play(0, 1.0, 1);

  EXPECT_FALSE(ask());
}

// An input that made no sound leaves the next one free to make its own
TEST_F(UiSoundArbitrationTest, AnInputThatSaidNothingDoesNotSpendTheNext) {
  { UiSoundPlayer::GuiInputScope quiet(&m_player); }

  UiSoundPlayer::GuiInputScope scope(&m_player);

  EXPECT_TRUE(ask());
}

} // namespace firelight::audio
