// TODO: NEEDS REVIEW
#include "firelight/library/content_discoverer.hpp"

#include "firelight/library/archive_reader.hpp"
#include "firelight/library/content_extensions.hpp"
#include "firelight/library/content_identifier.hpp"
#include "firelight/library/disc_inspector.hpp"
#include "firelight/library/file_bytes.hpp"

#include <firelight/library/accepted_extensions.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/util/strings.hpp>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <set>

namespace firelight::library {

ContentFile toContentFile(const DiscoveredFile &file, const IdentifiedContent &identified) {
  return ContentFile{.m_type = identified.isDisc ? ContentType::Disc : ContentType::Cartridge,
                     .m_fileSizeBytes = identified.fileSizeBytes,
                     .m_filePath = file.path,
                     .m_fileMd5 = identified.fileMd5,
                     // Not computed for content yet; DAT matching against content.db is what would need it
                     .m_fileCrc32 = "",
                     .m_inArchive = file.isInArchive(),
                     .m_archivePathName = file.archivePath,
                     .m_platformId = identified.platformId,
                     .m_contentHash = identified.contentHash,
                     .m_discNumber = file.tags.discNumber,
                     .m_regions = file.tags.regions};
}

namespace {
// The containers the walk descends into rather than catalogues
bool isArchiveExtension(const std::string &extension) {
  return extension == "zip" || extension == "7z" || extension == "tar" || extension == "rar";
}

// A patch is catalogued by its bytes rather than identified, so it leaves the pipeline early
bool isPatchExtension(const std::string &extension) {
  return extension == "ips" || extension == "bps" || extension == "ups" || extension == "mod";
}

// Windows-authored cue sheets carry backslashes, and a sheet may name a track by a path
std::string baseNameOfToken(const std::string &token) {
  const auto separator = token.find_last_of("/\\");
  return strings::toLower(separator == std::string::npos ? token : token.substr(separator + 1));
}
} // namespace

// TODO
// Every track name the sheets of one container already speak for. One rule over two sources, so a
// sheet format cannot be recognised on disk and missed inside an archive
class ContentDiscoverer::TrackSuppressor {
public:
  explicit TrackSuppressor(std::string directoryPath, std::string archivePath)
      : m_directoryPath(std::move(directoryPath)), m_archivePath(std::move(archivePath)) {}

  [[nodiscard]] bool speaksFor(const std::string &path) {
    if (!m_names.has_value()) {
      m_names = m_archivePath.empty() ? namesInDirectory() : namesInArchive();
    }

    return m_names->count(baseNameOfToken(path)) > 0;
  }

private:
  [[nodiscard]] std::set<std::string> namesInDirectory() const {
    std::set<std::string> names;
    const QDir directory(QString::fromStdString(m_directoryPath));

    for (const auto &name : directory.entryList(QDir::Files)) {
      if (!namesTracks(suffixOf(name.toStdString()))) {
        continue;
      }

      const auto bytes = readAllBytes(directory.absoluteFilePath(name).toStdString());

      for (const auto &candidate : DiscInspector::sheetFilenameCandidates(bytes)) {
        names.insert(baseNameOfToken(candidate));
      }
    }

    return names;
  }

  [[nodiscard]] std::set<std::string> namesInArchive() const {
    std::set<std::string> names;
    const ArchiveReader reader(m_archivePath);

    for (const auto &entry : reader.listEntries()) {
      if (!namesTracks(suffixOf(entry.baseName))) {
        continue;
      }

      for (const auto &candidate : DiscInspector::sheetFilenameCandidates(reader.readEntryByPath(entry.pathName))) {
        names.insert(baseNameOfToken(candidate));
      }
    }

    return names;
  }

  std::string m_directoryPath;
  std::string m_archivePath;
  std::optional<std::set<std::string>> m_names;
};

ContentDiscoverer::ContentDiscoverer(const platforms::IPlatformService &platformService,
                                     const int64_t maxUnrecognizedFileBytes)
    : m_platformService(platformService), m_maxUnrecognizedFileBytes(maxUnrecognizedFileBytes) {}

std::optional<DiscoveredFile> ContentDiscoverer::triage(const std::string &path, const std::string &archivePath,
                                                        const size_t sizeBytes, const std::set<std::string> &accepted,
                                                        const std::function<std::vector<uint8_t>()> &readBytes,
                                                        TrackSuppressor &suppressor, Skipped &skipped) const {
  const auto extension = suffixOf(path);

  if (extension.empty()) {
    return std::nullopt;
  }

  // TODO
  // The reader handed in by an archive walk is only good while its callback is running, and
  // anything put in Skipped is read after the walk has finished. This one re-opens instead
  const auto deferredReader = [path, archivePath] {
    return archivePath.empty() ? readAllBytes(path) : ArchiveReader(archivePath).readEntryByPath(path);
  };

  if (isPatchExtension(extension)) {
    skipped.patches.push_back(DiscoveredFile{.path = path,
                                             .archivePath = archivePath,
                                             .extension = extension,
                                             .sizeBytes = sizeBytes,
                                             .isDisc = false,
                                             .tags = parseFilenameTags(path),
                                             .readBytes = deferredReader});
    return std::nullopt;
  }

  if (accepted.count(extension) == 0) {
    // TODO
    // Counted rather than listed: this answers which formats people own, and a folder of save
    // files must not become a folder of rows
    ++skipped.unrecognizedExtensions[extension];
    return std::nullopt;
  }

  const auto isDisc = isDiscExtension(extension);

  // TODO
  // After the acceptance gate, so a huge file nothing accepts is still one tally rather than a row
  // of its own. Disc images are exempt because they are legitimately this large
  if (!isDisc && static_cast<int64_t>(sizeBytes) > m_maxUnrecognizedFileBytes) {
    skipped.tooLarge.push_back(DiscoveredFile{.path = path,
                                              .archivePath = archivePath,
                                              .extension = extension,
                                              .sizeBytes = sizeBytes,
                                              .isDisc = false,
                                              .tags = parseFilenameTags(path),
                                              .readBytes = deferredReader});
    return std::nullopt;
  }

  // TODO
  // A raw track is reached through the sheet naming it. One nothing names is a file in its own
  // right, and skipping those is how a zipped cartridge went missing
  if (isDiscTrackExtension(extension) && suppressor.speaksFor(path)) {
    return std::nullopt;
  }

  return DiscoveredFile{.path = path,
                        .archivePath = archivePath,
                        .extension = extension,
                        .sizeBytes = sizeBytes,
                        .isDisc = isDisc,
                        .tags = parseFilenameTags(path),
                        .readBytes = readBytes};
}

ContentDiscoverer::Skipped ContentDiscoverer::walk(const std::string &directoryPath,
                                                   const std::function<void(const DiscoveredFile &)> &fn,
                                                   const std::function<bool()> &keepGoing,
                                                   const bool includeFiles) const {
  Skipped skipped;
  const auto accepted = acceptedExtensions(m_platformService);
  TrackSuppressor looseSuppressor(directoryPath, {});

  QDirIterator iter(QString::fromStdString(directoryPath), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

  while (iter.hasNext()) {
    if (!keepGoing()) {
      return skipped;
    }

    const auto fileInfo = iter.nextFileInfo();

    // TODO
    // Collected even when the files are being skipped: a subdirectory can change without its
    // parent's mtime moving, so the descent has to happen either way
    if (fileInfo.isDir()) {
      skipped.subdirectories.push_back(fileInfo.filePath().toStdString());
      continue;
    }

    if (!includeFiles) {
      continue;
    }

    const auto path = fileInfo.filePath().toStdString();
    const auto extension = suffixOf(path);

    if (isArchiveExtension(extension)) {
      TrackSuppressor archiveSuppressor({}, path);

      ArchiveReader(path).forEachEntry(
          [&](const ArchiveReader::Entry &entry, const std::function<std::vector<uint8_t>()> &readBytes) {
            if (entry.size <= 0) {
              return;
            }

            const auto found = triage(entry.pathName, path, static_cast<size_t>(entry.size), accepted, readBytes,
                                      archiveSuppressor, skipped);

            if (found.has_value()) {
              fn(*found);
            }
          });
      continue;
    }

    const auto found = triage(
        path, {}, static_cast<size_t>(fileInfo.size()), accepted, [path] { return readAllBytes(path); },
        looseSuppressor, skipped);

    if (found.has_value()) {
      fn(*found);
    }
  }

  skipped.completed = true;
  return skipped;
}

} // namespace firelight::library
