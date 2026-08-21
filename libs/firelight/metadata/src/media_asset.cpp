#include <firelight/metadata/media_asset.hpp>

namespace firelight::metadata {
const char *toString(const MediaSource source) {
  switch (source) {
  case MediaSource::RetroAchievements:
    return "retroachievements";
  case MediaSource::SteamGridDb:
    return "steamgriddb";
  case MediaSource::User:
    return "user";
  case MediaSource::Unknown:
  default:
    return "unknown";
  }
}

MediaSource mediaSourceFromString(const std::string &value) {
  if (value == "retroachievements") {
    return MediaSource::RetroAchievements;
  }
  if (value == "steamgriddb") {
    return MediaSource::SteamGridDb;
  }
  if (value == "user") {
    return MediaSource::User;
  }

  return MediaSource::Unknown;
}
} // namespace firelight::metadata
