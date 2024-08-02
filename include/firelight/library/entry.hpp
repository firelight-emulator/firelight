#pragma once

#include <string>

namespace firelight::library {
    enum ItemType {
        ROM,
        PATCH
    };

    struct Entry {
        int id = -1;
        std::string displayName;
        std::string contentId;
        std::string fileMd5;
        std::string filePath;

        ItemType itemType;
        unsigned itemId;

        unsigned activeSaveSlot = 1;
        bool hidden = false;

        unsigned createdAt;
    };
} // namespace firelight::library
