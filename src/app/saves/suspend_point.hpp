#pragma once
#include <cstdint>
#include <qbuffer.h>
#include <vector>
#include <QImage>
#include <QUrl>

struct SuspendPoint {
  std::vector<uint8_t> state;
  long long timestamp;
  QImage image;
};
