//
// Created by alexs on 12/19/2023.
//

#ifndef FIRELIGHT_VIDEO_DATA_RECEIVER_HPP
#define FIRELIGHT_VIDEO_DATA_RECEIVER_HPP

#include <cstddef>
class CoreVideoDataReceiver {
public:
  virtual void receive(const void *data, unsigned width, unsigned height,
                       size_t pitch) = 0;
};

#endif // FIRELIGHT_VIDEO_DATA_RECEIVER_HPP
