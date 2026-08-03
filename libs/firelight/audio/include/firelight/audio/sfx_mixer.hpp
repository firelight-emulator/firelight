#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace firelight::audio {

/**
 * Sums short UI sound clips into a single interleaved float32 stream.
 *
 * Clips are registered up front and are immutable afterwards. Play requests cross from the calling
 * thread to the rendering thread through a lock-free single-producer/single-consumer ring, so
 * render() never blocks and never allocates and is safe to call from a realtime audio callback.
 */
class SfxMixer {
public:
  /** The largest number of distinct clips that can be registered */
  static constexpr int MAX_CLIPS = 32;

  /** The largest number of voices that can sound simultaneously across every clip */
  static constexpr int MAX_VOICES = 24;

  /** How many unhandled play/stop requests can queue before further ones are dropped */
  static constexpr int COMMAND_CAPACITY = 128;

  /** How many frames a voice is faded over when it is stopped or taken over */
  static constexpr int STOP_FADE_FRAMES = 64;

  SfxMixer() = default;

  ~SfxMixer() = default;

  SfxMixer(const SfxMixer &) = delete;

  SfxMixer &operator=(const SfxMixer &) = delete;

  /**
   * Sets the rate and channel count render() produces. Every registered clip must already be in
   * this format. Only legal while nothing is calling render()
   */
  void configure(int sampleRate, int channelCount);

  /**
   * @return The rate render() produces, in Hz
   */
  [[nodiscard]] int getSampleRate() const;

  /**
   * @return The channel count render() produces
   */
  [[nodiscard]] int getChannelCount() const;

  /**
   * Takes ownership of one clip's samples. Safe to call while render() is running: the clip table
   * is fixed-size so publishing one cannot move another, and the rendering thread only reads
   * indices below the count this publishes
   *
   * @param samples Interleaved float32 at the configured rate and channel count
   * @return The clip's id, or -1 if MAX_CLIPS are already registered or the mixer is unconfigured
   */
  int addClip(std::vector<float> samples);

  /**
   * Drops every clip and silences every voice. Only legal while nothing is calling render()
   */
  void clearClips();

  /**
   * @return How many clips are registered
   */
  [[nodiscard]] int getClipCount() const;

  /**
   * Queues a request to start clipId on a new voice. Never blocks, never allocates
   *
   * @param clipId An id returned by addClip
   * @param gain Linear gain applied to the clip's samples
   * @param maxVoices How many voices of this same clip may sound at once before the oldest of them
   *   is taken over
   * @param pitch Playback rate multiplier; 1.0 plays the clip at its recorded pitch
   * @return false if the command ring was full and the request was dropped
   */
  bool play(int clipId, float gain, int maxVoices, float pitch = 1.0F);

  /**
   * Queues a request to fade out every voice currently playing clipId. Never blocks, never
   * allocates
   *
   * @return false if the command ring was full and the request was dropped
   */
  bool stop(int clipId);

  /**
   * Drops every queued play and stop request without acting on it.
   *
   * Requests only take effect when render() drains them, so any that pile up while nothing is
   * rendering would otherwise all start in the first buffer that follows — several copies of the
   * same sound at the same position, summing in phase. Only legal while nothing is calling render()
   */
  void discardPendingCommands();

  /**
   * Writes exactly frameCount * getChannelCount() interleaved float32 samples, overwriting whatever
   * output held. Drains pending commands first. Never blocks, never allocates
   */
  void render(float *output, size_t frameCount);

  /**
   * @return How many voices were sounding at the end of the last render()
   */
  [[nodiscard]] int getActiveVoiceCount() const;

  /**
   * @return How many requests have been dropped because the command ring was full
   */
  [[nodiscard]] uint64_t getDroppedCommandCount() const;

private:
  /** One queued play or stop request */
  struct Command {
    enum class Kind : uint8_t { Play, Stop };

    Kind kind = Kind::Play;
    int16_t clipId = -1;
    int16_t maxVoices = 0;
    float gain = 1.0F;
    float pitch = 1.0F;
  };

  /** One clip's immutable samples */
  struct Clip {
    std::vector<float> samples;
    size_t frameCount = 0;
  };

  /** One sounding instance of a clip */
  struct Voice {
    int clipId = -1;
    double position = 0.0;
    double step = 1.0;
    float gain = 1.0F;
    uint64_t serial = 0;
    int fadeRemaining = -1;
  };

  /**
   * Moves every queued command into the voice table. Called only from render()
   */
  void drainCommands();

  /**
   * Picks the voice slot a new play should take, fading out the oldest when a cap is reached
   *
   * @return The slot index, or -1 if none could be freed
   */
  int acquireVoice(int clipId, int maxVoices);

  /**
   * Adds one voice's contribution to the output buffer, advancing its read position
   */
  void renderVoice(Voice &voice, float *output, size_t frameCount);

  bool pushCommand(const Command &command);

  std::array<Clip, MAX_CLIPS> m_clips{};
  std::atomic<int> m_clipCount{0};

  std::array<Voice, MAX_VOICES> m_voices{};
  uint64_t m_nextSerial = 1;

  std::array<Command, COMMAND_CAPACITY> m_commands{};
  std::atomic<uint32_t> m_writeIndex{0};
  std::atomic<uint32_t> m_readIndex{0};
  std::atomic<uint64_t> m_droppedCommands{0};

  std::atomic<int> m_activeVoiceCount{0};

  int m_sampleRate = 0;
  int m_channelCount = 0;
};

} // namespace firelight::audio
