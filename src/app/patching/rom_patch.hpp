//
// Created by alexs on 1/11/2024.
//

#ifndef IROMPATCH_HPP
#define IROMPATCH_HPP
#include <cstdint>
#include <vector>

class IRomPatch {
public:
  virtual ~IRomPatch() = default;
  virtual std::vector<uint8_t>
  patchRom(const std::vector<uint8_t> &data) const = 0;
};

#endif // IROMPATCH_HPP
