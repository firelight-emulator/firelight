#include "shop_item_model.hpp"

namespace firelight::shop {
    ShopItemModel::ShopItemModel(db::IContentDatabase &contentDatabase) : m_contentDatabase(contentDatabase) {
        m_items.emplace_back(Item{
            .id = 1,
            .title = "Mario Kart 64: Amped Up",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "Litronom",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Mario Kart 64",
            .capsuleImageUrl = "file:system/_img/mk64ampedup.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "Pokémon Radical Red",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "soupercell",
            .modId = 1,
            .platformId = 1,
            .platformName = "GBA",
            .targetGameId = 1,
            .targetGameName = "Pokémon FireRed Version",
            .capsuleImageUrl = "file:system/_img/radicalred.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "Tajna and the Mana Seeds",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "Tob",
            .modId = 1,
            .platformId = 1,
            .platformName = "SNES",
            .targetGameId = 1,
            .targetGameName = "Super Mario World",
            .capsuleImageUrl = "file:system/_img/tajna.png",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "Ultimate Goomboss Challenge",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "Enneagram",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Paper Mario",
            .capsuleImageUrl = "file:system/_img/ultimategoomboss.png",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "SM64: Beyond the Cursed Mirror",
            .tagline = "Tagline",
            .description = "Description",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .capsuleImageUrl = "file:system/_img/cursedmirror.jpg",
            .userHasRequiredGame = true
        });
    }

    QHash<int, QByteArray> ShopItemModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Id] = "id";
        roles[Title] = "title";
        roles[Tagline] = "tagline";
        roles[Description] = "description";
        roles[CreatorName] = "creator_name";
        roles[ModId] = "mod_id";
        roles[PlatformId] = "platform_id";
        roles[PlatformName] = "platform_name";
        roles[TargetGameId] = "target_game_id";
        roles[TargetGameName] = "target_game_name";
        roles[CapsuleImageUrl] = "capsule_image_url";
        roles[UserHasRequiredGame] = "user_has_required_game";
        return roles;
    }

    int ShopItemModel::rowCount(const QModelIndex &parent) const {
        return m_items.size();
    }

    QVariant ShopItemModel::data(const QModelIndex &index, int role) const {
        if (role < Qt::UserRole || index.row() >= m_items.size()) {
            return QVariant{};
        }

        auto item = m_items.at(index.row());

        switch (role) {
            case Id:
                return item.id;
            case Title:
            case Qt::DisplayRole:
                return item.title;
            case Tagline:
                return item.tagline;
            case Description:
                return item.description;
            case CreatorName:
                return item.creatorName;
            case ModId:
                return item.modId;
            case PlatformId:
                return item.platformId;
            case PlatformName:
                return item.platformName;
            case TargetGameId:
                return item.targetGameId;
            case TargetGameName:
                return item.targetGameName;
            case CapsuleImageUrl:
                return item.capsuleImageUrl;
            case UserHasRequiredGame:
                return item.userHasRequiredGame;
            default:
                return QVariant{};
        }
    }
} // namespace firelight::shop
