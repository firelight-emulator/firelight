//
// Created by alexs on 12/7/2023.
//

#ifndef FIRELIGHT_SOURCE_HPP
#define FIRELIGHT_SOURCE_HPP

#include <cstdint>
namespace FL::Audio {

class Source {
public:
  virtual void play() = 0;
  virtual void pause() = 0;
  virtual void queueSamples(int8_t *data, int n) = 0;
  virtual void setSampleRate(int rate) = 0;
  virtual void setAudioFormat() = 0;
};

} // namespace FL::Audio

#endif // FIRELIGHT_SOURCE_HPP
