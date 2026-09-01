// TODO: NEEDS REVIEW
#include "disc_set_list_model.hpp"

#include <firelight/library/user_library_repository.hpp>

#include <QFileInfo>

namespace firelight::gui {

namespace {
// TODO
// Which rung of the ladder placed a disc, in the words the person looking at it would use
QString describeSource(const library::DiscSource source) {
  switch (source) {
  case library::DiscSource::User:
    return QObject::tr("you chose this");
  case library::DiscSource::PlaylistFile:
    return QObject::tr("from your playlist");
  case library::DiscSource::Database:
    return QObject::tr("from the database");
  case library::DiscSource::Filename:
    return QObject::tr("from the file name");
  }

  return {};
}
} // namespace

DiscSetListModel::DiscSetListModel(library::IUserLibraryRepository &library, QObject *parent)
    : QAbstractListModel(parent), m_library(library) {
  refresh();
}

int DiscSetListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return static_cast<int>(m_rows.size());
}

QVariant DiscSetListModel::data(const QModelIndex &index, const int role) const {
  if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size())) {
    return {};
  }

  const auto &row = m_rows[index.row()];

  switch (role) {
  case IsSet:
    return row.isSet;
  case SetId:
    return row.setId;
  case Title:
    return row.title;
  case DiscNumber:
    return row.discNumber;
  case MemberId:
    return row.memberId;
  case ContentFileId:
    return row.contentFileId;
  case FileName:
    return row.fileName;
  case IsPresent:
    return row.isPresent;
  case IsClaimed:
    return row.isClaimed;
  case IsUncertain:
    return row.isUncertain;
  case Source:
    return row.source;
  case DiscCount:
    return row.discCount;
  default:
    return {};
  }
}

QHash<int, QByteArray> DiscSetListModel::roleNames() const {
  return {{IsSet, "isSet"},         {SetId, "setId"},
          {Title, "title"},         {DiscNumber, "discNumber"},
          {MemberId, "memberId"},   {ContentFileId, "contentFileId"},
          {FileName, "fileName"},   {IsPresent, "isPresent"},
          {IsClaimed, "isClaimed"}, {IsUncertain, "isUncertain"},
          {Source, "source"},       {DiscCount, "discCount"}};
}

void DiscSetListModel::refresh() {
  beginResetModel();
  m_rows.clear();

  for (const auto &set : m_library.getDiscSets()) {
    const auto members = m_library.getDiscSetMembers(set.id);

    m_rows.push_back(Row{.isSet = true,
                         .setId = set.id,
                         .title = set.title.empty() ? tr("(unnamed)") : QString::fromStdString(set.title),
                         .discCount = set.discCount});

    for (const auto &member : members) {
      Row row;
      row.setId = set.id;
      row.discNumber = member.m_discNumber;
      row.memberId = member.m_id;
      row.isUncertain = member.m_isUncertain;
      row.source = describeSource(member.m_source);
      row.fileName = QFileInfo(QString::fromStdString(member.m_memberPath)).fileName();

      // A row naming a file nothing has catalogued is holding that disc's place until it turns up
      row.isClaimed = !member.m_contentFileId.has_value();

      if (member.m_contentFileId.has_value()) {
        row.contentFileId = *member.m_contentFileId;

        if (const auto file = m_library.getContentFile(row.contentFileId)) {
          row.isPresent = file->m_missingSince == 0;
        }
      }

      m_rows.push_back(row);
    }
  }

  endResetModel();
}

} // namespace firelight::gui
