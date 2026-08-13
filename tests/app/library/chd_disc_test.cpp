#include <firelight/library/disc_inspector.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/library_scanner2.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>

// CHD is how most people actually store discs, and Firelight reads it through a cdreader shim of
// its own rather than through anything rcheevos ships. These run against a real committed image,
// because a synthetic one would only prove our reader agrees with our own writer
namespace firelight::library {

namespace {
// The hash the shipped image resolves to. Pinned because everything a user owns hangs off it:
// a reader change that moved this would silently orphan their saves
constexpr auto TEST_DISC_HASH = "ff53a43a7b6fbae78b816a5967465c10";
constexpr auto TEST_DISC_PATH = "test_resources/testdisc.chd";
constexpr auto TEST_DISC_CUE_PATH = "test_resources/testdisc.cue";
constexpr auto TEST_DISC_BIN_PATH = "test_resources/testdisc.bin";

// A MODE2/2352 sector carries 2048 bytes of user data after its sync, header and
// subheader. Taking just those is what a plain .iso of the same disc holds
constexpr size_t RAW_SECTOR_SIZE = 2352;
constexpr size_t USER_DATA_OFFSET = 24;
constexpr size_t USER_DATA_SIZE = 2048;

bool testDiscExists() { return std::ifstream(TEST_DISC_PATH, std::ios::binary).good(); }
} // namespace

class ChdDiscTest : public testing::Test {
protected:
  platforms::PlatformService m_platformService;

  void SetUp() override {
    if (!testDiscExists()) {
      GTEST_SKIP() << TEST_DISC_PATH << " missing from the build directory";
    }
  }
};

// A console with no platform is not a failure to read the disc. The format was understood and the
// dump is fine, so what it turned out to be is kept rather than being flattened into "could not
// catalogue this" — which is the difference between telling somebody they own GameCube discs and
// telling them four files went missing
TEST_F(ChdDiscTest, ADiscForAnUnmodeledConsoleKeepsWhatItWasIdentifiedAs) {
  // Answers as though Firelight modeled nothing, so a disc that really does identify still has
  // nowhere to land. Cheaper and more exact than committing an image for a console we cannot run
  class NoPlatforms final : public platforms::IPlatformService {
  public:
    [[nodiscard]] std::optional<platforms::Platform> getPlatform(unsigned) const override { return {}; }

    [[nodiscard]] std::vector<platforms::Platform> listPlatforms() const override { return {}; }

    [[nodiscard]] int platformIdForExtension(const std::string &) const override {
      return platforms::PlatformService::PLATFORM_ID_UNKNOWN;
    }

    [[nodiscard]] int platformIdForRcConsole(int) const override {
      return platforms::PlatformService::PLATFORM_ID_UNKNOWN;
    }
  };

  NoPlatforms service;
  const DiscInspector inspector(service);
  std::vector<IdentifiedDiscMember> members;

  const auto identity = inspector.inspectFile(TEST_DISC_PATH, members);

  EXPECT_FALSE(identity.isIdentified());
  EXPECT_EQ(identity.outcome, IdentifyOutcome::PlatformNotSupported);
  EXPECT_FALSE(identity.identifiedAs.empty()) << "the console it identified as was thrown away";
}

// The reader shim end to end: open the CHD, walk its track metadata, read sectors and identify
TEST_F(ChdDiscTest, AChdIdentifiesAsItsRealPlatformWithAStableHash) {
  const DiscInspector inspector(m_platformService);
  std::vector<IdentifiedDiscMember> members;

  const auto identity = inspector.inspectFile(TEST_DISC_PATH, members);

  ASSERT_TRUE(identity.isIdentified()) << "the CHD reader could not identify a real disc image";
  EXPECT_EQ(identity.platformId, platforms::PlatformService::PLATFORM_ID_PS1);
  EXPECT_EQ(identity.contentHash, TEST_DISC_HASH);
}

// Identifying twice must give the same answer, since the second time is a rescan
TEST_F(ChdDiscTest, IdentifyingTheSameChdTwiceGivesTheSameHash) {
  const DiscInspector inspector(m_platformService);
  std::vector<IdentifiedDiscMember> first;
  std::vector<IdentifiedDiscMember> second;

  EXPECT_EQ(inspector.inspectFile(TEST_DISC_PATH, first).contentHash,
            inspector.inspectFile(TEST_DISC_PATH, second).contentHash);
}

// The whole disc-set design rests on this: a playlist carries its first disc's identity, so it
// attaches to the entry that already exists instead of making a second one
TEST_F(ChdDiscTest, APlaylistNamingAChdHashesToThatChd) {
  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());

  const auto discPath = directory.filePath("Game (Disc 1).chd");
  ASSERT_TRUE(QFile::copy(TEST_DISC_PATH, discPath));

  const auto playlistPath = directory.filePath("Game.m3u");
  {
    QFile playlist(playlistPath);
    ASSERT_TRUE(playlist.open(QIODevice::WriteOnly));
    playlist.write("Game (Disc 1).chd\n");
  }

  const DiscInspector inspector(m_platformService);
  std::vector<IdentifiedDiscMember> members;
  const auto viaPlaylist = inspector.inspectFile(playlistPath.toStdString(), members);

  ASSERT_TRUE(viaPlaylist.isIdentified());
  EXPECT_EQ(viaPlaylist.contentHash, TEST_DISC_HASH);
}

// And the shape a user sees: a .chd in a games folder is a game
TEST_F(ChdDiscTest, AChdOnDiskBecomesALibraryEntry) {
  QTemporaryDir games;
  ASSERT_TRUE(games.isValid());
  ASSERT_TRUE(QFile::copy(TEST_DISC_PATH, games.filePath("Final Fantasy VII (USA) (Disc 1).chd")));

  SqliteUserLibraryRepository repository{QString(":memory:")};
  const LibraryIngestService ingest(repository);

  ContentDirectory directory;
  directory.path = games.path().toStdString();
  ASSERT_TRUE(repository.create(directory));

  LibraryScanner2 scanner(repository, m_platformService);
  QEventLoop loop;
  QObject::connect(&scanner, &LibraryScanner2::scanningChanged, &loop, [&] {
    if (!scanner.isScanning()) {
      loop.quit();
    }
  });
  QTimer::singleShot(60000, &loop, &QEventLoop::quit);
  scanner.scanAll();
  loop.exec();

  const auto entries = repository.getEntries(0, -1);
  ASSERT_EQ(entries.size(), 1u);
  EXPECT_EQ(entries.front().contentHash, TEST_DISC_HASH);
  EXPECT_EQ(entries.front().platformId, platforms::PlatformService::PLATFORM_ID_PS1);
  EXPECT_FALSE(entries.front().hidden);
  // The disc tag on a real filename reaches the content file
  ASSERT_EQ(repository.getContentFiles().size(), 1u);
  EXPECT_EQ(repository.getContentFiles().front().m_discNumber, 1);
}

// The invariance the whole design rests on: one disc is one game however it was dumped. A cue
// that hashed differently from its chd would be a second entry with its own saves
TEST_F(ChdDiscTest, ACueAndAChdOfOneDiscHashTheSame) {
  if (!std::ifstream(TEST_DISC_CUE_PATH, std::ios::binary).good() ||
      !std::ifstream(TEST_DISC_BIN_PATH, std::ios::binary).good()) {
    GTEST_SKIP() << "testdisc.cue/.bin missing from the build directory";
  }

  const DiscInspector inspector(m_platformService);
  std::vector<IdentifiedDiscMember> fromCue;
  std::vector<IdentifiedDiscMember> fromChd;

  const auto cueIdentity = inspector.inspectFile(TEST_DISC_CUE_PATH, fromCue);
  const auto chdIdentity = inspector.inspectFile(TEST_DISC_PATH, fromChd);

  ASSERT_TRUE(cueIdentity.isIdentified()) << "the cue+bin pair did not identify";
  EXPECT_EQ(cueIdentity.platformId, chdIdentity.platformId);
  EXPECT_EQ(cueIdentity.contentHash, chdIdentity.contentHash);
  EXPECT_EQ(cueIdentity.contentHash, TEST_DISC_HASH);

  // The sheet names the track beside it, which is what the scanner suppresses
  ASSERT_FALSE(fromCue.empty());
  EXPECT_EQ(fromCue.front().role, "track");
}

// rcheevos offers a .iso to PS2, PSP, 3DO, Sega CD, GameCube and Wii and to no PlayStation, so
// without our own candidate a PS1 disc dumped as a plain .iso matches nothing and vanishes
TEST_F(ChdDiscTest, APlayStationDiscDumpedAsAnIsoStillIdentifies) {
  std::ifstream bin(TEST_DISC_BIN_PATH, std::ios::binary);

  if (!bin.good()) {
    GTEST_SKIP() << "testdisc.bin missing from the build directory";
  }

  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());
  const auto isoPath = directory.filePath("testdisc.iso").toStdString();

  {
    std::ofstream iso(isoPath, std::ios::binary);
    std::vector<char> sector(RAW_SECTOR_SIZE);

    while (bin.read(sector.data(), static_cast<std::streamsize>(sector.size()))) {
      iso.write(sector.data() + USER_DATA_OFFSET, USER_DATA_SIZE);
    }
  }

  const DiscInspector inspector(m_platformService);
  std::vector<IdentifiedDiscMember> members;
  const auto identity = inspector.inspectFile(isoPath, members);

  ASSERT_TRUE(identity.isIdentified()) << "a PlayStation disc as a plain .iso was not identified at all";
  EXPECT_EQ(identity.platformId, platforms::PlatformService::PLATFORM_ID_PS1);
  // Identity is the content, so the format it arrived in must not move it
  EXPECT_EQ(identity.contentHash, TEST_DISC_HASH);
}

} // namespace firelight::library
