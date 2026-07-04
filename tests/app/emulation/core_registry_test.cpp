#include <libretro/core_registry.hpp>
#include <platform_metadata.hpp>

#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <gtest/gtest.h>

#include <algorithm>

namespace firelight {
namespace {
bool contains(const std::vector<PlatformCore> &cores, const std::string &id) {
  return std::any_of(cores.begin(), cores.end(),
                     [&](const auto &c) { return c.id == id; });
}
} // namespace

TEST(CoreRegistryTest, DefaultCoreMatchesLegacyMapping) {
  const auto &registry = CoreRegistry::instance();
  EXPECT_EQ(registry.defaultCoreForPlatform(PlatformMetadata::PLATFORM_ID_GAMEBOY),
            "gambatte_libretro");
  EXPECT_EQ(registry.defaultCoreForPlatform(
                PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE),
            "mgba_libretro");
  EXPECT_EQ(registry.defaultCoreForPlatform(PlatformMetadata::PLATFORM_ID_N64),
            "mupen64plus_next_libretro");
  EXPECT_EQ(
      registry.defaultCoreForPlatform(PlatformMetadata::PLATFORM_ID_SEGA_GENESIS),
      "genesis_plus_gx_libretro");
  EXPECT_TRUE(registry.defaultCoreForPlatform(999999).empty());
}

TEST(CoreRegistryTest, CoresForPlatformListsDefaultFirstPlusAlternates) {
  const auto &registry = CoreRegistry::instance();

  const auto gb =
      registry.coresForPlatform(PlatformMetadata::PLATFORM_ID_GAMEBOY);
  ASSERT_FALSE(gb.empty());
  EXPECT_EQ(gb.front().id, "gambatte_libretro");
  EXPECT_TRUE(gb.front().isDefault);
  // mGBA is offered as an alternate for the Game Boy.
  EXPECT_TRUE(contains(gb, "mgba_libretro"));

  const auto gba =
      registry.coresForPlatform(PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE);
  ASSERT_EQ(gba.size(), 1u);
  EXPECT_EQ(gba.front().id, "mgba_libretro");
  EXPECT_TRUE(gba.front().isDefault);
}

TEST(CoreRegistryTest, SupportsPlatformAndDllPath) {
  const auto &registry = CoreRegistry::instance();
  EXPECT_TRUE(registry.supportsPlatform(
      "mgba_libretro", PlatformMetadata::PLATFORM_ID_GAMEBOY));
  EXPECT_FALSE(registry.supportsPlatform(
      "gambatte_libretro", PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE));

  const auto path = registry.dllPathFor("mgba_libretro");
  EXPECT_NE(path.find("mgba_libretro"), std::string::npos);
  EXPECT_TRUE(registry.dllPathFor("does_not_exist").empty());
}

class CoreRegistryResolveTest : public testing::Test {
protected:
  settings::SqliteSettingsRepository m_repo{":memory:"};
  settings::SettingsService m_service{m_repo};
  const int m_gb = PlatformMetadata::PLATFORM_ID_GAMEBOY;
  const std::string m_hash = "hash1";

  void SetUp() override { settings::SettingsService::setInstance(&m_service); }
  void TearDown() override {
    settings::SettingsService::setInstance(nullptr);
    // The registry is a singleton; clear the session override so it can't leak
    // into other tests.
    CoreRegistry::instance().setSessionCoreOverride("");
  }
};

TEST_F(CoreRegistryResolveTest, ResolvesDefaultThenPlatformThenGame) {
  const auto &registry = CoreRegistry::instance();

  // No overrides -> default.
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash), "gambatte_libretro");

  // Valid platform override wins (mGBA supports Game Boy).
  m_service.setPlatformValue(m_gb, "core", "mgba_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash), "mgba_libretro");

  // Game override wins over platform.
  m_service.setGameValue(m_hash, "core", "gambatte_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash), "gambatte_libretro");
}

TEST_F(CoreRegistryResolveTest, IgnoresOverrideThatDoesNotSupportPlatform) {
  const auto &registry = CoreRegistry::instance();
  // snes9x can't run a Game Boy -> the override is rejected, falls back.
  m_service.setPlatformValue(m_gb, "core", "snes9x_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash), "gambatte_libretro");
}

TEST_F(CoreRegistryResolveTest, SessionOverrideBeatsStoredOverrides) {
  auto &registry = CoreRegistry::instance();
  // A stored game override would normally win...
  m_service.setGameValue(m_hash, "core", "gambatte_libretro");
  // ...but a CLI --core session override beats every stored tier (mGBA runs GB).
  registry.setSessionCoreOverride("mgba_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash), "mgba_libretro");
}

TEST_F(CoreRegistryResolveTest, SessionOverrideIgnoredWhenPlatformUnsupported) {
  auto &registry = CoreRegistry::instance();
  // snes9x can't run a Game Boy -> the session override is guarded out.
  registry.setSessionCoreOverride("snes9x_libretro");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash), "gambatte_libretro");

  // Clearing it restores normal resolution.
  registry.setSessionCoreOverride("");
  EXPECT_EQ(registry.resolveCoreName(m_gb, m_hash), "gambatte_libretro");
}

} // namespace firelight
