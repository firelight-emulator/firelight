#pragma once

#include <firelight/library/patch_file.hpp>

namespace firelight::library {

// Associates a discovered patch file with the ROM(s) it targets, creating the
// run configuration(s) that let an entry launch the patched content.
//
// The intended backing is the read-only content database (its `patches` table
// links a patch's hash to a known ROM), but that matching is a future feature;
// NullPatchAssociator is the current no-op implementation.
class IPatchAssociator {
public:
  virtual ~IPatchAssociator() = default;

  // Returns the number of associations created for the given patch.
  virtual int associate(const PatchFile &patch) = 0;
};

// No-op associator used until content-database patch matching lands.
class NullPatchAssociator final : public IPatchAssociator {
public:
  int associate(const PatchFile & /*patch*/) override { return 0; }
};

} // namespace firelight::library
