
#include "rom_file.hpp"

#include <QFile>
#include <archive.h>
#include <archive_entry.h>
#include <qcryptographichash.h>
#include <rcheevos/rc_hash.h>
#include <spdlog/spdlog.h>
#include <zlib.h>

#include "../platform_metadata.hpp"

namespace firelight::library {
RomFile::RomFile(const QString &path) : m_filePath(path) {
  m_suffix = path.right(path.length() - (path.lastIndexOf('.') + 1)).toLower();

  m_platformId =
      PlatformMetadata::getIdFromFileExtension(m_suffix.toStdString());
  if (m_platformId == PlatformMetadata::PLATFORM_ID_UNKNOWN) {
    return;
    // TODO: check for other stuff later.
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  const auto bytes = file.readAll();
  file.close();

  m_fileSizeBytes = bytes.size();
  m_fileMd5 =
      QCryptographicHash::hash(bytes, QCryptographicHash::Algorithm::Md5)
          .toHex();

  const auto crc = crc32(0L, nullptr, 0);
  m_fileCrc32 = QString::number(
      crc32(crc, reinterpret_cast<const Bytef *>(bytes.data()), bytes.size()));

  generateContentBytesAndHash(bytes);
  m_isValid = !m_contentHash.isEmpty();
}

RomFile::RomFile(const QString &filename, const char *data, const size_t size,
                 const QString &archivePathName)
    : m_filePath(filename) {
  if (!archivePathName.isEmpty()) {
    m_archivePathName = archivePathName;
    m_inArchive = true;
  }

  m_suffix = filename.right(filename.length() - (filename.lastIndexOf('.') + 1))
                 .toLower();

  m_platformId =
      PlatformMetadata::getIdFromFileExtension(m_suffix.toStdString());
  if (m_platformId == PlatformMetadata::PLATFORM_ID_UNKNOWN) {
    // TODO: check for other stuff later.
  }

  const auto bytes = QByteArray::fromRawData(data, size);

  m_fileSizeBytes = bytes.size();
  m_fileMd5 =
      QCryptographicHash::hash(bytes, QCryptographicHash::Algorithm::Md5)
          .toHex();

  const auto crc = crc32(0L, nullptr, 0);
  m_fileCrc32 =
      QString::number(crc32(crc, reinterpret_cast<const Bytef *>(data), size));

  generateContentBytesAndHash(bytes);
  m_isValid = !m_contentHash.isEmpty();
}

RomFile::RomFile(const QSqlQuery &query) {
  if (!query.isActive() || !query.isValid()) {
    return;
  }

  m_id = query.value(0).toInt();
  m_filePath = query.value(1).toString();
  m_fileSizeBytes = query.value(2).toUInt();
  m_fileMd5 = query.value(3).toString();
  m_fileCrc32 = query.value(4).toString();
  m_inArchive = query.value(5).toBool();
  m_archivePathName = query.value(6).toString();
  m_platformId = query.value(7).toInt();
  m_contentHash = query.value(8).toString();
}

void RomFile::load() {
  if (m_inArchive) {
    archive_entry *entry;

    archive *a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    int r = archive_read_open_filename(
        a, m_archivePathName.toStdString().c_str(), 10240);

    if (r != ARCHIVE_OK) {
      return;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
      if (m_filePath != archive_entry_pathname(entry)) {
        continue;
      }

      auto size = archive_entry_size(entry);
      if (size > 0) {
        auto buffer = new char[size];
        archive_read_data(a, buffer, size);

        QByteArray bytes(buffer, size);
        generateContentBytesAndHash(bytes);
        m_isValid = !m_contentHash.isEmpty();

        delete[] buffer;
      } else {
        archive_read_data_skip(a);
      }
    }
    r = archive_read_free(a);
    if (r != ARCHIVE_OK) {
      return;
    }

    return;
  }

  QFile file(m_filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  const auto bytes = file.readAll();
  file.close();

  generateContentBytesAndHash(bytes);
  m_isValid = !m_contentHash.isEmpty();
}

bool RomFile::isValid() const { return m_isValid; }
void RomFile::setId(const int id) { m_id = id; }
int RomFile::getId() { return m_id; }

QByteArray RomFile::getContentBytes() { return m_contentBytes; }

QString RomFile::getContentHash() { return m_contentHash; }

QString RomFile::getFileMd5() { return m_fileMd5; }

QString RomFile::getFileCrc32() { return m_fileCrc32; }

QString RomFile::getFilePath() { return m_filePath; }

size_t RomFile::getFileSizeBytes() const { return m_fileSizeBytes; }

bool RomFile::inArchive() const { return m_inArchive; }

QString RomFile::getArchivePathName() { return m_archivePathName; }

int RomFile::getPlatformId() const { return m_platformId; }

void RomFile::applyPatchToFullBytes(const PatchFile &patchFile) {}
void RomFile::applyPatchToContentBytes(const PatchFile &patchFile) {
  if (m_contentBytes.isEmpty()) {
    spdlog::warn("No content bytes to apply patch to");
    return;
  }

  std::vector<uint8_t> bytes;
  bytes.resize(m_contentBytes.size());
  std::ranges::copy(m_contentBytes, bytes.begin());

  const auto patched = patchFile.patch(bytes);
  const QByteArray qBytes(reinterpret_cast<const char *>(patched.data()),
                          patched.size());

  generateContentBytesAndHash(qBytes);

  spdlog::warn("Applying patch file {}", patchFile.m_filePath);
}

void RomFile::generateContentBytesAndHash(const QByteArray &fileBytes) {
  // if (!m_contentBytes.isEmpty() && !m_contentHash.isEmpty()) {
  //   return;
  // }

  switch (m_platformId) {
  case PlatformMetadata::PLATFORM_ID_NES:
    m_contentBytes = fileBytes;

    // Disregard the header for hashing but keep it for the content.
    if (fileBytes.startsWith("NES\x1A")) {
      m_contentHash = QCryptographicHash::hash(
                          fileBytes.mid(16), QCryptographicHash::Algorithm::Md5)
                          .toHex();
    } else {
      m_contentHash = QCryptographicHash::hash(
                          fileBytes, QCryptographicHash::Algorithm::Md5)
                          .toHex();
    }
    return;
  case PlatformMetadata::PLATFORM_ID_SNES:
    m_contentBytes = fileBytes;

    if (fileBytes.size() % 1024 == 512) {
      m_contentHash =
          QCryptographicHash::hash(fileBytes.mid(512),
                                   QCryptographicHash::Algorithm::Md5)
              .toHex();
    } else {
      m_contentHash = QCryptographicHash::hash(
                          fileBytes, QCryptographicHash::Algorithm::Md5)
                          .toHex();
    }
    return;
  case PlatformMetadata::PLATFORM_ID_N64: {
    static const QByteArray Z64_SIGNATURE = "\x80\x37\x12\x40";
    static const QByteArray V64_SIGNATURE = "\x37\x80\x40\x12";
    static const QByteArray N64_SIGNATURE = "\x40\x12\x37\x80";

    if (fileBytes.startsWith(V64_SIGNATURE)) {
      QByteArray other{};
      other.resize(fileBytes.length());

      for (int i = 0; i < fileBytes.length(); i += 2) {
        other[i] = fileBytes[i + 1];
        other[i + 1] = fileBytes[i];
      }
      m_contentHash =
          QCryptographicHash::hash(other, QCryptographicHash::Algorithm::Md5)
              .toHex();
      m_contentBytes = other;
    } else if (fileBytes.startsWith(N64_SIGNATURE)) {
      QByteArray other{};
      other.resize(fileBytes.length());

      for (int i = 0; i < fileBytes.length(); i += 4) {
        other[i] = fileBytes[i + 3];
        other[i + 1] = fileBytes[i + 2];
        other[i + 2] = fileBytes[i + 1];
        other[i + 3] = fileBytes[i];
      }
      m_contentHash =
          QCryptographicHash::hash(other, QCryptographicHash::Algorithm::Md5)
              .toHex();
      m_contentBytes = other;
    } else if (fileBytes.startsWith(Z64_SIGNATURE)) {
      m_contentHash = QCryptographicHash::hash(
                          fileBytes, QCryptographicHash::Algorithm::Md5)
                          .toHex();
      m_contentBytes = fileBytes;
    }
    return;
  }
  case PlatformMetadata::PLATFORM_ID_GAMEBOY:
  case PlatformMetadata::PLATFORM_ID_GAMEBOY_COLOR:
  case PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE:
  case PlatformMetadata::PLATFORM_ID_SEGA_GENESIS:
  case PlatformMetadata::PLATFORM_ID_SEGA_GAMEGEAR:
  case PlatformMetadata::PLATFORM_ID_SEGA_MASTER_SYSTEM:
    m_contentHash =
        QCryptographicHash::hash(fileBytes, QCryptographicHash::Algorithm::Md5)
            .toHex();
    m_contentBytes = fileBytes;
    return;
  default:
    auto rcConsoleId = RC_CONSOLE_UNKNOWN;
    switch (m_platformId) {
    case 1:
      rcConsoleId = RC_CONSOLE_GAMEBOY;
      break;
    case 2:
      rcConsoleId = RC_CONSOLE_GAMEBOY_COLOR;
      break;
    case 3:
      rcConsoleId = RC_CONSOLE_GAMEBOY_ADVANCE;
      break;
    case 4:
      rcConsoleId = RC_CONSOLE_VIRTUAL_BOY;
      break;
    case 5:
      rcConsoleId = RC_CONSOLE_NINTENDO;
      break;
    case 6:
      rcConsoleId = RC_CONSOLE_SUPER_NINTENDO;
      break;
    case 7:
      rcConsoleId = RC_CONSOLE_NINTENDO_64;
      break;
    case 10:
      rcConsoleId = RC_CONSOLE_NINTENDO_DS;
      break;
    case 12:
      rcConsoleId = RC_CONSOLE_MASTER_SYSTEM;
      break;
    case 13:
      rcConsoleId = RC_CONSOLE_MEGA_DRIVE;
      break;
    case 14:
      rcConsoleId = RC_CONSOLE_GAME_GEAR;
      break;
    case 15:
      rcConsoleId = RC_CONSOLE_SATURN;
      break;
    case 16:
      rcConsoleId = RC_CONSOLE_SEGA_32X;
      break;
    case 17:
      rcConsoleId = RC_CONSOLE_SEGA_CD;
      break;
    case 18:
      rcConsoleId = RC_CONSOLE_PLAYSTATION;
      break;
    case 19:
      rcConsoleId = RC_CONSOLE_PLAYSTATION_2;
      break;
    case 20:
      rcConsoleId = RC_CONSOLE_PSP;
      break;
    case PlatformMetadata::PLATFORM_ID_SUPERGRAFX:
    case PlatformMetadata::PLATFORM_ID_TURBOGRAFX16:
      rcConsoleId = RC_CONSOLE_PC_ENGINE;
      break;
    case PlatformMetadata::PLATFORM_ID_POKEMON_MINI:
      rcConsoleId = RC_CONSOLE_POKEMON_MINI;
      break;
    default:
      rcConsoleId = RC_CONSOLE_UNKNOWN;
    }

    char hash[33];
    rc_hash_generate_from_buffer(
        hash, rcConsoleId, reinterpret_cast<const uint8_t *>(fileBytes.data()),
        fileBytes.size());

    m_contentHash = QByteArray::fromHex(hash).toHex();
    m_contentBytes = fileBytes;
  }
}
} // namespace firelight::library
