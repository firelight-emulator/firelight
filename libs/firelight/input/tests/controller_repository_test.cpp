#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/shortcut_registry.hpp>
#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QtCore/qnamespace.h>
#include <gtest/gtest.h>

namespace firelight::input {

class ControllerRepositoryTest : public testing::Test {
protected:
  void TearDown() override { ShortcutRegistry::instance().clear(); }

  // A stand-in for the shipped catalog, so these tests pin the seeding rule
  // rather than whatever data/shortcuts.json happens to say today
  static void loadPresets() {
    ShortcutRegistry::instance().loadFromString(R"({
      "defaultPreset": "firelight",
      "actions": [{"id": "fast_forward"}, {"id": "save_state"},
                  {"id": "open_quick_menu"}],
      "presets": [
        {"id": "firelight",
         "gamepad": {"fast_forward": ["Select+RightTrigger"],
                     "open_quick_menu": ["Select+R3", "Select+Start"]},
         "keyboard": {"save_state": ["Key_F1"]}}
      ]
    })");
  }

  platforms::PlatformService m_platformService;
  SqliteControllerRepository m_repo{":memory:", m_platformService};
};

TEST_F(ControllerRepositoryTest, CreateAndListProfiles) {
  const auto a = m_repo.createProfile("Alpha");
  const auto b = m_repo.createProfile("Beta");
  ASSERT_TRUE(a);
  ASSERT_TRUE(b);

  const auto all = m_repo.listProfiles();
  int found = 0;
  for (const auto &p : all) {
    if (p->getId() == a->getId() || p->getId() == b->getId()) {
      ++found;
    }
  }
  EXPECT_EQ(found, 2);
}

// The whole point of shipping presets: a brand-new profile has working hotkeys
// with no user action. Before seeding existed, every profile started blank
TEST_F(ControllerRepositoryTest, NewProfileIsSeededFromTheDefaultPreset) {
  loadPresets();

  const auto pad = m_repo.createProfile("SeededPad", DeviceType::Gamepad);
  ASSERT_TRUE(pad);
  EXPECT_EQ(pad->getPresetId(), "firelight");
  EXPECT_FALSE(pad->isKeyboardProfile());

  const auto &sources =
      pad->getShortcutMapping()->getBindings("fast_forward");
  ASSERT_EQ(sources.size(), 1u);
  EXPECT_EQ(sources[0].code, GamepadInput::RightTrigger);
  ASSERT_EQ(sources[0].modifiers.size(), 1u);
  EXPECT_EQ(sources[0].modifiers[0], GamepadInput::Select);

  // Every alternate is seeded, including ones a given pad can't produce: they
  // simply never match, and the next one takes over
  EXPECT_EQ(pad->getShortcutMapping()->getBindings("open_quick_menu").size(), 2u);

  // A gamepad profile gets the gamepad table only
  EXPECT_TRUE(pad->getShortcutMapping()->getBindings("save_state").empty());
}

TEST_F(ControllerRepositoryTest, KeyboardProfileIsSeededFromTheKeyboardTable) {
  loadPresets();

  const auto keyboard = m_repo.createProfile("SeededKeys", DeviceType::Keyboard);
  ASSERT_TRUE(keyboard);
  EXPECT_TRUE(keyboard->isKeyboardProfile());

  const auto &sources = keyboard->getShortcutMapping()->getBindings("save_state");
  ASSERT_EQ(sources.size(), 1u);
  EXPECT_EQ(sources[0].code, Qt::Key_F1);
  EXPECT_EQ(sources[0].type, SourceType::Key);

  // Gamepad bindings would be meaningless codes on a keyboard
  EXPECT_TRUE(keyboard->getShortcutMapping()->getBindings("fast_forward").empty());
}

// Reads a profile back with none of the in-memory state that created it
// Connections are keyed per thread, so a second repository here shares the same
// live database while keeping its own empty profile cache — which is exactly the
// "what does the next launch see" question these tests are asking
TEST_F(ControllerRepositoryTest, PersistedProfileStateIsReadBack) {
  loadPresets();
  const auto keyboard = m_repo.createProfile("ReloadKeys", DeviceType::Keyboard);
  const auto pad = m_repo.createProfile("ReloadPad", DeviceType::Gamepad);
  ASSERT_TRUE(keyboard);
  ASSERT_TRUE(pad);

  SqliteControllerRepository reopened{":memory:", m_platformService};

  // The kind was hardcoded to 0 on insert, so the keyboard's profile used to
  // come back as a gamepad profile and be re-flagged in memory every launch
  const auto loadedKeyboard = reopened.getProfile(keyboard->getId());
  ASSERT_TRUE(loadedKeyboard);
  EXPECT_TRUE(loadedKeyboard->isKeyboardProfile());
  EXPECT_EQ(loadedKeyboard->getPresetId(), "firelight");

  const auto loadedPad = reopened.getProfile(pad->getId());
  ASSERT_TRUE(loadedPad);
  EXPECT_FALSE(loadedPad->isKeyboardProfile());
  // Seeding writes through the mapping's sync callback, so it has to have
  // reached the shortcuts row rather than just the profile in memory
  EXPECT_EQ(loadedPad->getShortcutMapping()->getBindings("fast_forward").size(),
            1u);
}

TEST_F(ControllerRepositoryTest, ProfileWithNoPresetCatalogStartsUnbound) {
  // No catalog loaded: creation must still work, just with nothing bound
  const auto profile = m_repo.createProfile("Bare", DeviceType::Gamepad);
  ASSERT_TRUE(profile);
  EXPECT_TRUE(profile->getShortcutMapping()->getBindings("fast_forward").empty());
}

TEST_F(ControllerRepositoryTest, SetProfileAnalogSettingsPersistsToProfile) {
  const auto profile = m_repo.createProfile("Tuned");
  ASSERT_TRUE(profile);

  AnalogSettings settings;
  settings.leftStick.innerDeadzone = 0.12f;
  settings.rightStick.sensitivity = 1.4f;
  m_repo.setProfileAnalogSettings(profile->getId(), settings);

  EXPECT_EQ(profile->getDefaultAnalogSettings(), settings);
}

TEST_F(ControllerRepositoryTest, CloneCopiesAnalogSettings) {
  const auto src = m_repo.createProfile("Source");
  ASSERT_TRUE(src);

  AnalogSettings settings;
  settings.leftStick.innerDeadzone = 0.2f;
  m_repo.setProfileAnalogSettings(src->getId(), settings);

  const auto clone = m_repo.cloneProfile(src->getId(), "Clone");
  ASSERT_TRUE(clone);
  EXPECT_NE(clone->getId(), src->getId());
  EXPECT_EQ(clone->getDefaultAnalogSettings(), settings);
}

TEST_F(ControllerRepositoryTest, DeleteProfileRemovesIt) {
  const auto profile = m_repo.createProfile("Doomed");
  ASSERT_TRUE(profile);
  const auto id = profile->getId();

  EXPECT_TRUE(m_repo.deleteProfile(id));
  EXPECT_EQ(m_repo.getProfile(id), nullptr);
}

TEST_F(ControllerRepositoryTest, ExportImportRoundTrip) {
  const auto src = m_repo.createProfile("Exportable");
  ASSERT_TRUE(src);

  AnalogSettings settings;
  settings.leftStick.innerDeadzone = 0.17f;
  m_repo.setProfileAnalogSettings(src->getId(), settings);

  const auto platforms = m_platformService.listPlatforms();
  ASSERT_FALSE(platforms.empty());
  ASSERT_FALSE(platforms.front().controllerTypes.empty());
  const auto platformId = platforms.front().id;
  const auto controllerType = platforms.front().controllerTypes.front().id;

  auto mapping =
      src->getMappingForPlatformAndController(platformId, controllerType);
  ASSERT_TRUE(mapping);
  mapping->addMapping(GamepadInput::SouthFace, GamepadInput::EastFace);
  mapping->sync();

  const auto json = m_repo.exportProfile(src->getId());
  ASSERT_FALSE(json.empty());

  const auto imported = m_repo.importProfile(json);
  ASSERT_TRUE(imported);
  EXPECT_NE(imported->getId(), src->getId());
  EXPECT_EQ(imported->getDefaultAnalogSettings(), settings);

  auto importedMapping =
      imported->getMappingForPlatformAndController(platformId, controllerType);
  ASSERT_TRUE(importedMapping);
  const auto mapped = importedMapping->getMappedInput(GamepadInput::SouthFace);
  ASSERT_TRUE(mapped.has_value());
  EXPECT_EQ(mapped.value(), GamepadInput::EastFace);
}

TEST_F(ControllerRepositoryTest, DeviceInfoLookupByVendorProductVersion) {
  const auto profile = m_repo.createProfile("For Device");
  ASSERT_TRUE(profile);

  const DeviceIdentifier identifier{
      .deviceName = "Xbox Controller",
      .type = DeviceType::Gamepad,
      .vendorId = 1118,
      .productId = 654,
      .productVersion = 272,
  };
  m_repo.updateDeviceInfo(identifier, DeviceInfo{"My Xbox", profile->getId()});

  // An identical model (same vendor/product/version) resolves to the same info
  const auto found = m_repo.getDeviceInfo(DeviceIdentifier{
      .deviceName = "Xbox Controller #2",
      .type = DeviceType::Gamepad,
      .vendorId = 1118,
      .productId = 654,
      .productVersion = 272,
  });
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->profileId, profile->getId());
  EXPECT_EQ(found->displayName, "My Xbox");

  // A different product is a different device
  const DeviceIdentifier other{
      .vendorId = 1118, .productId = 999, .productVersion = 272};
  EXPECT_FALSE(m_repo.getDeviceInfo(other).has_value());
}

TEST_F(ControllerRepositoryTest, PlatformPreferredTypeRoundTrip) {
  constexpr int platformId = 7;
  EXPECT_FALSE(m_repo.getPlatformPreferredType(platformId).has_value());

  m_repo.setPlatformPreferredType(platformId, NINTENDO_NSO_N64);
  const auto preferred = m_repo.getPlatformPreferredType(platformId);
  ASSERT_TRUE(preferred.has_value());
  EXPECT_EQ(preferred.value(), static_cast<int>(NINTENDO_NSO_N64));

  // Overwrite is allowed
  m_repo.setPlatformPreferredType(platformId, SONY_DUALSENSE);
  EXPECT_EQ(m_repo.getPlatformPreferredType(platformId).value(),
            static_cast<int>(SONY_DUALSENSE));

  m_repo.clearPlatformPreferredType(platformId);
  EXPECT_FALSE(m_repo.getPlatformPreferredType(platformId).has_value());
}

TEST_F(ControllerRepositoryTest, GameProfileOverrideRoundTrip) {
  const auto profile = m_repo.createProfile("ForGame");
  ASSERT_TRUE(profile);

  EXPECT_FALSE(m_repo.getGameProfileOverride("abc123").has_value());

  m_repo.setGameProfileOverride("abc123", profile->getId());
  const auto override = m_repo.getGameProfileOverride("abc123");
  ASSERT_TRUE(override.has_value());
  EXPECT_EQ(override.value(), profile->getId());

  m_repo.clearGameProfileOverride("abc123");
  EXPECT_FALSE(m_repo.getGameProfileOverride("abc123").has_value());
}

TEST_F(ControllerRepositoryTest, RenameProfilePersists) {
  const auto profile = m_repo.createProfile("Old Name");
  ASSERT_TRUE(profile);

  EXPECT_TRUE(m_repo.renameProfile(profile->getId(), "New Name"));
  EXPECT_EQ(profile->getName(), "New Name");

  bool found = false;
  for (const auto &p : m_repo.listProfiles()) {
    if (p->getId() == profile->getId()) {
      found = true;
      EXPECT_EQ(p->getName(), "New Name");
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(ControllerRepositoryTest, GetProfileReturnsCachedInstance) {
  const auto created = m_repo.createProfile("Cached");
  ASSERT_TRUE(created);
  // The same shared_ptr should come back from the in-memory cache
  EXPECT_EQ(m_repo.getProfile(created->getId()), created);
}

TEST_F(ControllerRepositoryTest, CloneCopiesPerPlatformBindings) {
  const auto platforms = m_platformService.listPlatforms();
  ASSERT_FALSE(platforms.empty());
  ASSERT_FALSE(platforms.front().controllerTypes.empty());
  const auto platformId = platforms.front().id;
  const auto controllerType = platforms.front().controllerTypes.front().id;

  const auto src = m_repo.createProfile("BindingSrc");
  ASSERT_TRUE(src);
  auto mapping =
      src->getMappingForPlatformAndController(platformId, controllerType);
  ASSERT_TRUE(mapping);
  mapping->addMapping(GamepadInput::SouthFace, GamepadInput::EastFace);
  mapping->sync();

  const auto clone = m_repo.cloneProfile(src->getId(), "BindingClone");
  ASSERT_TRUE(clone);
  auto clonedMapping =
      clone->getMappingForPlatformAndController(platformId, controllerType);
  ASSERT_TRUE(clonedMapping);
  const auto mapped = clonedMapping->getMappedInput(GamepadInput::SouthFace);
  ASSERT_TRUE(mapped.has_value());
  EXPECT_EQ(mapped.value(), GamepadInput::EastFace);
}

} // namespace firelight::input
