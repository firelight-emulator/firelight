//
// Created by alexs on 2/16/2024.
//

#ifndef USERDATA_DATABASE_HPP
#define USERDATA_DATABASE_HPP
#include <string>

class IUserdataDatabase {
protected:
  ~IUserdataDatabase() = default;

public:
  virtual void savePlaySession(std::string romMd5, long long startTime,
                               long long endTime,
                               long long unpausedDurationSeconds) = 0;
};

#endif // USERDATA_DATABASE_HPP
