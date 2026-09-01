// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/content_file.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/patch_file.hpp>
#include <firelight/library/run_configuration.hpp>

#include <optional>
#include <string>

namespace firelight::library {
class IUserLibraryRepository;

// The content selected to launch for an entry: the chosen ROM/disc file and,
// optionally, a patch to apply on top of it
struct ResolvedContent {
  bool valid = false;
  ContentFile contentFile;
  std::optional<PatchFile> patch;

  // The set this launches, unset for a single file. The path in contentFile is a playlist that we generated
  std::optional<int> discSetId{};
};

// TODO
/**
 * Works out what to hand the core for a library entry: the way in the entry launches through, or
 * the next usable one when that way in names a file nobody has
 */
class EntryResolver {
public:
  EntryResolver(IUserLibraryRepository &library, std::string appDataDirectory);

  [[nodiscard]] ResolvedContent resolve(const Entry &entry) const;

private:
  static bool contentFileExists(const ContentFile &info);

  IUserLibraryRepository &m_library;

  // TODO: Give it app data or a specific directory?
  // Holds the playlists a disc set launches through, which are derived from the identity
  // rather than recorded anywhere
  std::string m_appDataDirectory;
};

} // namespace firelight::library
