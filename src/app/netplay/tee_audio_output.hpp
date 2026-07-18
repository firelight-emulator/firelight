#pragma once

#include "host_stream_sender.hpp"

#include <firelight/libretro/audio_output.hpp>

#include <memory>

namespace firelight::netplay {

// Wraps the real audio output and mirrors the core's PCM into the host stream
// sender (which no-ops unless a stream is armed). Also reports the core's
// sample rate so the encoder can resample to Opus's 48 kHz
class TeeAudioOutput final : public IAudioOutput {
public:
  TeeAudioOutput(std::shared_ptr<IAudioOutput> inner, HostStreamSender *tap) : m_inner(std::move(inner)), m_tap(tap) {}

  size_t receive(const int16_t *data, const size_t numFrames) override {
    if (m_tap) {
      m_tap->pushAudio(data, numFrames);
    }
    return m_inner ? m_inner->receive(data, numFrames) : numFrames;
  }

  void initialize(const double newFrequency) override {
    if (m_tap) {
      m_tap->setAudioSampleRate(static_cast<int>(newFrequency));
    }
    if (m_inner) {
      m_inner->initialize(newFrequency);
    }
  }

  void setMuted(const bool muted) override {
    if (m_inner) {
      m_inner->setMuted(muted);
    }
  }

  [[nodiscard]] bool isMuted() const override { return m_inner && m_inner->isMuted(); }

  void setPaused(const bool paused) override {
    if (m_inner) {
      m_inner->setPaused(paused);
    }
  }

  [[nodiscard]] float getBufferLevel() const override { return m_inner ? m_inner->getBufferLevel() : -1.0f; }

  void setPlaybackRateRatio(const double ratio) override {
    if (m_inner) {
      m_inner->setPlaybackRateRatio(ratio);
    }
  }

  void setDynamicRateControlEnabled(const bool enabled) override {
    if (m_inner) {
      m_inner->setDynamicRateControlEnabled(enabled);
    }
  }

private:
  std::shared_ptr<IAudioOutput> m_inner;
  HostStreamSender *m_tap = nullptr;
};

} // namespace firelight::netplay
