#pragma once

#include <cstdint>
#include <string>

namespace firelight::library {
    struct ContentDirectory {
        int id = -1;
        std::string path;
        int numFiles = 0;
        int numContentFiles = 0;
        // Filesystem mtime of the directory (epoch ms). Persisted from the DB;
        // not currently read back, kept for future use
        uint64_t lastModifiedEpochMs = 0;
        bool recursive = true;
        unsigned createdAt = 0;
    };
} // namespace firelight::library
