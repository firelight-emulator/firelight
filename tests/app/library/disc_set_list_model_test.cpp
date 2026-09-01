// TODO: NEEDS REVIEW
#include "app/library/gui/disc_set_list_model.hpp"

#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>

#include <QTemporaryDir>
#include <gtest/gtest.h>

// What the disc set manager shows: every set and the discs under it, flattened into one list
namespace firelight::gui {

class DiscSetListModelTest : public testing::Test {
protected:
  QTemporaryDir m_root;
  library::SqliteUserLibraryRepository m_repo{":memory:"};
  library::DiscSetService m_discSets{m_repo, (m_root.path() + "/appdata").toStdString()};
  library::LibraryIngestService m_ingest{m_repo, m_discSets};

  std::string romsPath() const { return (m_root.path() + "/roms").toStdString(); }

  void addDisc(const std::string &title, const std::string &hash, const int discNumber) {
    library::ContentFile file;
    file.m_type = library::ContentType::Disc;
    file.m_filePath = romsPath() + "/" + title + " (Disc " + std::to_string(discNumber) + ").cue";
    file.m_platformId = 7;
    file.m_contentHash = hash;
    file.m_discNumber = discNumber;
    file.m_normalizedTitle = library::normalizeTitle(title);
    file.m_fileSizeBytes = 1000 + discNumber;
    ASSERT_TRUE(m_repo.create(file));
  }

  QVariant roleAt(const DiscSetListModel &model, const int row, const int role) {
    return model.data(model.index(row, 0), role);
  }
};

// A set is a row and so is each of its discs, in disc order under the set they belong to
TEST_F(DiscSetListModelTest, ASetAndItsDiscsAreOneFlatList) {
  addDisc("Final Fantasy VII", "disc1", 1);
  addDisc("Final Fantasy VII", "disc2", 2);

  DiscSetListModel model(m_repo);

  ASSERT_EQ(model.rowCount(), 3) << "one set and two discs did not make three rows";

  EXPECT_TRUE(roleAt(model, 0, DiscSetListModel::IsSet).toBool());
  EXPECT_FALSE(roleAt(model, 1, DiscSetListModel::IsSet).toBool());
  EXPECT_FALSE(roleAt(model, 2, DiscSetListModel::IsSet).toBool());

  EXPECT_EQ(roleAt(model, 1, DiscSetListModel::DiscNumber).toInt(), 1);
  EXPECT_EQ(roleAt(model, 2, DiscSetListModel::DiscNumber).toInt(), 2);

  // Every disc says which set it is under, so the list can be read without tracking the last header
  const auto setId = roleAt(model, 0, DiscSetListModel::SetId).toInt();
  EXPECT_EQ(roleAt(model, 1, DiscSetListModel::SetId).toInt(), setId);
  EXPECT_EQ(roleAt(model, 2, DiscSetListModel::SetId).toInt(), setId);
}

// TODO
// A disc a playlist named but nobody has yet is shown as holding a place rather than as a file
TEST_F(DiscSetListModelTest, ADiscNobodyHasIsShownAsAClaim) {
  addDisc("Lunar", "disc2", 2);

  const auto owned = m_repo.getContentFileWithPath(romsPath() + "/Lunar (Disc 2).cue");
  ASSERT_TRUE(owned.has_value());

  ASSERT_TRUE(
      m_discSets.claimPlaylist(romsPath() + "/lunar.m3u", {romsPath() + "/Lunar (Disc 1).cue", owned->m_filePath})
          .has_value());

  DiscSetListModel model(m_repo);
  ASSERT_EQ(model.rowCount(), 3);

  EXPECT_TRUE(roleAt(model, 1, DiscSetListModel::IsClaimed).toBool()) << "a disc nobody has read as a real file";
  EXPECT_FALSE(roleAt(model, 1, DiscSetListModel::IsPresent).toBool());
  EXPECT_FALSE(roleAt(model, 2, DiscSetListModel::IsClaimed).toBool());
  EXPECT_TRUE(roleAt(model, 2, DiscSetListModel::IsPresent).toBool());

  EXPECT_EQ(roleAt(model, 1, DiscSetListModel::Source).toString(), QStringLiteral("from your playlist"));
}

// TODO
// Taking a disc out says where it does not belong, not that it is a game on its own, so the set of
// one it lands in is shown as a guess
TEST_F(DiscSetListModelTest, ADetachedDiscIsShownAsUncertain) {
  addDisc("Koudelka", "disc1", 1);
  addDisc("Koudelka", "disc2", 2);

  const auto second = m_repo.getContentFileWithPath(romsPath() + "/Koudelka (Disc 2).cue");
  ASSERT_TRUE(second.has_value());
  ASSERT_TRUE(m_discSets.detachDisc(second->m_id));

  DiscSetListModel model(m_repo);

  auto uncertain = 0;
  for (auto row = 0; row < model.rowCount(); ++row) {
    if (!roleAt(model, row, DiscSetListModel::IsSet).toBool() &&
        roleAt(model, row, DiscSetListModel::IsUncertain).toBool()) {
      ++uncertain;
    }
  }

  EXPECT_EQ(uncertain, 1) << "the disc that was pulled out was not shown as a guess";
}

} // namespace firelight::gui
