#pragma once

#include <firelight/library/content_file.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/patch_file.hpp>
#include <firelight/library/run_configuration.hpp>

#include <optional>

namespace firelight::library {
class IUserLibraryRepository;

// The content selected to launch for an entry: the chosen ROM/disc file and,
// optionally, a patch to apply on top of it.
struct ResolvedContent {
  bool valid = false;
  ContentFile contentFile;
  std::optional<PatchFile> patch;
};

// Chooses the "most correct" content to launch for a library entry from among
// its run configurations.
class EntryResolver {
public:
  explicit EntryResolver(IUserLibraryRepository &library);

  [[nodiscard]] ResolvedContent resolve(const Entry &entry) const;

private:
  static bool contentFileExists(const ContentFile &info);
  static int scoreConfig(const RunConfiguration &config,
                         const ContentFile &contentFile);

  IUserLibraryRepository &m_library;
};

} // namespace firelight::library
