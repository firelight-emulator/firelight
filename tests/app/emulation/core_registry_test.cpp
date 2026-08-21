#include <firelight/platforms/platform_service.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
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

// Whether a game can start is a platform fact, not an entry one: the library shows a badge
// rather than hiding the row, so this has to agree with what checkAvailability reports
TEST(CoreRegistryTest, PlatformIsPlayableExactlyWhenItsCoreIsInstalled) {
  const auto &registry = CoreRegistry::instance();
  auto checkedAny = false;

  for (const auto &entry : registry.checkAvailability()) {
    for (const auto platformId : entry.defaultForPlatforms) {
      EXPECT_EQ(registry.hasInstalledCore(platformId), entry.present)
          << "platform " << platformId << " defaults to " << entry.coreId;
      checkedAny = true;
    }
  }

  EXPECT_TRUE(checkedAny) << "no platform declares a default core, so nothing was actually checked";
}

// A platform nothing can run is not playable, rather than being reported as fine because no
// core objected
TEST(CoreRegistryTest, AnUnknownPlatformIsNotPlayable) {
  EXPECT_FALSE(CoreRegistry::instance().hasInstalledCore(999999));
}

// A platform declares the formats its dumps come in; a core declares what it will open. Nothing
// used to compare the two, so a platform could accept a file that the core running it refuses —
// the entry appears, and the launch fails with nothing to explain it
TEST(CoreRegistryTest, EveryDeclaredExtensionIsOpenedByACoreThatRunsThePlatform) {
  const auto &registry = CoreRegistry::instance();
  const platforms::PlatformService platformService;
  auto checkedPlatforms = 0;

  for (const auto &platform : platformService.listPlatforms()) {
    const auto platformId = static_cast<int>(platform.id);
    const auto cores = registry.coresForPlatform(platformId);

    // A platform with no core has nothing to disagree with; those badge PlatformNotSupported,
    // which is the designed answer rather than a mismatch
    if (cores.empty() || platform.fileAssociations.empty()) {
      continue;
    }

    ++checkedPlatforms;

    for (const auto &extension : platform.fileAssociations) {
      const auto opened = std::any_of(cores.begin(), cores.end(), [&](const PlatformCore &core) {
        const auto &extensions = registry.fileExtensionsFor(core.id);
        return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
      });

      EXPECT_TRUE(opened) << platform.name << " accepts ." << extension << " but no core that runs it opens that";
    }
  }

  EXPECT_GT(checkedPlatforms, 0) << "no platform pairs a core with declared extensions, so nothing was checked";
}

TEST(CoreRegistryTest, TheDiscSystemsResolveToTheCoresThatCanReadThem) {
  const auto &registry = CoreRegistry::instance();

  EXPECT_EQ(registry.defaultCoreForPlatform(platforms::PlatformService::PLATFORM_ID_SEGA_CD),
            "genesis_plus_gx_libretro");
  EXPECT_EQ(registry.defaultCoreForPlatform(platforms::PlatformService::PLATFORM_ID_PC_ENGINE_CD),
            "mednafen_supergrafx_libretro");
}

//****************
// bios
//****************

// The system directory is process-wide, so every case here puts back what it found
class CoreRegistryBiosTest : public testing::Test {
protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "fl_bios_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);
  }

  void TearDown() override {
    CoreRegistry::instance().setSystemDirectory("");
    std::filesystem::remove_all(m_directory);
  }

  void placeFile(const std::string &filename) const { std::ofstream(m_directory / filename).put('\0'); }

  void useDirectory() const { CoreRegistry::instance().setSystemDirectory(m_directory.string()); }

  std::filesystem::path m_directory;
};

TEST_F(CoreRegistryBiosTest, APlatformThatBootsFromNothingNeedsNothing) {
  useDirectory();

  EXPECT_TRUE(CoreRegistry::instance().biosStatusForPlatform(platforms::PlatformService::PLATFORM_ID_GAMEBOY).empty());
  EXPECT_TRUE(CoreRegistry::instance().hasRequiredBios(platforms::PlatformService::PLATFORM_ID_GAMEBOY));
}

// An extra a core can use is not a reason to tell somebody their game is broken
TEST_F(CoreRegistryBiosTest, AnOptionalFileNeverHoldsAPlatformBack) {
  auto &registry = CoreRegistry::instance();
  const auto genesis = platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS;
  useDirectory();

  const auto statuses = registry.biosStatusForPlatform(genesis);

  ASSERT_FALSE(statuses.empty()) << "the Genesis lock-on and bootrom files should be modelled";
  EXPECT_TRUE(std::all_of(statuses.begin(), statuses.end(),
                          [](const auto &s) { return s.requirement.necessity == BiosNecessity::Optional; }));

  // Nothing at all in the directory, and the platform is still fine
  EXPECT_TRUE(registry.hasRequiredBios(genesis));
  EXPECT_FALSE(registry.coreProblemForPlatform(genesis, nullptr).has_value());
}

// The file alone does nothing, so anything offering to help has to say what else is needed
TEST_F(CoreRegistryBiosTest, TheLockOnFilesCarryTheCoreOptionThatGatesThem) {
  useDirectory();

  const auto statuses =
      CoreRegistry::instance().biosStatusForPlatform(platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS);

  const auto gameGenie = std::find_if(statuses.begin(), statuses.end(), [](const auto &status) {
    return status.requirement.filenames.front() == "ggenie.bin";
  });

  ASSERT_NE(gameGenie, statuses.end());
  // The key and value the core actually reads, not the friendly category the docs group them by
  EXPECT_EQ(gameGenie->requirement.coreOption, "genesis_plus_gx_lock_on");
  EXPECT_EQ(gameGenie->requirement.coreOptionValue, "game genie");
}

// The Game Boy platforms resolve to Gambatte by default and to mGBA when overridden, and the two
// want different files. Keying on the platform alone would answer for whichever core was written
// down first
TEST_F(CoreRegistryBiosTest, TheSamePlatformUnderADifferentCoreWantsDifferentFiles) {
  const auto &registry = CoreRegistry::instance();
  const auto gba = platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE;
  useDirectory();

  const auto underMgba = registry.biosStatusFor("mgba_libretro", gba);

  ASSERT_EQ(underMgba.size(), 1u);
  EXPECT_EQ(underMgba[0].requirement.filenames.front(), "gba_bios.bin");
  EXPECT_EQ(underMgba[0].requirement.necessity, BiosNecessity::Optional);

  EXPECT_TRUE(registry.biosStatusFor("gambatte_libretro", gba).empty());
}

// It had a BIOS entry and no core, so resolveCoreName came back empty and the platform reported
// as unsupported before anything asked about the file
TEST_F(CoreRegistryBiosTest, TheDiskSystemResolvesToACoreSoItsBiosIsReachedAtAll) {
  const auto &registry = CoreRegistry::instance();
  const auto fds = platforms::PlatformService::PLATFORM_ID_FAMICOM_DISK_SYSTEM;

  ASSERT_EQ(registry.defaultCoreForPlatform(fds), "fceumm_libretro");
  ASSERT_TRUE(registry.hasInstalledCore(fds));

  useDirectory();
  const auto problem = registry.coreProblemForPlatform(fds, nullptr);

  ASSERT_TRUE(problem.has_value());
  EXPECT_EQ(*problem, library::EntryProblem::BiosMissing) << "not PlatformNotSupported";

  placeFile("disksys.rom");

  EXPECT_FALSE(registry.coreProblemForPlatform(fds, nullptr).has_value());
}

// Owning the US BIOS boots US games and nothing else, so the ones still absent have to survive
// into the result rather than being collapsed into a yes
TEST_F(CoreRegistryBiosTest, OneRegionsBiosIsReportedAsTheGapItIs) {
  auto &registry = CoreRegistry::instance();
  const auto segaCd = platforms::PlatformService::PLATFORM_ID_SEGA_CD;
  useDirectory();
  placeFile("bios_CD_U.bin");

  const auto statuses = registry.biosStatusForPlatform(segaCd);

  ASSERT_EQ(statuses.size(), 1u);
  EXPECT_EQ(statuses[0].requirement.necessity, BiosNecessity::PerRegion);
  EXPECT_TRUE(statuses[0].isSatisfied);
  EXPECT_EQ(statuses[0].presentFiles, std::vector<std::string>{"bios_CD_U.bin"});
  EXPECT_EQ(statuses[0].missingFiles, (std::vector<std::string>{"bios_CD_E.bin", "bios_CD_J.bin"}));
}

// Filing every file as absent would read as a directory that was searched and came up empty
TEST_F(CoreRegistryBiosTest, AnUnsetDirectoryReportsNeitherPresentNorMissing) {
  CoreRegistry::instance().setSystemDirectory("");

  const auto statuses = CoreRegistry::instance().biosStatusForPlatform(platforms::PlatformService::PLATFORM_ID_SEGA_CD);

  ASSERT_EQ(statuses.size(), 1u);
  EXPECT_TRUE(statuses[0].presentFiles.empty());
  EXPECT_TRUE(statuses[0].missingFiles.empty());
  EXPECT_TRUE(statuses[0].isSatisfied);
}

// Not knowing where to look is not the same as having looked and found nothing
TEST_F(CoreRegistryBiosTest, AnUnsetSystemDirectoryReportsNothingMissing) {
  CoreRegistry::instance().setSystemDirectory("");

  EXPECT_TRUE(CoreRegistry::instance().hasRequiredBios(platforms::PlatformService::PLATFORM_ID_SEGA_CD));
}

TEST_F(CoreRegistryBiosTest, AnyOneRegionOfTheBiosSatisfiesIt) {
  auto &registry = CoreRegistry::instance();
  useDirectory();

  ASSERT_FALSE(registry.hasRequiredBios(platforms::PlatformService::PLATFORM_ID_SEGA_CD));

  placeFile("bios_CD_E.bin");

  EXPECT_TRUE(registry.hasRequiredBios(platforms::PlatformService::PLATFORM_ID_SEGA_CD));
}

TEST_F(CoreRegistryBiosTest, ADifferentPlatformsBiosDoesNotCount) {
  auto &registry = CoreRegistry::instance();
  useDirectory();
  placeFile("syscard3.pce");

  EXPECT_TRUE(registry.hasRequiredBios(platforms::PlatformService::PLATFORM_ID_PC_ENGINE_CD));
  EXPECT_FALSE(registry.hasRequiredBios(platforms::PlatformService::PLATFORM_ID_SEGA_CD));
}

// The whole chain: an installed core with nothing to boot from is a problem of its own rather
// than passing as fine
TEST_F(CoreRegistryBiosTest, AnInstalledCoreWithNoBiosIsReportedAsSuch) {
  auto &registry = CoreRegistry::instance();
  const auto segaCd = platforms::PlatformService::PLATFORM_ID_SEGA_CD;

  ASSERT_TRUE(registry.hasInstalledCore(segaCd)) << "this case only means anything while the core is installed";

  useDirectory();
  const auto missing = registry.coreProblemForPlatform(segaCd, nullptr);

  ASSERT_TRUE(missing.has_value());
  EXPECT_EQ(*missing, library::EntryProblem::BiosMissing);

  placeFile("bios_CD_U.bin");

  EXPECT_FALSE(registry.coreProblemForPlatform(segaCd, nullptr).has_value());
}

} // namespace firelight
