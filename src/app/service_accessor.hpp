#pragma once

namespace firelight {
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

protected:
  static input::InputService *getInputService();
  static platforms::PlatformService *getPlatformService();

private:
  static input::InputService *s_inputService;
  static platforms::PlatformService *s_platformService;
};

} // namespace firelight
