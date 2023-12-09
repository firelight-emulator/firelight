//
// Created by alexs on 10/26/2023.
//

#include "gameloop.hpp"
#include <cstdio>

namespace FL {
void GameLoopMetrics::recordFrameWorkDuration(double durationMillis) {
  totalFrameWorkDurationMillis += durationMillis;
  numFrames++;

  if (numFrames == 300) {
    printAllStats();
    resetFrameWorkDuration();
  }
}

void GameLoopMetrics::resetFrameWorkDuration() {
  totalFrameWorkDurationMillis = 0;
  numFrames = 0;
}

void GameLoopMetrics::printAllStats() const {
  printf("Average frame work duration: %fms\n",
         totalFrameWorkDurationMillis / numFrames);
}

} // namespace FL