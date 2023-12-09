//
// Created by alexs on 11/7/2023.
//

#ifndef FIRELIGHT_BBOX_HPP
#define FIRELIGHT_BBOX_HPP

namespace FL::Math {

class Box {
public:
  Box();
  explicit Box(int x, int y, int w, int h);
  int xPx = 0;
  int yPx = 0;
  int widthPx = 0;
  int heightPx = 0;
};

} // namespace FL::Math

#endif // FIRELIGHT_BBOX_HPP
