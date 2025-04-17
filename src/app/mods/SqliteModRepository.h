#pragma once
#include "mod_repository.hpp"

#include <map>

namespace firelight::mods {

class SqliteModRepository final : public IModRepository {
public:
  SqliteModRepository();
  std::optional<ModInfo> getModInfo(int modId) override;

private:
  std::map<int, ModInfo> m_mods;
};

} // mods