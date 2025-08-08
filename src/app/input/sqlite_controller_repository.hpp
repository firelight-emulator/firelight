#pragma once

#include <QSqlDatabase>
#include <memory>

#include "controller_repository.hpp"
#include "input2/gamepad_profile.hpp"

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

  // [[nodiscard]] std::shared_ptr<ControllerProfile>
  // getControllerProfile(int profileId) const override;
  //
  // [[nodiscard]] std::shared_ptr<ControllerProfile>
  // getControllerProfile(std::string name, int vendorId, int productId,
  //                      int productVersion) override;
  //
  // [[nodiscard]] std::shared_ptr<InputMapping>
  // getInputMapping(int mappingId) const override;
  //
  // [[nodiscard]] std::shared_ptr<InputMapping>
  // getInputMapping(int profileId, int platformId) override;

  std::shared_ptr<KeyboardMapping> getKeyboardMapping(int profileId,
                                                      int platformId) override;

private:
  QString m_dbFilePath;

  [[nodiscard]] QSqlDatabase getDatabase() const;
  std::shared_ptr<InputMapping>
  getOrCreateMapping(int profileId, int platformId, int controllerTypeId);

  int m_keyboardProfileId;

  std::vector<std::shared_ptr<GamepadProfile>> m_profiles{};
  std::vector<std::shared_ptr<InputMapping>> m_inputMappings{};

  // std::vector<std::shared_ptr<ControllerProfile>> m_profiles{};
  // std::vector<std::shared_ptr<InputMapping>> m_inputMappings{};
};
} // namespace firelight::input
// firelight
