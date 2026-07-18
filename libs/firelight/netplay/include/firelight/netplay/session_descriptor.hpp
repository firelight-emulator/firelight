#pragma once

#include "sync_strategy.hpp"

#include <string>

namespace firelight::netplay {

// TODO
// The game the lobby has selected. contentHash/platformId identify the content
// (lockstep verifies them later); gameName/artUrl are for display, so guests
// without the content can still see what's being played
struct SessionDescriptor {
  std::string gameName;
  std::string contentHash;
  int platformId = 0;
  std::string artUrl;
  SyncStrategyKind strategy = SyncStrategyKind::HostStream;

  bool operator==(const SessionDescriptor &) const = default;
};

} // namespace firelight::netplay
