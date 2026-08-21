#include "ui_sound_player.hpp"

#include "audio_device_selection.hpp"
#include "audio_settings.hpp"

#include <firelight/audio/pcm_converter.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QFile>
#include <SDL2/SDL_audio.h>
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

namespace firelight::audio {

namespace {
// TODO
// Device enumeration arrives in bursts when a device appears, so the reopen is
// deferred until the burst settles
constexpr int REOPEN_DEBOUNCE_MS = 250;

// TODO
// An output that has genuinely gone would otherwise be reopened over and over. Each failure waits
// longer, up to this
constexpr int MAX_REOPEN_BACKOFF_MS = 10000;

constexpr int FALLBACK_SAMPLE_RATE = 48000;
constexpr int FALLBACK_CHANNEL_COUNT = 2;

// How many frames are converted at a time on a device that does not take float
constexpr size_t SCRATCH_FRAMES = 2048;

// TODO
// Keeping a stream open stops the audio path from being torn down, but some
// output hardware powers its analogue side down when it sees only digital
// zeroes and swallows the whole of the next sound while it wakes. Broadband
// noise is used rather than an alternating sample, which would sit at the
// Nyquist frequency and be filtered back out by the converter
constexpr float DEFAULT_KEEPALIVE_DIVISOR = 4096.0F;

// TODO
// Diagnostic override, as a divisor of full scale: FL_UI_KEEPALIVE=16384 is
// quieter, 1024 louder, 0 turns the noise off entirely
float keepAliveAmplitude() {
  auto divisor = DEFAULT_KEEPALIVE_DIVISOR;

  if (qEnvironmentVariableIsSet("FL_UI_KEEPALIVE")) {
    const auto configured = qEnvironmentVariableIntValue("FL_UI_KEEPALIVE");

    if (configured == 0) {
      return 0.0F;
    }

    if (configured > 0) {
      divisor = static_cast<float>(configured);
    }
  }

  return 1.0F / divisor;
}

float clampSample(const float sample) { return std::clamp(sample, -1.0F, 1.0F); }
} // namespace

UiSoundPlayer::UiSoundPlayer(settings::SettingsService &settingsService, QObject *parent)
    : QObject(parent), m_settingsService(settingsService) {
  m_keepAliveAmplitude = keepAliveAmplitude();
  spdlog::debug("UI sound keepalive noise at 1/{:.0f} of full scale",
                m_keepAliveAmplitude > 0.0F ? 1.0F / m_keepAliveAmplitude : 0.0F);

  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, &UiSoundPlayer::onAudioOutputsChanged);

  m_reopenTimer.setSingleShot(true);
  m_reopenBackoffMs = REOPEN_DEBOUNCE_MS;
  m_reopenTimer.setInterval(m_reopenBackoffMs);
  connect(&m_reopenTimer, &QTimer::timeout, this, [this] { reopen(); });

  const auto onKey = [this](const std::string &key) {
    if (key == OUTPUT_DEVICE_KEY) {
      m_reopenTimer.start();
    } else if (key == UI_SOUND_VOLUME_KEY) {
      refreshVolume();
    }
  };

  m_settingChangedConnection = EventDispatcher::instance().subscribe<settings::GlobalSettingChangedEvent>(
      [onKey](const settings::GlobalSettingChangedEvent &e) { onKey(e.key); });
  m_settingResetConnection = EventDispatcher::instance().subscribe<settings::GlobalSettingResetEvent>(
      [onKey](const settings::GlobalSettingResetEvent &e) { onKey(e.key); });

  refreshVolume();
  reopen();
}

void UiSoundPlayer::refreshVolume() {
  auto percent = 100;

  try {
    percent = std::stoi(m_settingsService.getGlobalValue(UI_SOUND_VOLUME_KEY).value_or("100"));
  } catch (const std::exception &) {
    percent = 100;
  }

  // A user-facing slider is expected to be perceptual, unlike the per-sound
  // volumes, which are linear amplitude and stay that way
  m_volume = QtAudio::convertVolume(std::clamp(percent, 0, 100) / 100.0F, QtAudio::LogarithmicVolumeScale,
                                    QtAudio::LinearVolumeScale);

  if (m_sink != nullptr) {
    m_sink->setVolume(m_volume);
  }
}

UiSoundPlayer::~UiSoundPlayer() { m_sink.reset(); }

QAudioFormat UiSoundPlayer::chooseFormat(const QAudioDevice &device) {
  if (device.isNull()) {
    return {};
  }

  const auto preferred = device.preferredFormat();

  if (device.isFormatSupported(preferred) &&
      (preferred.sampleFormat() == QAudioFormat::Float || preferred.sampleFormat() == QAudioFormat::Int16 ||
       preferred.sampleFormat() == QAudioFormat::Int32)) {
    return preferred;
  }

  QAudioFormat format;
  format.setSampleRate(preferred.sampleRate() > 0 ? preferred.sampleRate() : FALLBACK_SAMPLE_RATE);
  format.setChannelCount(preferred.channelCount() > 0 ? preferred.channelCount() : FALLBACK_CHANNEL_COUNT);

  for (const auto candidate : {QAudioFormat::Float, QAudioFormat::Int16, QAudioFormat::Int32}) {
    format.setSampleFormat(candidate);

    if (device.isFormatSupported(format)) {
      return format;
    }
  }

  return {};
}

void UiSoundPlayer::addKeepAlive(float *samples, const size_t frames, const size_t channels) {
  if (m_keepAliveAmplitude <= 0.0F) {
    return;
  }

  for (size_t frame = 0; frame < frames; ++frame) {
    m_keepAliveNoise = m_keepAliveNoise * 1664525U + 1013904223U;
    const auto normalized = static_cast<float>(static_cast<int32_t>(m_keepAliveNoise >> 8)) / 8388608.0F;
    const auto offset = normalized * m_keepAliveAmplitude;

    for (size_t channel = 0; channel < channels; ++channel) {
      samples[frame * channels + channel] += offset;
    }
  }
}

template <typename SampleType> void UiSoundPlayer::renderInto(QSpan<SampleType> buffer) {
  const auto channels = static_cast<size_t>(m_mixer.getChannelCount());

  if (channels == 0 || buffer.isEmpty()) {
    return;
  }

  // TODO
  // Confirms once that the backend really is pulling from the callback
  if (!m_callbackSeen.test_and_set(std::memory_order_relaxed)) {
    spdlog::debug("UI sound callback delivering {} frames", buffer.size() / channels);
  }

  if constexpr (std::is_same_v<SampleType, float>) {
    const auto frames = buffer.size() / channels;
    m_mixer.render(buffer.data(), frames);
    addKeepAlive(buffer.data(), frames, channels);

    return;
  } else {
    const auto totalFrames = static_cast<size_t>(buffer.size()) / channels;
    size_t done = 0;

    while (done < totalFrames) {
      const auto chunk = std::min(SCRATCH_FRAMES, totalFrames - done);
      m_mixer.render(m_scratch.data(), chunk);
      addKeepAlive(m_scratch.data(), chunk, channels);

      for (size_t i = 0; i < chunk * channels; ++i) {
        const auto sample = clampSample(m_scratch[i]);

        if constexpr (std::is_same_v<SampleType, int16_t>) {
          buffer[static_cast<qsizetype>(done * channels + i)] = static_cast<int16_t>(std::lroundf(sample * 32767.0F));
        } else {
          buffer[static_cast<qsizetype>(done * channels + i)] =
              static_cast<int32_t>(std::lround(static_cast<double>(sample) * 2147483647.0));
        }
      }

      done += chunk;
    }
  }
}

template <typename SampleType> void UiSoundPlayer::startSink(const QAudioDevice &device) {
  m_sink = std::make_unique<QAudioSink>(device, m_format);
  m_sink->setVolume(m_volume);

  // TODO
  // Queued so the handler never runs inside the sink's own emission, where the sink cannot be
  // destroyed
  connect(m_sink.get(), &QAudioSink::stateChanged, this, &UiSoundPlayer::onSinkStateChanged, Qt::QueuedConnection);

  m_sink->start([this](QSpan<SampleType> buffer) { renderInto(buffer); });

  if (m_sink->error() != QtAudio::NoError) {
    spdlog::warn("UI sound stream failed to start (error {}); UI sounds will be silent",
                 static_cast<int>(m_sink->error()));
    m_sink.reset();
    return;
  }

  // A stream is running again, so the next failure starts from the short wait
  m_reopenBackoffMs = REOPEN_DEBOUNCE_MS;
  m_reopenTimer.setInterval(m_reopenBackoffMs);
}

void UiSoundPlayer::reopen() {
  // Everything below mutates state the callback reads, so the callback has to be
  // gone first
  m_sink.reset();

  // Anything queued while there was no stream is stale by the time a new one
  // starts, and would all fire in its first buffer
  m_mixer.discardPendingCommands();

  const auto device = selectOutputDevice(m_settingsService);

  if (device.isNull()) {
    spdlog::warn("No audio output available; UI sounds will be silent");
    return;
  }

  m_format = chooseFormat(device);

  if (!m_format.isValid()) {
    spdlog::warn("No usable format on audio output '{}'; UI sounds will be silent", device.description().toStdString());
    return;
  }

  const auto rateChanged = m_mixer.getSampleRate() != m_format.sampleRate();
  const auto channelsChanged = m_mixer.getChannelCount() != m_format.channelCount();

  if (rateChanged || channelsChanged) {
    m_mixer.configure(m_format.sampleRate(), m_format.channelCount());
    rebuildClips();
  }

  m_scratch.assign(SCRATCH_FRAMES * static_cast<size_t>(m_format.channelCount()), 0.0F);
  m_deviceId = device.id();

  switch (m_format.sampleFormat()) {
  case QAudioFormat::Float:
    startSink<float>(device);
    break;
  case QAudioFormat::Int16:
    startSink<int16_t>(device);
    break;
  case QAudioFormat::Int32:
    startSink<int32_t>(device);
    break;
  default:
    spdlog::warn("Unsupported sample format for UI sounds; they will be silent");
    return;
  }

  spdlog::info("UI sound stream on '{}': {}Hz {}ch format {}", device.description().toStdString(),
               m_format.sampleRate(), m_format.channelCount(), static_cast<int>(m_format.sampleFormat()));
}

void UiSoundPlayer::rebuildClips() {
  m_mixer.clearClips();

  for (const auto &bytes : m_clipSources) {
    addClipFromBytes(bytes);
  }
}

int UiSoundPlayer::addClipFromBytes(const QByteArray &bytes) {
  SDL_AudioSpec spec;
  Uint8 *samples = nullptr;
  Uint32 length = 0;

  // SDL walks the RIFF chunk list and widens packed 24-bit to 32-bit for us, so
  // the assets' three rates, two channel counts and two bit depths all arrive in
  // a shape swresample takes directly
  if (SDL_LoadWAV_RW(SDL_RWFromConstMem(bytes.constData(), bytes.size()), 1, &spec, &samples, &length) == nullptr) {
    spdlog::warn("UI sound could not be decoded: {}", SDL_GetError());
    return -1;
  }

  PcmData decoded;
  decoded.sampleRate = spec.freq;
  decoded.channelCount = spec.channels;
  decoded.bytes.assign(samples, samples + length);
  SDL_FreeWAV(samples);

  switch (spec.format) {
  case AUDIO_U8:
    decoded.sampleType = PcmSampleType::UInt8;
    break;
  case AUDIO_S16LSB:
    decoded.sampleType = PcmSampleType::Int16;
    break;
  case AUDIO_S32LSB:
    decoded.sampleType = PcmSampleType::Int32;
    break;
  case AUDIO_F32LSB:
    decoded.sampleType = PcmSampleType::Float32;
    break;
  default:
    spdlog::warn("UI sound has an unsupported sample format ({})", spec.format);
    return -1;
  }

  auto converted = PcmConverter::convert(decoded, m_mixer.getSampleRate(), m_mixer.getChannelCount());

  if (!converted.has_value()) {
    return -1;
  }

  return m_mixer.addClip(std::move(*converted));
}

QString UiSoundPlayer::resolveResourcePath(const QUrl &source) {
  if (source.scheme() == "qrc") {
    const auto path = source.path();
    return path.startsWith('/') ? ":" + path : ":/" + path;
  }

  if (source.isLocalFile()) {
    return source.toLocalFile();
  }

  return source.toString();
}

int UiSoundPlayer::registerClip(const QUrl &source) {
  const auto path = resolveResourcePath(source);

  if (path.isEmpty()) {
    return -1;
  }

  if (const auto existing = m_clipIdsByPath.constFind(path); existing != m_clipIdsByPath.constEnd()) {
    return existing.value();
  }

  QFile file(path);

  if (!file.open(QIODevice::ReadOnly)) {
    spdlog::warn("UI sound '{}' could not be opened", path.toStdString());
    return -1;
  }

  const auto bytes = file.readAll();
  const auto id = addClipFromBytes(bytes);

  if (id < 0) {
    spdlog::warn("UI sound '{}' could not be decoded", path.toStdString());
    return -1;
  }

  // Kept so the clip can be converted again if the output format changes
  m_clipSources.push_back(bytes);
  m_clipIdsByPath.insert(path, id);

  spdlog::debug("UI sound clip {} registered from '{}'", id, path.toStdString());

  return id;
}

void UiSoundPlayer::play(const int clipId, const qreal gain, const int voices, const qreal pitch) {
  // Without a stream nothing drains the mixer's queue, so a request now would
  // sit there and fire alongside every other one the moment a stream appears —
  // several copies of the same sound starting in the same buffer, in phase
  if (!isRunning()) {
    return;
  }

  m_mixer.play(clipId, static_cast<float>(gain), voices, static_cast<float>(pitch));
}

void UiSoundPlayer::stop(const int clipId) {
  if (!isRunning()) {
    return;
  }

  m_mixer.stop(clipId);
}

bool UiSoundPlayer::isRunning() const { return m_sink != nullptr && m_sink->state() != QtAudio::StoppedState; }

void UiSoundPlayer::onAudioOutputsChanged() {
  // A stopped stream is reopened whatever the list says: an output can be torn down and come back
  // under the same id, which is what a driver restart or a sample rate changed in the system's
  // sound settings looks like from here
  if (isRunning() && selectOutputDevice(m_settingsService).id() == m_deviceId) {
    return;
  }

  m_reopenTimer.start();
}

// TODO
// The stream stops itself when the output it was running on goes away. Nothing else notices: the
// machine's list of outputs is unchanged, the sink is still there, and every sound played into it
// is silently dropped
void UiSoundPlayer::onSinkStateChanged(const QtAudio::State state) {
  if (state != QtAudio::StoppedState || m_sink == nullptr || m_sink->error() == QtAudio::NoError) {
    return;
  }

  spdlog::warn("UI sound stream stopped with error {}; reopening in {}ms", static_cast<int>(m_sink->error()),
               m_reopenBackoffMs);

  m_reopenTimer.setInterval(m_reopenBackoffMs);
  m_reopenBackoffMs = std::min(m_reopenBackoffMs * 2, MAX_REOPEN_BACKOFF_MS);
  m_reopenTimer.start();
}

} // namespace firelight::audio
