#include <firelight/metadata/media_slot.hpp>

namespace firelight::metadata {

const std::vector<MediaType> &ladderFor(const MediaSlot slot) {
  // A slot's own shape comes first, then the other grid shapes, then anything with a
  // picture in it. Falling to a differently-shaped grid crops badly but still beats
  // an empty tile
  static const std::vector<MediaType> squareTile = {MediaType::GridSquare,   MediaType::Icon,
                                                    MediaType::GridVertical, MediaType::GridPortrait,
                                                    MediaType::TitleScreen,  MediaType::Ingame};
  static const std::vector<MediaType> portraitTile = {MediaType::GridPortrait, MediaType::GridVertical,
                                                      MediaType::BoxartFront,  MediaType::GridSquare,
                                                      MediaType::TitleScreen,  MediaType::Ingame};
  static const std::vector<MediaType> verticalTile = {MediaType::GridVertical, MediaType::GridPortrait,
                                                      MediaType::BoxartFront,  MediaType::GridSquare,
                                                      MediaType::TitleScreen,  MediaType::Ingame};
  static const std::vector<MediaType> bannerTile = {MediaType::GridBanner, MediaType::Hero, MediaType::Ingame,
                                                    MediaType::TitleScreen};
  static const std::vector<MediaType> boxartFront = {MediaType::BoxartFront, MediaType::GridPortrait,
                                                     MediaType::GridVertical, MediaType::GridSquare};
  static const std::vector<MediaType> boxartBack = {MediaType::BoxartBack};
  static const std::vector<MediaType> hero = {MediaType::Hero, MediaType::GridBanner, MediaType::Ingame,
                                              MediaType::TitleScreen};

  switch (slot) {
  case MediaSlot::TilePortrait:
    return portraitTile;
  case MediaSlot::TileVertical:
    return verticalTile;
  case MediaSlot::TileBanner:
    return bannerTile;
  case MediaSlot::BoxartFront:
    return boxartFront;
  case MediaSlot::BoxartBack:
    return boxartBack;
  case MediaSlot::Hero:
    return hero;
  case MediaSlot::TileSquare:
  default:
    return squareTile;
  }
}

std::optional<MediaAsset> resolveSlot(const std::vector<MediaAsset> &assets, const MediaSlot slot) {
  for (const auto type : ladderFor(slot)) {
    const MediaAsset *first = nullptr;

    for (const auto &asset : assets) {
      if (asset.type != type) {
        continue;
      }

      if (asset.selected) {
        return asset;
      }

      if (first == nullptr) {
        first = &asset;
      }
    }

    if (first != nullptr) {
      return *first;
    }
  }

  return std::nullopt;
}

} // namespace firelight::metadata
