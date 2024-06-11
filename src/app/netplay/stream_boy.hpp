#pragma once
#include <qimage.h>

namespace firelight::netplay {
class IStreamBoy {
protected:
  ~IStreamBoy() = default;

private:
  virtual void takeImage(QImage image) = 0;
  virtual void doIt() = 0;
};
} // namespace firelight::netplay