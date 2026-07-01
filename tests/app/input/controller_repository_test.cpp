#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <gtest/gtest.h>

namespace firelight::input {

class ControllerRepositoryTest : public testing::Test {
protected:
  SqliteControllerRepository m_repo{":memory:"};
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

  const auto platforms =
      platforms::PlatformService::getInstance().listPlatforms();
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

  // An identical model (same vendor/product/version) resolves to the same info.
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

  // A different product is a different device.
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

  // Overwrite is allowed.
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

} // namespace firelight::input
