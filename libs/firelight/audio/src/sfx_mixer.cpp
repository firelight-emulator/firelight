#include <firelight/audio/sfx_mixer.hpp>

#include <algorithm>
#include <cmath>

namespace firelight::audio {

void SfxMixer::configure(const int sampleRate, const int channelCount) {
  m_sampleRate = sampleRate;
  m_channelCount = channelCount;

  m_voices.fill(Voice{});
  m_activeVoiceCount.store(0, std::memory_order_relaxed);
}

int SfxMixer::getSampleRate() const { return m_sampleRate; }

int SfxMixer::getChannelCount() const { return m_channelCount; }

int SfxMixer::addClip(std::vector<float> samples) {
  const auto count = m_clipCount.load(std::memory_order_relaxed);

  if (count >= MAX_CLIPS || m_channelCount <= 0) {
    return -1;
  }

  m_clips[count].frameCount = samples.size() / static_cast<size_t>(m_channelCount);
  m_clips[count].samples = std::move(samples);

  // TODO
  // Publishes the samples to the rendering thread; it only ever indexes below
  // the count it reads here
  m_clipCount.store(count + 1, std::memory_order_release);

  return count;
}

void SfxMixer::clearClips() {
  m_clipCount.store(0, std::memory_order_release);

  for (auto &clip : m_clips) {
    clip.samples.clear();
    clip.frameCount = 0;
  }

  m_voices.fill(Voice{});
  m_activeVoiceCount.store(0, std::memory_order_relaxed);
}

int SfxMixer::getClipCount() const { return m_clipCount.load(std::memory_order_acquire); }

bool SfxMixer::play(const int clipId, const float gain, const int maxVoices, const float pitch) {
  if (clipId < 0 || clipId >= m_clipCount.load(std::memory_order_acquire)) {
    return false;
  }

  Command command;
  command.kind = Command::Kind::Play;
  command.clipId = static_cast<int16_t>(clipId);
  command.maxVoices = static_cast<int16_t>(std::clamp(maxVoices, 1, MAX_VOICES));
  command.gain = gain;
  command.pitch = pitch > 0.0F ? pitch : 1.0F;

  return pushCommand(command);
}

bool SfxMixer::stop(const int clipId) {
  if (clipId < 0) {
    return false;
  }

  Command command;
  command.kind = Command::Kind::Stop;
  command.clipId = static_cast<int16_t>(clipId);

  return pushCommand(command);
}

bool SfxMixer::pushCommand(const Command &command) {
  const auto write = m_writeIndex.load(std::memory_order_relaxed);
  const auto next = (write + 1) % COMMAND_CAPACITY;

  // TODO
  // One slot is left unused so a full ring is distinguishable from an empty one
  if (next == m_readIndex.load(std::memory_order_acquire)) {
    m_droppedCommands.fetch_add(1, std::memory_order_relaxed);
    return false;
  }

  m_commands[write] = command;
  m_writeIndex.store(next, std::memory_order_release);

  return true;
}

void SfxMixer::discardPendingCommands() {
  m_readIndex.store(m_writeIndex.load(std::memory_order_acquire), std::memory_order_release);
}

int SfxMixer::acquireVoice(const int clipId, const int maxVoices) {
  auto free = -1;
  auto matching = 0;
  auto oldestOfClip = -1;
  auto oldestOverall = -1;

  for (auto i = 0; i < MAX_VOICES; ++i) {
    const auto &voice = m_voices[i];

    if (voice.clipId < 0) {
      if (free < 0) {
        free = i;
      }

      continue;
    }

    if (oldestOverall < 0 || voice.serial < m_voices[oldestOverall].serial) {
      oldestOverall = i;
    }

    if (voice.clipId != clipId || voice.fadeRemaining >= 0) {
      continue;
    }

    matching++;

    if (oldestOfClip < 0 || voice.serial < m_voices[oldestOfClip].serial) {
      oldestOfClip = i;
    }
  }

  if (matching >= maxVoices && oldestOfClip >= 0) {
    return oldestOfClip;
  }

  if (free >= 0) {
    return free;
  }

  return oldestOverall;
}

void SfxMixer::drainCommands() {
  auto read = m_readIndex.load(std::memory_order_relaxed);
  const auto write = m_writeIndex.load(std::memory_order_acquire);

  while (read != write) {
    const auto &command = m_commands[read];

    if (command.kind == Command::Kind::Stop) {
      for (auto &voice : m_voices) {
        if (voice.clipId == command.clipId && voice.fadeRemaining < 0) {
          voice.fadeRemaining = STOP_FADE_FRAMES;
        }
      }
    } else if (command.clipId < m_clipCount.load(std::memory_order_acquire)) {
      const auto slot = acquireVoice(command.clipId, command.maxVoices);

      if (slot >= 0) {
        auto &voice = m_voices[slot];
        voice.clipId = command.clipId;
        voice.position = 0.0;
        voice.step = command.pitch;
        voice.gain = command.gain;
        voice.serial = m_nextSerial++;
        voice.fadeRemaining = -1;
      }
    }

    read = (read + 1) % COMMAND_CAPACITY;
  }

  m_readIndex.store(read, std::memory_order_release);
}

void SfxMixer::renderVoice(Voice &voice, float *output, const size_t frameCount) {
  const auto &clip = m_clips[voice.clipId];
  const auto channels = static_cast<size_t>(m_channelCount);

  for (size_t frame = 0; frame < frameCount; ++frame) {
    const auto index = static_cast<size_t>(voice.position);

    if (index >= clip.frameCount) {
      voice.clipId = -1;
      return;
    }

    auto gain = voice.gain;

    if (voice.fadeRemaining >= 0) {
      voice.fadeRemaining--;

      if (voice.fadeRemaining <= 0) {
        voice.clipId = -1;
        return;
      }

      gain *= static_cast<float>(voice.fadeRemaining) / static_cast<float>(STOP_FADE_FRAMES);
    }

    // TODO
    // Reading at a fractional position is what makes pitch adjustable, so the
    // sample either side of it is blended
    const auto fraction = static_cast<float>(voice.position - static_cast<double>(index));
    const auto hasNext = index + 1 < clip.frameCount;

    for (size_t channel = 0; channel < channels; ++channel) {
      const auto current = clip.samples[index * channels + channel];
      const auto next = hasNext ? clip.samples[(index + 1) * channels + channel] : 0.0F;

      output[frame * channels + channel] += (current + (next - current) * fraction) * gain;
    }

    voice.position += voice.step;
  }
}

void SfxMixer::render(float *output, const size_t frameCount) {
  drainCommands();

  if (output == nullptr || m_channelCount <= 0) {
    return;
  }

  std::fill_n(output, frameCount * static_cast<size_t>(m_channelCount), 0.0F);

  if (frameCount == 0) {
    m_activeVoiceCount.store(0, std::memory_order_relaxed);
    return;
  }

  const auto clipCount = m_clipCount.load(std::memory_order_acquire);
  auto active = 0;

  for (auto &voice : m_voices) {
    if (voice.clipId < 0 || voice.clipId >= clipCount) {
      voice.clipId = -1;
      continue;
    }

    renderVoice(voice, output, frameCount);

    if (voice.clipId >= 0) {
      active++;
    }
  }

  m_activeVoiceCount.store(active, std::memory_order_relaxed);

  const auto sampleCount = frameCount * static_cast<size_t>(m_channelCount);

  for (size_t i = 0; i < sampleCount; ++i) {
    output[i] = std::clamp(output[i], -1.0F, 1.0F);
  }
}

int SfxMixer::getActiveVoiceCount() const { return m_activeVoiceCount.load(std::memory_order_relaxed); }

uint64_t SfxMixer::getDroppedCommandCount() const { return m_droppedCommands.load(std::memory_order_relaxed); }

} // namespace firelight::audio
