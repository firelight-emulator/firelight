//
// Created by alexs on 11/25/2023.
//

#ifndef FIRELIGHT_UTILS_HPP
#define FIRELIGHT_UTILS_HPP

namespace FL::Util {
bool isSysBigEndian() {
  unsigned int num = 1;
  char *byte = (char *)&num;

  return (*byte != 1);
}
} // namespace FL::Util

#endif // FIRELIGHT_UTILS_HPP
