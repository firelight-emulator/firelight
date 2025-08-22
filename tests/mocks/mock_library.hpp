#pragma once
#include <gmock/gmock-function-mocker.h>
#include <library/user_library.hpp>

class MockLibrary final : public firelight::library::IUserLibrary {
public:
  MOCK_METHOD(bool, create, (firelight::library::RomFileInfo &), (override));
  MOCK_METHOD(void, create, (firelight::library::PatchFile &), (override));
  MOCK_METHOD(bool, create, (firelight::library::FolderInfo &), (override));
  MOCK_METHOD(bool, create, (firelight::library::FolderEntryInfo &),
              (override));
  MOCK_METHOD(bool, create, (firelight::library::WatchedDirectory &),
              (override));

  MOCK_METHOD(bool, update, (firelight::library::FolderInfo &), (override));
  MOCK_METHOD(std::vector<firelight::library::FolderInfo>, listFolders,
              (firelight::library::FolderInfo), (override));
  MOCK_METHOD(bool, deleteFolder, (int), (override));

  MOCK_METHOD(bool, deleteFolderEntry, (firelight::library::FolderEntryInfo &),
              (override));

  MOCK_METHOD(bool, update, (firelight::library::Entry &), (override));
  MOCK_METHOD(bool, update, (const firelight::library::WatchedDirectory &),
              (override));

  MOCK_METHOD(bool, deleteContentDirectory, (int), (override));

  MOCK_METHOD(std::optional<firelight::library::RomFileInfo>,
              getRomFileWithPathAndSize, (const QString &, size_t, bool),
              (override));

  MOCK_METHOD(std::vector<firelight::library::RomFileInfo>, getRomFiles, (),
              (override));

  MOCK_METHOD(std::optional<firelight::library::RomFileInfo>, getRomFile, (int),
              (override));
  MOCK_METHOD(bool, deleteRomFile, (int), (override));

  MOCK_METHOD(std::optional<firelight::library::PatchFile>,
              getPatchFileWithPathAndSize, (const QString &, size_t, bool),
              (override));

  MOCK_METHOD(std::optional<firelight::library::PatchFile>, getPatchFile, (int),
              (override));

  MOCK_METHOD(std::vector<firelight::library::PatchFile>, getPatchFiles, (),
              (override));

  MOCK_METHOD(std::vector<firelight::library::Entry>, getEntries, (int, int),
              (override));

  MOCK_METHOD(std::optional<firelight::library::Entry>, getEntry, (int),
              (override));

  MOCK_METHOD(std::optional<firelight::library::Entry>, getEntryWithContentHash,
              (const QString &), (override));

  MOCK_METHOD(std::vector<firelight::library::RunConfiguration>,
              getRunConfigurations, (const QString &), (override));

  MOCK_METHOD(void, doStuffWithRunConfigurations, (), (override));

  MOCK_METHOD(std::vector<firelight::library::RomFileInfo>,
              getRomFilesWithContentHash, (const QString &), (override));

  MOCK_METHOD(std::vector<firelight::library::WatchedDirectory>,
              getWatchedDirectories, (), (override));
};