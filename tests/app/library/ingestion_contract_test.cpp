#include <firelight/library/accepted_extensions.hpp>
#include <firelight/library/content_loader.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/library_scanner2.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
#include <cstring>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <set>
#include <string>

// The contract: nothing may be accepted without a fixture saying what it identifies as. Adding
// an extension without a row here turns this suite red in the commit that added it, which is the
// only check in the codebase that fails on the absence of work rather than the presence of a bug
namespace firelight::library {

namespace {
using PS = platforms::PlatformService;

QByteArray filled(const int size, const char seed) {
  QByteArray bytes(size, Qt::Uninitialized);
  for (int i = 0; i < size; ++i) {
    bytes[i] = static_cast<char>((i * 31 + seed) & 0xFF);
  }
  return bytes;
}

QByteArray withMagicAt(const int size, const int offset, const char *magic) {
  auto bytes = filled(size, 3);
  bytes.replace(offset, static_cast<int>(std::strlen(magic)), magic);
  return bytes;
}

// What an extension is expected to do when the scanner meets it
enum class Expectation {
  // Becomes a library entry on a known platform
  Identifies,
  // Accepted and reported, because nothing can read the format yet
  KnownGap,
  // Identifiable in principle, but no fixture has been written for it. The list is asserted
  // below so this debt cannot grow without somebody editing it
  AwaitingFixture,
};

struct Recipe {
  Expectation expectation = Expectation::AwaitingFixture;
  int platformId = PS::PLATFORM_ID_UNKNOWN;
  QByteArray (*build)() = nullptr;
};

// Plain whole-file md5 platforms: the hasher has no content-dependent branch, so synthetic
// bytes exercise everything a real dump would
QByteArray plainRom() { return filled(65536, 7); }
QByteArray nesRom() { return withMagicAt(40960, 0, "NES\x1A"); }
QByteArray fdsRom() { return withMagicAt(40960, 0, "FDS\x1A"); }
QByteArray z64Rom() {
  auto bytes = filled(65536, 11);
  bytes[0] = static_cast<char>(0x80);
  bytes[1] = static_cast<char>(0x37);
  bytes[2] = static_cast<char>(0x12);
  bytes[3] = static_cast<char>(0x40);
  return bytes;
}
QByteArray genesisCartridge() { return withMagicAt(524288, 0x100, "SEGA MEGA DRIVE "); }

// The same cartridge scattered into a copier image: 512 leading bytes, then each 16KB block split
// into its odd bytes followed by its even ones
QByteArray copierImage() {
  const auto rom = genesisCartridge();
  QByteArray image(512, static_cast<char>(0xAA));
  constexpr int BLOCK = 0x4000;
  constexpr int HALF = BLOCK / 2;

  for (int start = 0; start < rom.size(); start += BLOCK) {
    QByteArray scattered(BLOCK, Qt::Uninitialized);

    for (int i = 0; i < HALF; ++i) {
      scattered[i] = rom[start + i * 2 + 1];
      scattered[HALF + i] = rom[start + i * 2];
    }

    image.append(scattered);
  }

  return image;
}

// The same cartridge from the fifth byte with every byte flipped
QByteArray encodedImage() {
  const auto rom = genesisCartridge();
  QByteArray image(4, '\0');

  for (int i = 0; i < rom.size(); ++i) {
    image.append(static_cast<char>(static_cast<unsigned char>(rom[i]) ^ 0x40));
  }

  image.append('\0');
  return image;
}
QByteArray segaCdImage() { return withMagicAt(65536, 0, "SEGADISCSYSTEM  "); }

const std::map<std::string, Recipe> &recipes() {
  static const std::map<std::string, Recipe> table{
      // Cartridges whose platform comes from the extension and whose hash is a plain md5
      {"gb", {Expectation::Identifies, PS::PLATFORM_ID_GAMEBOY, plainRom}},
      {"dmg", {Expectation::Identifies, PS::PLATFORM_ID_GAMEBOY, plainRom}},
      {"gbc", {Expectation::Identifies, PS::PLATFORM_ID_GAMEBOY_COLOR, plainRom}},
      {"gba", {Expectation::Identifies, PS::PLATFORM_ID_GAMEBOY_ADVANCE, plainRom}},
      {"sms", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_MASTER_SYSTEM, plainRom}},
      {"bms", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_MASTER_SYSTEM, plainRom}},
      {"md", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_GENESIS, genesisCartridge}},
      {"gen", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_GENESIS, genesisCartridge}},
      // Containers holding the same cartridge, normalised before hashing so all three agree
      {"smd", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_GENESIS, copierImage}},
      {"mdx", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_GENESIS, encodedImage}},
      {"gg", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_GAMEGEAR, plainRom}},
      {"sg", {Expectation::Identifies, PS::PLATFORM_ID_SG1000, plainRom}},
      {"min", {Expectation::Identifies, PS::PLATFORM_ID_POKEMON_MINI, plainRom}},
      {"vb", {Expectation::Identifies, PS::PLATFORM_ID_VIRTUAL_BOY, plainRom}},
      {"32x", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_32X, plainRom}},
      {"ws", {Expectation::Identifies, PS::PLATFORM_ID_WONDERSWAN, plainRom}},
      {"wsc", {Expectation::Identifies, PS::PLATFORM_ID_WONDERSWAN, plainRom}},
      {"pc2", {Expectation::Identifies, PS::PLATFORM_ID_WONDERSWAN, plainRom}},
      {"ngp", {Expectation::Identifies, PS::PLATFORM_ID_NEOGEO_POCKET, plainRom}},
      {"ngc", {Expectation::Identifies, PS::PLATFORM_ID_NEOGEO_POCKET, plainRom}},
      {"ngpc", {Expectation::Identifies, PS::PLATFORM_ID_NEOGEO_POCKET, plainRom}},
      {"npc", {Expectation::Identifies, PS::PLATFORM_ID_NEOGEO_POCKET, plainRom}},

      // Cartridges whose hasher reads a header
      {"nes", {Expectation::Identifies, PS::PLATFORM_ID_NES, nesRom}},
      {"unf", {Expectation::Identifies, PS::PLATFORM_ID_NES, plainRom}},
      {"unif", {Expectation::Identifies, PS::PLATFORM_ID_NES, plainRom}},
      {"fds", {Expectation::Identifies, PS::PLATFORM_ID_FAMICOM_DISK_SYSTEM, fdsRom}},
      {"sfc", {Expectation::Identifies, PS::PLATFORM_ID_SNES, plainRom}},
      {"smc", {Expectation::Identifies, PS::PLATFORM_ID_SNES, plainRom}},
      {"swc", {Expectation::Identifies, PS::PLATFORM_ID_SNES, plainRom}},
      {"fig", {Expectation::Identifies, PS::PLATFORM_ID_SNES, plainRom}},
      {"bs", {Expectation::Identifies, PS::PLATFORM_ID_SNES, plainRom}},
      {"st", {Expectation::Identifies, PS::PLATFORM_ID_SNES, plainRom}},
      {"n64", {Expectation::Identifies, PS::PLATFORM_ID_N64, z64Rom}},
      {"z64", {Expectation::Identifies, PS::PLATFORM_ID_N64, z64Rom}},
      {"v64", {Expectation::Identifies, PS::PLATFORM_ID_N64, z64Rom}},

      // Discs, identified by content
      {"bin", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_GENESIS, genesisCartridge}},
      {"iso", {Expectation::Identifies, PS::PLATFORM_ID_SEGA_CD, segaCdImage}},

      // Accepted so a real library reports them rather than losing them, with nothing able to
      // read one yet
      {"cso", {Expectation::KnownGap}},

      // TODO: fixtures for these need real structure (see the plan's step 10)
      {"nds", {Expectation::AwaitingFixture}},
      {"pce", {Expectation::AwaitingFixture}},
      {"sgx", {Expectation::AwaitingFixture}},
      {"cue", {Expectation::AwaitingFixture}},
      {"chd", {Expectation::AwaitingFixture}},
      {"m3u", {Expectation::AwaitingFixture}},
      {"gdi", {Expectation::AwaitingFixture}},
      {"pbp", {Expectation::AwaitingFixture}},
      {"img", {Expectation::AwaitingFixture}},
      {"mdf", {Expectation::AwaitingFixture}},
  };
  return table;
}
} // namespace

class IngestionContractTest : public testing::Test {
protected:
  QTemporaryDir m_tempDir;
  platforms::PlatformService m_platformService;
  std::unique_ptr<SqliteUserLibraryRepository> m_repo;
  std::unique_ptr<LibraryIngestService> m_ingest;

  void SetUp() override {
    ASSERT_TRUE(m_tempDir.isValid());
    m_repo = std::make_unique<SqliteUserLibraryRepository>(QString(":memory:"));
    m_ingest = std::make_unique<LibraryIngestService>(*m_repo);
  }

  void write(const QString &name, const QByteArray &bytes) const {
    QFile file(m_tempDir.filePath(name));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(bytes);
  }

  void scan() {
    ContentDirectory directory;
    directory.path = m_tempDir.path().toStdString();
    m_repo->create(directory);

    LibraryScanner2 scanner(*m_repo, m_platformService);
    QEventLoop loop;
    QObject::connect(&scanner, &LibraryScanner2::scanningChanged, &loop, [&] {
      if (!scanner.isScanning()) {
        loop.quit();
      }
    });
    QTimer::singleShot(20000, &loop, &QEventLoop::quit);
    scanner.scanAll();
    loop.exec();
  }
};

// The half that fails on the absence of work: add a string to DISC_EXTENSIONS or to any
// platform's file associations without saying what it does, and this goes red
TEST_F(IngestionContractTest, EveryAcceptedExtensionHasARecipe) {
  const auto accepted = acceptedExtensions(m_platformService);
  ASSERT_FALSE(accepted.empty());

  for (const auto &extension : accepted) {
    EXPECT_TRUE(recipes().count(extension) > 0) << extension << " is accepted but no recipe says what it identifies as";
  }

  for (const auto &[extension, recipe] : recipes()) {
    EXPECT_TRUE(accepted.count(extension) > 0) << extension << " has a recipe but nothing accepts it any more";
  }
}

// Pinning the debt: a fixture that has not been written yet is fine, a growing list of them is
// not. Anything added here has to be added deliberately
TEST_F(IngestionContractTest, TheFixturesStillOwedAreExactlyThese) {
  std::set<std::string> awaiting;

  for (const auto &[extension, recipe] : recipes()) {
    if (recipe.expectation == Expectation::AwaitingFixture) {
      awaiting.insert(extension);
    }
  }

  const std::set<std::string> expected{"chd", "cue", "gdi", "img", "m3u", "mdf", "nds", "pbp", "pce", "sgx"};
  EXPECT_EQ(awaiting, expected);
}

class AcceptedExtensionTest : public IngestionContractTest, public testing::WithParamInterface<std::string> {};

// The scanned hash has to be the one the loader recomputes at launch. That is the save key, so a
// disagreement writes somebody's progress under a name nothing will look for again
TEST_P(AcceptedExtensionTest, IdentifiesAndAgreesWithTheLoader) {
  const auto &recipe = recipes().at(GetParam());
  ASSERT_NE(recipe.build, nullptr);

  write(QString::fromStdString("Game." + GetParam()), recipe.build());
  scan();

  const auto entries = m_repo->getEntries(0, -1);
  ASSERT_EQ(entries.size(), 1u) << GetParam() << " did not become a library entry";
  EXPECT_EQ(entries.front().platformId, recipe.platformId) << GetParam() << " landed on the wrong platform";
  EXPECT_FALSE(entries.front().contentHash.empty());

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u);

  const ContentLoader loader;
  const auto loaded = loader.load(files.front());
  EXPECT_EQ(loaded.contentHash, files.front().m_contentHash)
      << GetParam() << ": the launch hash and the catalogued hash disagree";
}

INSTANTIATE_TEST_SUITE_P(Identifiable, AcceptedExtensionTest, testing::ValuesIn([] {
                           std::vector<std::string> identifiable;
                           for (const auto &[extension, recipe] : recipes()) {
                             if (recipe.expectation == Expectation::Identifies) {
                               identifiable.push_back(extension);
                             }
                           }
                           return identifiable;
                         }()),
                         [](const testing::TestParamInfo<std::string> &info) { return info.param; });

// A known gap asserts positively, so it fails both if the format starts working and if somebody
// quietly stops accepting it
TEST_F(IngestionContractTest, AKnownGapIsAcceptedAndReported) {
  write("Game.cso", filled(65536, 13));
  scan();

  EXPECT_TRUE(m_repo->getEntries(0, -1).empty()) << "cso identified, so its recipe is out of date";

  const auto drops = m_repo->getScanDrops();
  ASSERT_EQ(drops.size(), 1u) << "cso was not reported, so a real PSP library loses files silently";
  EXPECT_EQ(drops.front().extension, "cso");
  EXPECT_EQ(drops.front().outcome, IdentifyOutcome::NoIdentifier);
}

} // namespace firelight::library
