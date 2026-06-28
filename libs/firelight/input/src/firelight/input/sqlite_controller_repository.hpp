#pragma once

#include <QSqlDatabase>
#include <memory>

#include <firelight/input/controller_repository.hpp>
#include <firelight/input/gamepad_profile.hpp>

namespace firelight::input {
class SqliteControllerRepository final : public IControllerRepository {
public:
  explicit SqliteControllerRepository(QString dbFilePath);

  ~SqliteControllerRepository() override = default;

  std::shared_ptr<GamepadProfile> getProfile(int id) override;
  std::shared_ptr<GamepadProfile> createProfile(std::string name) override;

  [[nodiscard]] std::optional<DeviceInfo>
  getDeviceInfo(DeviceIdentifier identifier) const override;

  void updateDeviceInfo(DeviceIdentifier identifier,
                        const DeviceInfo &info) override;

private:
  QString m_dbFilePath;

  [[nodiscard]] QSqlDatabase getDatabase() const;
  std::shared_ptr<InputMapping>
  getOrCreateMapping(int profileId, int platformId, int controllerTypeId);

  int m_keyboardProfileId;

  std::vector<std::shared_ptr<GamepadProfile>> m_profiles{};
  std::vector<std::shared_ptr<InputMapping>> m_inputMappings{};
};
} // namespace firelight::input
