#pragma once

#include <firelight/libretro/audio_output.hpp>
#include <firelight/libretro/retropad_provider.hpp>
#include <firelight/media/stream_decoder.hpp>
#include <firelight/netplay/messages.hpp>
#include <firelight/netplay/session.hpp>

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace firelight::netplay {

// The guest side of the game stream: decodes what the host sends (video
// frames go to the attached view, audio to a local output) and ticks the
// guest's own controller state back to the host. Packets arrive on network
// threads; the input tick runs its own thread
class GuestStreamReceiver {
public:
  GuestStreamReceiver(
      NetplaySession &session,
      std::function<std::shared_ptr<IAudioOutput>()> audioFactory,
      libretro::IRetropadProvider *localPads);
  ~GuestStreamReceiver();

  // Where decoded frames go (the stream view); called on decode threads
  void setFrameSink(std::function<void(media::StreamVideoFrame)> sink);

  void configure(const StreamConfig &config);
  void startPlaying(int platformId);
  void stopPlaying();
  [[nodiscard]] bool playing() const { return m_playing.load(); }

  void onVideoPacket(std::span<const uint8_t> data);
  void onAudioPacket(std::span<const uint8_t> data);

private:
  void inputTickLoop();

  NetplaySession &m_session;
  std::function<std::shared_ptr<IAudioOutput>()> m_audioFactory;
  libretro::IRetropadProvider *m_localPads = nullptr;

  media::StreamDecoder m_decoder;
  std::mutex m_sinkMutex;
  std::function<void(media::StreamVideoFrame)> m_frameSink;

  std::shared_ptr<IAudioOutput> m_audioOutput;
  std::mutex m_audioMutex;

  std::atomic<bool> m_playing{false};
  std::atomic<int> m_platformId{0};
  std::atomic<int> m_inputTickHz{60};
  std::atomic<uint32_t> m_lastVideoPtsMs{0};
  std::atomic<uint32_t> m_inputSeq{0};
  std::thread m_inputThread;
};

} // namespace firelight::netplay
