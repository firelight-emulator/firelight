#pragma once

namespace firelight {
namespace settings {
class SettingsService;
}
namespace platforms {
class PlatformService;
}
namespace input {
class InputService;
}

class ServiceAccessor {
public:
  static void setInputService(input::InputService *service);
  static void setPlatformService(platforms::PlatformService *service);
  static void setSettingsService(settings::SettingsService *service);

protected:
  static input::InputService *getInputService();
  static platforms::PlatformService *getPlatformService();
  static settings::SettingsService *getSettingsService();

private:
  static input::InputService *s_inputService;
  static platforms::PlatformService *s_platformService;
  static settings::SettingsService *s_settingsService;
};

} // namespace firelight
