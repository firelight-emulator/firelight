#pragma once
#include <cstdint>
#include <qbuffer.h>
#include <vector>
#include <string>
#include <QImage>
#include <QUrl>

struct SuspendPoint {
  std::string contentHash;
  std::vector<uint8_t> state;
  long long timestamp;
  QImage image;
  bool locked = false;
};
