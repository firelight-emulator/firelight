#pragma once

#include <string>

class IUserdataDatabase {
protected:
  ~IUserdataDatabase() = default;

public:
  virtual void savePlaySession(std::string romMd5, long long startTime,
                               long long endTime,
                               long long unpausedDurationSeconds) = 0;
};
