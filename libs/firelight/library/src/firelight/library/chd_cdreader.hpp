#pragma once

namespace firelight::library {

/**
 * Teaches rcheevos to read CHD.
 *
 * Its bundled reader dispatches cue, gdi and then raw bin, so a CHD is read as though it were
 * uncompressed and no disc inside one is ever identified. This installs a reader that opens CHD
 * through libchdr and hands every other format back to the original.
 *
 * Safe to call more than once; only the first call installs
 */
void installChdCdReader();

} // namespace firelight::library
