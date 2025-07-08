#pragma once

#include <QString>

namespace firelight::library {
    struct RunConfiguration {
        static constexpr std::string TYPE_ROM = "rom";
        static constexpr std::string TYPE_PATCH = "patch";

        int id = -1;
        std::string type;
        std::string contentHash;
        int romId = -1;
        int patchId = -1;
        int64_t createdAt = 0;
    };
} // namespace firelight::library
