#pragma once

namespace firelight::library {

/**
 * What happened when a file was identified.
 *
 * Anything other than Identified is a file we accepted and then could not catalogue, which is
 * recorded rather than dropped. The distinction between the failures is what says whose fault it
 * is: NoIdentifier is ours, NotRecognized is the dump's
 */
enum class IdentifyOutcome {
  Identified,
  // Nothing knows how to read this extension, so it can never identify however good the dump is
  NoIdentifier,
  // Something tried and the bytes matched nothing it knows
  NotRecognized,
  // The platform was known but hashing produced nothing
  HashFailed,
  // The bytes could not be read at all
  Unreadable,
  // Something read the format and named a system Firelight does not model
  PlatformNotSupported,
};

} // namespace firelight::library
