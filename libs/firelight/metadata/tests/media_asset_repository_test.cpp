#include <firelight/metadata/sqlite_media_asset_repository.hpp>

#include <gtest/gtest.h>

namespace firelight::metadata {
  class MediaAssetRepositoryTest : public testing::Test {
  protected:
    SqliteMediaAssetRepository repo{":memory:"};

    static MediaAsset make(const std::string &hash, MediaType type,
                           MediaSource source, const std::string &url,
                           const std::string &extId = "") {
      MediaAsset asset;
      asset.contentHash = hash;
      asset.type = type;
      asset.source = source;
      asset.remoteUrl = url;
      asset.externalId = extId;
      return asset;
    }
  };

  TEST_F(MediaAssetRepositoryTest, AddAndList) {
    auto icon = make("h1", MediaType::Icon, MediaSource::RetroAchievements, "u1");
    ASSERT_TRUE(repo.add(icon));
    EXPECT_GT(icon.id, 0);

    const auto all = repo.listForContent("h1");
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].remoteUrl, "u1");
    EXPECT_EQ(all[0].source, MediaSource::RetroAchievements);
    EXPECT_EQ(all[0].type, MediaType::Icon);
  }

  TEST_F(MediaAssetRepositoryTest, ThumbUrlRoundTrips) {
    auto icon = make("h1", MediaType::Icon, MediaSource::SteamGridDb, "full", "9");
    icon.thumbUrl = "thumb";
    ASSERT_TRUE(repo.add(icon));

    const auto all = repo.listForContent("h1");
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].thumbUrl, "thumb");
    EXPECT_EQ(all[0].displaySource(), "full"); // full stays the persisted source
    EXPECT_EQ(all[0].displayThumb(), "thumb"); // small surfaces use the thumb
  }

  TEST_F(MediaAssetRepositoryTest, DisplayThumbFallsBackToSource) {
    auto icon = make("h1", MediaType::Icon, MediaSource::RetroAchievements, "u1");
    ASSERT_TRUE(repo.add(icon));
    EXPECT_EQ(repo.listForContent("h1")[0].displayThumb(), "u1");
  }

  TEST_F(MediaAssetRepositoryTest, AddIsIdempotentOnDuplicate) {
    auto a = make("h1", MediaType::Icon, MediaSource::RetroAchievements, "u1");
    auto b = make("h1", MediaType::Icon, MediaSource::RetroAchievements, "u1");
    ASSERT_TRUE(repo.add(a));
    ASSERT_TRUE(repo.add(b));
    EXPECT_EQ(a.id, b.id);
    EXPECT_EQ(repo.listForContent("h1").size(), 1u);
  }

  TEST_F(MediaAssetRepositoryTest, SingleSelectedPerType) {
    auto ra = make("h1", MediaType::Icon, MediaSource::RetroAchievements, "u1");
    ra.selected = true;
    auto sg = make("h1", MediaType::Icon, MediaSource::SteamGridDb, "u2", "sg42");
    ASSERT_TRUE(repo.add(ra));
    ASSERT_TRUE(repo.add(sg));

    auto sel = repo.selectedFor("h1", MediaType::Icon);
    ASSERT_TRUE(sel.has_value());
    EXPECT_EQ(sel->remoteUrl, "u1");

    ASSERT_TRUE(repo.setSelected(sg.id));
    sel = repo.selectedFor("h1", MediaType::Icon);
    ASSERT_TRUE(sel.has_value());
    EXPECT_EQ(sel->id, sg.id);

    int selectedCount = 0;
    for (const auto &x: repo.listForContentAndType("h1", MediaType::Icon)) {
      if (x.selected) {
        ++selectedCount;
      }
    }
    EXPECT_EQ(selectedCount, 1);
  }

  TEST_F(MediaAssetRepositoryTest, DifferentTypesCoexist) {
    auto icon = make("h1", MediaType::Icon, MediaSource::RetroAchievements, "i");
    auto box =
        make("h1", MediaType::BoxartFront, MediaSource::RetroAchievements, "b");
    ASSERT_TRUE(repo.add(icon));
    ASSERT_TRUE(repo.add(box));
    EXPECT_EQ(repo.listForContentAndType("h1", MediaType::Icon).size(), 1u);
    EXPECT_EQ(repo.listForContentAndType("h1", MediaType::BoxartFront).size(), 1u);
  }

  TEST_F(MediaAssetRepositoryTest, Remove) {
    auto imported = make("h1", MediaType::Icon, MediaSource::User, "");
    imported.localPath = "/tmp/x.png";
    ASSERT_TRUE(repo.add(imported));
    ASSERT_TRUE(repo.remove(imported.id));
    EXPECT_TRUE(repo.listForContent("h1").empty());
  }
} // namespace firelight::metadata
