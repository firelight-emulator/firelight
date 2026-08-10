#pragma once

namespace firelight::library {

/**
 * Routes rcheevos' hashing diagnostics into the app's log.
 *
 * Without this they are discarded, and rcheevos is the only thing that knows why a file failed
 * to identify — which console candidates it tried, and what each one objected to.
 *
 * Both streams log below info: rcheevos reports a console candidate not matching through its
 * error channel, so a disc that identifies on the ninth candidate reports eight of them first
 *
 * Safe to call more than once; only the first call installs
 */
void installRcHashLogging();

} // namespace firelight::library
