#include <firelight/library/archive_reader.hpp>

#include <archive.h>
#include <archive_entry.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <utility>

namespace firelight::library {
namespace {

archive *openArchive(const std::string &path) {
  archive *a = archive_read_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  if (archive_read_open_filename(a, path.c_str(), 10240) != ARCHIVE_OK) {
    archive_read_free(a);
    return nullptr;
  }
  return a;
}

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

std::string baseNameOf(const char *entryPath) {
  return std::filesystem::path(entryPath).filename().string();
}

// Reads all data blocks of the archive's current entry into a buffer.
std::vector<uint8_t> readCurrentEntry(archive *a) {
  std::vector<uint8_t> bytes;
  const void *buffer;
  size_t length;
  la_int64_t offset;
  while (archive_read_data_block(a, &buffer, &length, &offset) == ARCHIVE_OK) {
    const auto *start = static_cast<const uint8_t *>(buffer);
    bytes.insert(bytes.end(), start, start + length);
  }
  return bytes;
}
} // namespace

ArchiveReader::ArchiveReader(std::string archivePath)
    : m_archivePath(std::move(archivePath)) {}

std::vector<ArchiveReader::Entry> ArchiveReader::listEntries() const {
  std::vector<Entry> entries;
  archive *a = openArchive(m_archivePath);
  if (!a) {
    return entries;
  }

  archive_entry *entry;
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    const char *path = archive_entry_pathname(entry);
    entries.push_back({path, baseNameOf(path), archive_entry_size(entry)});
    archive_read_data_skip(a);
  }

  archive_read_free(a);
  return entries;
}

std::vector<uint8_t>
ArchiveReader::readEntryByPath(const std::string &entryPath) const {
  archive *a = openArchive(m_archivePath);
  if (!a) {
    return {};
  }

  std::vector<uint8_t> bytes;
  archive_entry *entry;
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    if (entryPath == archive_entry_pathname(entry)) {
      bytes = readCurrentEntry(a);
      break;
    }
    archive_read_data_skip(a);
  }

  archive_read_free(a);
  return bytes;
}

std::vector<uint8_t>
ArchiveReader::readEntryByBaseName(const std::string &baseNameLower) const {
  archive *a = openArchive(m_archivePath);
  if (!a) {
    return {};
  }

  std::vector<uint8_t> bytes;
  archive_entry *entry;
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    if (toLower(baseNameOf(archive_entry_pathname(entry))) == baseNameLower) {
      bytes = readCurrentEntry(a);
      break;
    }
    archive_read_data_skip(a);
  }

  archive_read_free(a);
  return bytes;
}

bool ArchiveReader::extractEntries(
    const std::set<std::string> &wantedBaseNamesLower,
    const std::filesystem::path &destDir) const {
  archive *a = openArchive(m_archivePath);
  if (!a) {
    return false;
  }

  archive_entry *entry;
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    const std::string base = baseNameOf(archive_entry_pathname(entry));
    if (!wantedBaseNamesLower.contains(toLower(base))) {
      archive_read_data_skip(a);
      continue;
    }

    std::ofstream out(destDir / base, std::ios::binary);
    if (!out) {
      continue;
    }

    const void *buffer;
    size_t length;
    la_int64_t offset;
    while (archive_read_data_block(a, &buffer, &length, &offset) == ARCHIVE_OK) {
      out.write(static_cast<const char *>(buffer),
                static_cast<std::streamsize>(length));
    }
  }

  archive_read_free(a);
  return true;
}

void ArchiveReader::forEachEntry(
    const std::function<void(const Entry &,
                             const std::function<std::vector<uint8_t>()> &)> &fn)
    const {
  archive *a = openArchive(m_archivePath);
  if (!a) {
    return;
  }

  archive_entry *entry;
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    const char *path = archive_entry_pathname(entry);
    const Entry e{path, baseNameOf(path), archive_entry_size(entry)};

    bool consumed = false;
    const std::function<std::vector<uint8_t>()> reader = [&]() {
      consumed = true;
      return readCurrentEntry(a);
    };

    fn(e, reader);
    if (!consumed) {
      archive_read_data_skip(a);
    }
  }

  archive_read_free(a);
}

} // namespace firelight::library
