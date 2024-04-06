#pragma once

#include <QAbstractListModel>
#include <firelight/content_database.hpp>

namespace firelight::gui {

class ModListModel final : public QAbstractListModel {
  Q_OBJECT

public:
  /**
   * @enum Roles
   * @brief The roles that can be used with this model.
   */
  enum Roles {
    Id = Qt::UserRole + 1,
    Name,
    PrimaryAuthor,
    Platform,
    TargetGameName,
    TargetGameId
  };

  explicit ModListModel(IContentDatabase &contentDatabase);
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
    QString primaryAuthor;
    QString platform;
    QString targetGameName;
    int targetGameId;
  };

  IContentDatabase &m_contentDatabase;
  std::vector<Item> m_items;
};

} // namespace firelight::gui