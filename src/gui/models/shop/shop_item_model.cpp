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
            .capsuleImageUrl = "qrc:images/mods/mk64ampedup",
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
            .capsuleImageUrl = "qrc:images/mods/radicalred",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "Tajna and the Mana Seeds",
            .tagline = "Take control of Tajna in this complete overhaul hack for Super Mario World!",
            .description =
            "Fight, Jump and Spin yourself through these unique stages! Each of the 6 stages comes with their own collectibles and platforming challenges. This hack takes inspiration from old and new games and romhacks to introduce its mechanics in a simple and intuitive way.",
            .creatorName = "Tob",
            .modId = 1,
            .platformId = 1,
            .platformName = "SNES",
            .targetGameId = 1,
            .targetGameName = "Super Mario World",
            .capsuleImageUrl = "qrc:images/mods/tajna",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "Ultimate Goomboss Challenge",
            .tagline = "Saving the world is hungry work.",
            .description =
            "With the Star Rod recovered, a satisfied Mario and Goombario head out for a bite to eat. But who's this in their favourite picnic spot? And more importantly, can you come up with a strategy to defeat... THE ULTIMATE GOOMBOSS CHALLENGE? Made as a love letter to RPG boss battles, and a determination to make the most epic possible final product using only Goombas.",
            .creatorName = "Enneagram",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Paper Mario",
            .capsuleImageUrl = "qrc:images/mods/ultimategoomboss",
            .userHasRequiredGame = true
        });
        m_items.emplace_back(Item{
            .id = 1,
            .title = "Super Mario 64: Beyond the Cursed Mirror",
            .tagline =
            "Greetings Superstar, did you know that your nemesis hid in the decrepit castle all this time? It appears he fosters his strength in a hidden compartment behind one cursed mirror. I implore you to defeat him, once and for all – for the sake of the kingdom. -Yours truly, The Showrunner",
            .description =
            "SM64: Beyond the Cursed Mirror is a major Super Mario 64 ROM hack created by Rovert, which contains 15 custom-made courses and 120 stars waiting to be collected. With a unique in-depth story, original characters, and new mechanics, this SM64 ROM hack will provide a traditional yet unique experience.",
            .creatorName = "rovertronic",
            .modId = 1,
            .platformId = 1,
            .platformName = "Nintendo 64",
            .targetGameId = 1,
            .targetGameName = "Super Mario 64",
            .clearLogoImageUrl = "file:system/_img/cursedmirrorclear.png",
            .capsuleImageUrl = "qrc:images/mods/cursedmirror",
            .userHasRequiredGame = true,
            .screenshotUrls = {
                "file:system/_img/cursedmirror1.png",
                "file:system/_img/cursedmirror2.png",
                "file:system/_img/cursedmirror3.png",
                "file:system/_img/cursedmirror4.png",
                "file:system/_img/cursedmirror5.png",
                "file:system/_img/cursedmirror6.png"
            }
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
        roles[ClearLogoImageUrl] = "clear_logo_image_url";
        roles[CapsuleImageUrl] = "capsule_image_url";
        roles[UserHasRequiredGame] = "user_has_required_game";
        roles[ScreenshotUrls] = "screenshot_urls";
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
            case ClearLogoImageUrl:
                return item.clearLogoImageUrl;
            case CapsuleImageUrl:
                return item.capsuleImageUrl;
            case UserHasRequiredGame:
                return item.userHasRequiredGame;
            case ScreenshotUrls:
                return item.screenshotUrls;
            default:
                return QVariant{};
        }
    }
} // namespace firelight::shop
