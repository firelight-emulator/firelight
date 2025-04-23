#pragma once
#include <QImage>
#include <QUrl>
#include <cstdint>
#include <qbuffer.h>
#include <string>
#include <vector>

struct SuspendPoint {
  std::string contentHash;
  std::vector<uint8_t> state;
  std::vector<uint8_t> retroachievementsState;
  long long timestamp;
  QImage image;
  bool locked = false;
  int saveSlotNumber;
};
