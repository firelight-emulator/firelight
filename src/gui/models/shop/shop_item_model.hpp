#pragma once

#include <QAbstractListModel>
#include <firelight/content_database.hpp>

namespace firelight::shop {
  class ShopItemModel final : public QAbstractListModel {
    Q_OBJECT

  public:
    /**
     * @enum Roles
     * @brief The roles that can be used with this model.
     */
    enum Roles {
      Id = Qt::UserRole + 1,
      Title,
      Tagline,
      Description,
      CreatorName,
      ModId,
      PlatformId,
      PlatformName,
      TargetGameId,
      TargetGameName,
      CapsuleImageUrl,
      UserHasRequiredGame
    };

    explicit ShopItemModel(db::IContentDatabase &contentDatabase);

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
      QString title;
      QString tagline;
      QString description;
      QString creatorName;
      int modId;
      int platformId;
      QString platformName;
      int targetGameId;
      QString targetGameName;
      QString capsuleImageUrl;
      bool userHasRequiredGame;
    };

    db::IContentDatabase &m_contentDatabase;
    QList<Item> m_items;
  };
}
