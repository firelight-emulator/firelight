#pragma once

namespace firelight::library {
    struct RomEntry : Entry {
        RomEntry() {
            itemType = ROM;
        }

        int romId = -1;
        int gameId = -1;
    };
} // namespace firelight::library
