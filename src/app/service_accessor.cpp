#include "service_accessor.hpp"

namespace firelight {

input::InputService *ServiceAccessor::s_inputService;
platforms::PlatformService *ServiceAccessor::s_platformService;

void ServiceAccessor::setInputService(input::InputService *service) {
  s_inputService = service;
}
void ServiceAccessor::setPlatformService(platforms::PlatformService *service) {
  s_platformService = service;
}
input::InputService *ServiceAccessor::getInputService() {
  return s_inputService;
}
platforms::PlatformService *ServiceAccessor::getPlatformService() {
  return s_platformService;
}
} // namespace firelight