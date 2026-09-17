// TODO: NEEDS REVIEW
#include "audio/qt_microphone.hpp"

#include <firelight/audio/pcm_ring.hpp>

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QCoreApplication>
#include <QMediaDevices>
#include <QObject>
#include <QSpan>
#include <QThread>
#include <atomic>
#include <spdlog/spdlog.h>

namespace {
constexpr int PREFERRED_RATE = 44100;

// TODO
// How much captured sound is held for the core before the oldest is lost
constexpr size_t RING_SECONDS = 1;
} // namespace

// Completes the opaque libretro handle with the capture state
struct retro_microphone {
  std::shared_ptr<firelight::audio::PcmRing> ring;
  QAudioDevice device;
  QAudioFormat format;
  unsigned rate = 0;
  std::atomic<bool> active{false};
};

namespace firelight::audio {

/**
 * Owns the capture source on the GUI thread and pushes everything it hears into a ring
 */
class MicrophoneStream : public QObject {
public:
  MicrophoneStream() = default;

  ~MicrophoneStream() override { m_source.reset(); }

  /**
   * Starts capturing into ring, replacing any capture already running. Any thread
   */
  void start(const QAudioDevice &device, const QAudioFormat &format, std::shared_ptr<PcmRing> ring) {
    QMetaObject::invokeMethod(
        this,
        [this, device, format, ring = std::move(ring)] {
          m_source.reset();
          m_source = std::make_unique<QAudioSource>(device, format);
          m_source->start([ring](QSpan<const int16_t> heard) { ring->push(heard.data(), heard.size()); });

          if (m_source->error() != QtAudio::NoError) {
            spdlog::warn("[Microphone] Capture failed to start (error {})", static_cast<int>(m_source->error()));
            m_source.reset();
          }
        },
        Qt::QueuedConnection);
  }

  /**
   * Stops capturing. Any thread
   */
  void stop() {
    QMetaObject::invokeMethod(this, [this] { m_source.reset(); }, Qt::QueuedConnection);
  }

private:
  std::unique_ptr<QAudioSource> m_source;
};

QtMicrophone::QtMicrophone() {
  m_stream = new MicrophoneStream();

  if (const auto *app = QCoreApplication::instance(); app && app->thread() != QThread::currentThread()) {
    m_stream->moveToThread(app->thread());
  }
}

QtMicrophone::~QtMicrophone() {
  if (QThread::currentThread() == m_stream->thread()) {
    delete m_stream;
  } else {
    m_stream->deleteLater();
  }
}

retro_microphone_t *QtMicrophone::openMicrophone() {
  if (m_mic) {
    return m_mic.get();
  }

  const QAudioDevice device = QMediaDevices::defaultAudioInput();

  if (device.isNull()) {
    spdlog::warn("[Microphone] No audio input device available");
    return nullptr;
  }

  QAudioFormat format;
  format.setSampleRate(PREFERRED_RATE);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Int16);

  if (!device.isFormatSupported(format)) {
    format = device.preferredFormat();
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
  }

  auto mic = std::make_unique<retro_microphone>();
  mic->device = device;
  mic->format = format;
  mic->rate = static_cast<unsigned>(format.sampleRate());
  mic->ring = std::make_shared<PcmRing>(static_cast<size_t>(format.sampleRate()) * RING_SECONDS);
  m_mic = std::move(mic);
  return m_mic.get();
}

void QtMicrophone::closeMicrophone(retro_microphone_t *microphone) {
  if (!microphone || microphone != m_mic.get()) {
    return;
  }

  m_stream->stop();
  m_mic.reset();
}

bool QtMicrophone::getMicrophoneParameters(const retro_microphone_t *microphone, retro_microphone_params_t *params) {
  if (!microphone || !params) {
    return false;
  }

  params->rate = microphone->rate;
  return true;
}

bool QtMicrophone::setMicrophoneState(retro_microphone_t *microphone, bool state) {
  if (!microphone) {
    return false;
  }

  if (state == microphone->active) {
    return true;
  }

  if (state) {
    microphone->ring->discardAll();
    m_stream->start(microphone->device, microphone->format, microphone->ring);
  } else {
    m_stream->stop();
  }

  microphone->active = state;
  return true;
}

bool QtMicrophone::getMicrophoneState(const retro_microphone_t *microphone) { return microphone && microphone->active; }

int QtMicrophone::readMicrophone(retro_microphone_t *microphone, int16_t *samples, size_t num_samples) {
  if (!microphone || !microphone->active || !samples) {
    return 0;
  }

  return static_cast<int>(microphone->ring->pop(samples, num_samples));
}

} // namespace firelight::audio
