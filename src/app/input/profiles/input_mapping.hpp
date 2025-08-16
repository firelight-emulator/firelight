#pragma once
#include "input/gamepad_input.hpp"

#include <functional>
#include <map>
#include <optional>
#include <string>

namespace firelight::input {
class InputMapping {
public:
  explicit InputMapping(
      int id, int profileId, int platformId, int controllerType,
      std::function<void(InputMapping &)> syncCallback = nullptr);

  virtual ~InputMapping() = default;

  unsigned getId() const;

  unsigned getControllerProfileId() const;

  unsigned getPlatformId() const;

  unsigned getControllerType() const;

  void setId(unsigned id);

  void setControllerProfileId(unsigned controllerProfileId);

  void setPlatformId(unsigned platformId);

  void setControllerType(int controllerType);

  void addMapping(GamepadInput input, int mappedInput);

  std::optional<int> getMappedInput(GamepadInput input);

  std::map<GamepadInput, int> &getMappings();

  virtual void removeMapping(GamepadInput input);

  virtual std::string serialize();

  virtual void deserialize(const std::string &data);

  void sync();

private:
  std::function<void(InputMapping &)> m_syncCallback;
  int m_id = 0;
  int m_controllerProfileId = 0;
  int m_platformId = 0;
  int m_controllerType = 0;
  std::map<GamepadInput, int> m_mappings{};
};
} // namespace firelight::input
