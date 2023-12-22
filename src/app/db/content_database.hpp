//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_CONTENT_DATABASE_HPP
#define FIRELIGHT_CONTENT_DATABASE_HPP

#include "daos/rom.hpp"

#include <memory>
#include <optional>

class ContentDatabase {
public:
  virtual ~ContentDatabase() = default;
  virtual std::optional<ROM> getRomByMd5(const std::string &md5) = 0;
};

#endif // FIRELIGHT_CONTENT_DATABASE_HPP
