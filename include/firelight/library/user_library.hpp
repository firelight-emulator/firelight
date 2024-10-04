#pragma once

#include "entry.hpp"
#include "rom_file.hpp"

namespace firelight::library {
    class IUserLibrary {
    public:
        virtual ~IUserLibrary() = default;

        virtual void addRomFile(RomFile &path) = 0;

        virtual std::optional<RomFile> getRomFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                                                                 bool inArchive) = 0;

        virtual std::vector<Entry> getEntries(int offset, int limit) = 0;

        virtual std::optional<Entry> getEntry(int entryId) = 0;

        virtual std::vector<RomFile> getRomFilesWithContentHash(const QString &contentHash) = 0;
    };
}
