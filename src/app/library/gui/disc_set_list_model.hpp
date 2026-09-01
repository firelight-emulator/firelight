// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/disc_set.hpp>
#include <firelight/library/disc_set_member.hpp>

#include <QAbstractListModel>
#include <vector>

namespace firelight::library {
class IUserLibraryRepository;
}

namespace firelight::gui {

// TODO
/**
 * Every disc set and the discs in it, flattened so one list can show both.
 *
 * A set is a row and so is each of its discs, told apart by the IsSet role. Flat rather than nested
 * because the shape is only ever drawn as an indented list
 */
class DiscSetListModel final : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    IsSet = Qt::UserRole + 1,
    SetId,
    Title,
    DiscNumber,
    MemberId,
    ContentFileId,
    FileName,
    IsPresent,
    IsClaimed, // Named by something, but no file catalogued under that name yet
    IsUncertain,
    Source,   // Which rung of the ladder decided, in words
    DiscCount // How many discs the game came on, 0 when nothing has said
  };

  explicit DiscSetListModel(library::IUserLibraryRepository &library, QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  /**
   * Reads every set and its discs again
   */
  Q_INVOKABLE void refresh();

private:
  // TODO
  // One row of the flattened list: a set, or a disc of the set above it
  struct Row {
    bool isSet = false;
    int setId = -1;
    QString title;
    int discNumber = 0;
    int memberId = -1;
    int contentFileId = -1;
    QString fileName;
    bool isPresent = false;
    bool isClaimed = false;
    bool isUncertain = false;
    QString source;
    int discCount = 0;
  };

  library::IUserLibraryRepository &m_library;
  std::vector<Row> m_rows;
};

} // namespace firelight::gui
