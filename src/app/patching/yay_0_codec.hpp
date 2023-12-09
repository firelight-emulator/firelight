//
// Created by alexs on 11/25/2023.
//

#ifndef FIRELIGHT_YAY_0_CODEC_HPP
#define FIRELIGHT_YAY_0_CODEC_HPP

#include <cstdint>
#include <vector>
namespace FL::Patching {

class Yay0Codec {
public:
  std::vector<uint8_t> decompress(uint8_t *compressed);
};

} // namespace FL::Patching

#endif // FIRELIGHT_YAY_0_CODEC_HPP
