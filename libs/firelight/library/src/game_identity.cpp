// TODO: NEEDS REVIEW
#include <firelight/library/game_identity.hpp>

namespace firelight::library {

GameIdentity identityOf(const Entry &entry) {
  return GameIdentity{
      .platformId = entry.platformId,
      .title = entry.normalizedTitle,
      .regions = entry.metadata.regions,
  };
}

GameIdentity identityOf(const ContentFile &file) {
  return GameIdentity{
      .platformId = static_cast<unsigned>(file.m_platformId),
      .title = file.m_normalizedTitle,
      .regions = file.m_regions,
      .discNumber = file.m_discNumber,
  };
}

bool areSameGame(const GameIdentity &left, const GameIdentity &right) {
  if (left.platformId != right.platformId) {
    return false;
  }

  return !left.title.empty() && left.title == right.title;
}

bool areRegionsCompatible(const std::vector<std::string> &left, const std::vector<std::string> &right) {
  return left.empty() || right.empty() || left == right;
}

bool areVariants(const GameIdentity &left, const GameIdentity &right) {
  return areSameGame(left, right) && left.discNumber == right.discNumber;
}

bool areDiscsOfOneRelease(const GameIdentity &left, const GameIdentity &right) {
  return areSameGame(left, right) && areRegionsCompatible(left.regions, right.regions);
}

} // namespace firelight::library
