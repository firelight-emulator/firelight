#pragma once

namespace firelight::library {
    class PatchFile {
    public:
        int m_id = 0;
        std::string m_filePath;
        int64_t m_fileSize = 0;
        std::string m_fileMd5;
        std::string m_fileCrc32;
        std::string m_targetFileMd5;
        std::string m_patchedMd5;
        std::string m_patchedContentHash;
        bool m_inArchive = false;
        std::string m_archiveFilePath;
    };
}