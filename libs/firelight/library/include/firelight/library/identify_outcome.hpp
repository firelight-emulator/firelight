#pragma once

namespace firelight::library {

/**
 * What happened when a file was identified. Anything other than Identified is a file we accepted and then
 * could not catalog
 */
enum class IdentifyOutcome {
  Identified,
  NoIdentifier,        // Nothing knows how to read this extension, so it can never identify however good the dump is
  NotRecognized,       // Something tried and the bytes matched nothing it knows
  HashFailed,          // The platform was known but hashing produced nothing
  Unreadable,          // The bytes could not be read at all
  PlatformNotSupported // Something read the format and named a system we don't model
};

} // namespace firelight::library
