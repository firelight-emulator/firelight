#include <firelight/platforms/platform_service.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <algorithm>
#include <gtest/gtest.h>
#include <libretro/core_registry.hpp>

namespace firelight {
namespace {
bool contains(const std::vector<PlatformCore> &cores, const std::string &id) {
  return std::any_of(cores.begin(), cores.end(), [&](const auto &c) { return c.id == id; });
}
} // namespace

TEST(CoreRegistryTest, DefaultCoreMatchesLegacyMapping) {
  const auto &registry = CoreRegistry::instance();
  EXPECT_EQ(registry.defaultCoreForPlatform(firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY),
            "gambatte_libretro");
  EXPECT_EQ(registry.defaultCoreForPlatform(firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE),
            "mgba_libretro");
  EXPECT_EQ(registry.defaultCoreForPlatform(firelight::platforms::PlatformService::PLATFORM_ID_N64),
            "mupen64plus_next_libretro");
  EXPECT_EQ(registry.defaultCoreForPlatform(firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS),
            "genesis_plus_gx_libretro");
  EXPECT_TRUE(registry.defaultCoreForPlatform(999999).empty());
}

TEST(CoreRegistryTest, CoresForPlatformListsDefaultFirstPlusAlternates) {
  const auto &registry = CoreRegistry::instance();

  const auto gb = registry.coresForPlatform(firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY);
  ASSERT_FALSE(gb.empty());
  EXPECT_EQ(gb.front().id, "gambatte_libretro");
  EXPECT_TRUE(gb.front().isDefault);
  // mGBA is offered as an alternate for the Game Boy
  EXPECT_TRUE(contains(gb, "mgba_libretro"));

  const auto gba = registry.coresForPlatform(firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE);
  ASSERT_EQ(gba.size(), 1u);
  EXPECT_EQ(gba.front().id, "mgba_libretro");
  EXPECT_TRUE(gba.front().isDefault);
}

TEST(CoreRegistryTest, SupportsPlatformAndDllPath) {
  const auto &registry = CoreRegistry::instance();
  EXPECT_TRUE(registry.supportsPlatform("mgba_libretro", firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY));
  EXPECT_FALSE(registry.supportsPlatform("gambatte_libretro",
                                         firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE));

  const auto path = registry.dllPathFor("mgba_libretro");
  EXPECT_NE(path.find("mgba_libretro"), std::string::npos);
  EXPECT_TRUE(registry.dllPathFor("does_not_exist").empty());
}

TEST(CoreRegistryTest, DeviceCatalogClassifiesVariants) {
  const auto &registry = CoreRegistry::instance();

  // Genesis Plus GX exposes joypad variants + a mouse + light guns
  const auto &gpgx = registry.deviceCatalogForCore("genesis_plus_gx_libretro");
  ASSERT_FALSE(gpgx.empty());
  bool has6Button = false, hasLightgun = false;
  for (const auto &v : gpgx) {
    if (v.friendlyName == "Control Pad (6-button)") {
      has6Button = true;
      EXPECT_EQ(v.deviceClass, input::GamepadInputClass::Joypad);
      EXPECT_TRUE(v.isDefault);
    }
    hasLightgun = hasLightgun || v.deviceClass == input::GamepadInputClass::Lightgun;
  }
  EXPECT_TRUE(has6Button);
  EXPECT_TRUE(hasLightgun);

  // FCEUmm's Zapper is modeled as a light gun with a companion core option so
  // the core queries the light-gun protocol
  const auto &fceumm = registry.deviceCatalogForCore("fceumm_libretro");
  ASSERT_EQ(fceumm.size(), 1u);
  EXPECT_EQ(fceumm[0].friendlyName, "Zapper");
  EXPECT_EQ(fceumm[0].deviceClass, input::GamepadInputClass::Lightgun);
  ASSERT_EQ(fceumm[0].companionOptions.size(), 1u);
  EXPECT_EQ(fceumm[0].companionOptions[0].first, "fceumm_zapper_mode");
  EXPECT_EQ(fceumm[0].companionOptions[0].second, "clightgun");

  // Uncatalogued core -> empty (joypad only)
  EXPECT_TRUE(registry.deviceCatalogForCore("gambatte_libretro").empty());
}

TEST(CoreRegistryTest, AvailableVariantsCrossReferencesAdvertisement) {
  const auto &registry = CoreRegistry::instance();

  // snes9x catalog = Mouse(2), Super Scope(260), Justifier(516). The core here
  // advertises only the pad, mouse, and Super Scope -> Justifier is filtered
  // out, and a standard joypad is synthesized as the default (first)
  const auto variants = registry.availableControllerVariants("snes9x_libretro", {1u, 2u, 260u});
  ASSERT_EQ(variants.size(), 3u);
  EXPECT_TRUE(variants.front().isDefault);
  EXPECT_EQ(variants.front().deviceClass, input::GamepadInputClass::Joypad);
  bool hasMouse = false, hasScope = false, hasJustifier = false;
  for (const auto &v : variants) {
    hasMouse = hasMouse || v.coreDeviceId == 2u;
    hasScope = hasScope || v.coreDeviceId == 260u;
    hasJustifier = hasJustifier || v.coreDeviceId == 516u;
  }
  EXPECT_TRUE(hasMouse);
  EXPECT_TRUE(hasScope);
  EXPECT_FALSE(hasJustifier);

  // An uncatalogued core still yields the synthesized standard controller
  const auto gb = registry.availableControllerVariants("gambatte_libretro", {1u});
  ASSERT_EQ(gb.size(), 1u);
  EXPECT_TRUE(gb.front().isDefault);
  EXPECT_EQ(gb.front().deviceClass, input::GamepadInputClass::Joypad);
}

class CoreRegistryResolveTest : public testing::Test {
protected:
  settings::SqliteSettingsRepository m_repo{":memory:"};
  settings::SettingsService m_service{m_repo};
  const int m_gb = firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY;
  const std::string m_hash = "hash1";

  void TearDown() override {
    // The registry is a singleton; clear the session override so it can't leak
    // into other tests
    CoreRegistry::instance().setSessionCoreOverride("");
  }
};

TEST_F(CoreRegistryResolveTest, ResolvesDefaultThenPlatformThenGame) {
  const auto &registry = CoreRegistry::instance();

  // No overrides -> default
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash, &m_service), "gambatte_libretro");

  // Valid platform override wins (mGBA supports Game Boy)
  m_service.setPlatformValue(m_gb, "core", "mgba_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash, &m_service), "mgba_libretro");

  // Game override wins over platform
  m_service.setGameValue(m_hash, "core", "gambatte_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash, &m_service), "gambatte_libretro");
}

TEST_F(CoreRegistryResolveTest, IgnoresOverrideThatDoesNotSupportPlatform) {
  const auto &registry = CoreRegistry::instance();
  // snes9x can't run a Game Boy -> the override is rejected, falls back
  m_service.setPlatformValue(m_gb, "core", "snes9x_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash, &m_service), "gambatte_libretro");
}

TEST_F(CoreRegistryResolveTest, SessionOverrideBeatsStoredOverrides) {
  auto &registry = CoreRegistry::instance();
  // A stored game override would normally win...
  m_service.setGameValue(m_hash, "core", "gambatte_libretro");
  // ...but a CLI --core session override beats every stored tier (mGBA runs GB)
  registry.setSessionCoreOverride("mgba_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash, &m_service), "mgba_libretro");
}

TEST_F(CoreRegistryResolveTest, SessionOverrideIgnoredWhenPlatformUnsupported) {
  auto &registry = CoreRegistry::instance();
  // snes9x can't run a Game Boy -> the session override is guarded out
  registry.setSessionCoreOverride("snes9x_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash, &m_service), "gambatte_libretro");

  // Clearing it restores normal resolution
  registry.setSessionCoreOverride("");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash, &m_service), "gambatte_libretro");
}

TEST(CoreRegistryTest, AvailabilityCoversEveryRegisteredCore) {
  const auto &registry = CoreRegistry::instance();
  const auto availability = registry.checkAvailability();

  EXPECT_EQ(availability.size(), registry.cores().size());
  for (const auto &entry : availability) {
    EXPECT_FALSE(entry.coreId.empty());
    EXPECT_FALSE(entry.expectedPath.empty());
  }
}

TEST(CoreRegistryTest, AvailabilityPathMatchesDllPathFor) {
  const auto &registry = CoreRegistry::instance();
  for (const auto &entry : registry.checkAvailability()) {
    EXPECT_EQ(entry.expectedPath, registry.dllPathFor(entry.coreId));
  }
}

// A core with no supported platforms can never come out of resolveCoreName, so
// shipping it is wasted bytes
TEST(CoreRegistryTest, AvailabilityMarksCoresWithNoPlatformsUnreachable) {
  const auto &registry = CoreRegistry::instance();
  const auto availability = registry.checkAvailability();

  const auto geolith = std::find_if(availability.begin(), availability.end(),
                                    [](const auto &a) { return a.coreId == "geolith_libretro"; });
  ASSERT_NE(geolith, availability.end());
  EXPECT_FALSE(geolith->reachable);

  const auto gba =
      std::find_if(availability.begin(), availability.end(), [](const auto &a) { return a.coreId == "mgba_libretro"; });
  ASSERT_NE(gba, availability.end());
  EXPECT_TRUE(gba->reachable);
}

TEST(CoreRegistryTest, AvailabilityReportsWhichPlatformsDefaultToACore) {
  const auto &registry = CoreRegistry::instance();
  const auto availability = registry.checkAvailability();

  const auto gba =
      std::find_if(availability.begin(), availability.end(), [](const auto &a) { return a.coreId == "mgba_libretro"; });
  ASSERT_NE(gba, availability.end());
  EXPECT_NE(std::find(gba->defaultForPlatforms.begin(), gba->defaultForPlatforms.end(),
                      firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE),
            gba->defaultForPlatforms.end());
}

// Every platform default must name a core that is actually installed, or that
// platform cannot launch at all
TEST(CoreRegistryTest, EveryPlatformDefaultCoreIsInstalled) {
  const auto &registry = CoreRegistry::instance();
  for (const auto &entry : registry.checkAvailability()) {
    if (entry.defaultForPlatforms.empty()) {
      continue;
    }
    EXPECT_TRUE(entry.present) << entry.coreId << " is the default for " << entry.defaultForPlatforms.size()
                               << " platform(s) but is not installed at " << entry.expectedPath;
  }
}

} // namespace firelight
