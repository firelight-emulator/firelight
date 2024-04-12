#pragma once

#include <QAbstractListModel>
#include <firelight/content_database.hpp>

namespace firelight::gui {

class ModItemModel final : public QAbstractListModel {
  Q_OBJECT

public:
  /**
   * @enum Roles
   * @brief The roles that can be used with this model.
   */
  enum Roles {
    Id = Qt::UserRole + 1,
    Name,
    Description,
    PrimaryAuthor,
    Platform,
    TargetGameName,
    TargetGameId,
    ImageSource,
    RomIds
  };

  explicit ModItemModel(db::IContentDatabase &contentDatabase);
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

private:
  /**
   * @struct Item
   * @brief A struct representing an item in the model.
   */
  struct Item {
    int id;
    QString name;
    QString description;
    QString primaryAuthor;
    QString platform;
    QString targetGameName;
    int targetGameId;
    QString imageSource;
    QList<int> romIds;
  };

  db::IContentDatabase &m_contentDatabase;
  std::vector<Item> m_items;
};

} // namespace firelight::gui