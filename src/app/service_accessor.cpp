#include "service_accessor.hpp"

namespace firelight {

input::InputService *ServiceAccessor::s_inputService;
platforms::PlatformService *ServiceAccessor::s_platformService;
settings::SettingsService *ServiceAccessor::s_settingsService;

void ServiceAccessor::setInputService(input::InputService *service) {
  s_inputService = service;
}
void ServiceAccessor::setPlatformService(platforms::PlatformService *service) {
  s_platformService = service;
}
void ServiceAccessor::setSettingsService(settings::SettingsService *service) {
  s_settingsService = service;
}
input::InputService *ServiceAccessor::getInputService() {
  return s_inputService;
}
platforms::PlatformService *ServiceAccessor::getPlatformService() {
  return s_platformService;
}
settings::SettingsService *ServiceAccessor::getSettingsService() {
  return s_settingsService;
}
} // namespace firelight