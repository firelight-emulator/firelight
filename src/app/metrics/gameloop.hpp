//
// Created by alexs on 10/26/2023.
//

#ifndef FIRELIGHT_GAMELOOP_HPP
#define FIRELIGHT_GAMELOOP_HPP

#include <cstddef>
namespace FL {

class GameLoopMetrics {
public:
  /**
   * Record the amount of time spent doing work in one frame of the game
   * loop, where "work" is represented by the amount of time between the
   * start of a frame and the point that the frame buffer swap call is made.
   * @param durationMillis The number of milliseconds as defined above.
   */
  void recordFrameWorkDuration(double durationMillis);
  void resetFrameWorkDuration();
  void printAllStats() const;

private:
  double totalFrameWorkDurationMillis = 0;
  long numFrames = 0;
};

} // namespace FL

#endif // FIRELIGHT_GAMELOOP_HPP
