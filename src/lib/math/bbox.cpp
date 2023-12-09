//
// Created by alexs on 11/7/2023.
//

#include "bbox.hpp"

namespace FL::Math {

Box::Box() : xPx(0), yPx(0), widthPx(0), heightPx(0) {}

Box::Box(int x, int y, int w, int h)
    : xPx(x), yPx(y), widthPx(w), heightPx(h) {}
} // namespace FL::Math
