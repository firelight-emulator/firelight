#pragma once

namespace firelight::library {
    struct PatchEntry : Entry {
        PatchEntry() {
            itemType = PATCH;
        }

        int patchId = -1;
        int modId = -1;
    };
} // namespace firelight::library
