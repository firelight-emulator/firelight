#pragma once

#include "types.h"

namespace discord {

class StorageManager final {
public:
    ~StorageManager() = default;

    Result Read(char const* name,
                uint8_t* data,
                uint32_t dataLength,
                uint32_t* read);
    void ReadAsync(char const* name,
                   std::function<void(Result, uint8_t*, uint32_t)> callback);
    void ReadAsyncPartial(char const* name,
                          uint64_t offset,
                          uint64_t length,
                          std::function<void(Result, uint8_t*, uint32_t)> callback);
    Result Write(char const* name, uint8_t* data, uint32_t dataLength);
    void WriteAsync(char const* name,
                    uint8_t* data,
                    uint32_t dataLength,
                    std::function<void(Result)> callback);
    Result Delete(char const* name);
    Result Exists(char const* name, bool* exists);
    void Count(int32_t* count);
    Result Stat(char const* name, FileStat* stat);
    Result StatAt(int32_t index, FileStat* stat);
    Result GetPath(char path[4096]);

private:
    friend class Core;

    StorageManager() = default;
    StorageManager(StorageManager const& rhs) = delete;
    StorageManager& operator=(StorageManager const& rhs) = delete;
    StorageManager(StorageManager&& rhs) = delete;
    StorageManager& operator=(StorageManager&& rhs) = delete;

    IDiscordStorageManager* internal_;
    static IDiscordStorageEvents events_;
};

} // namespace discord
