#pragma once

#include <firelight/library/identify_outcome.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace firelight::library {

/**
 * A file the scanner accepted and then could not catalogue.
 *
 * Kept per file, because the answer to "where did my game go" is a path. The row goes away as
 * soon as that path identifies, so the table holds only what is still wrong
 */
struct ScanDrop {
  int id = -1;
  std::string filePath;
  std::string archivePath;
  std::string extension;
  size_t fileSizeBytes = 0;
  IdentifyOutcome outcome = IdentifyOutcome::NotRecognized;
  std::string identifiedAs;
  int64_t firstSeenAt = 0;
  int64_t lastSeenAt = 0;
};

/**
 * How many files carry an extension nothing accepts.
 *
 * A count and never a path: a folder of three thousand save files is one row, and a list of
 * what people actually own is the whole reason to keep it
 */
struct UnrecognizedExtension {
  std::string extension;
  int count = 0;
  int64_t lastSeenAt = 0;
};

} // namespace firelight::library
