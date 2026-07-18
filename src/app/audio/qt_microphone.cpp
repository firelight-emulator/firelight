#include "audio/qt_microphone.hpp"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QIODevice>
#include <QMediaDevices>
#include <spdlog/spdlog.h>

// Completes the opaque libretro handle with the Qt capture state
struct retro_microphone {
  std::unique_ptr<QAudioSource> source;
  QIODevice *io = nullptr;
  unsigned rate = 0;
  bool active = false;
};

namespace firelight::audio {

QtMicrophone::QtMicrophone() = default;
QtMicrophone::~QtMicrophone() = default;

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
  format.setSampleRate(44100);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Int16);
  if (!device.isFormatSupported(format)) {
    format = device.preferredFormat();
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
  }

  auto mic = std::make_unique<retro_microphone>();
  mic->source = std::make_unique<QAudioSource>(device, format);
  mic->rate = format.sampleRate();
  m_mic = std::move(mic);
  return m_mic.get();
}

void QtMicrophone::closeMicrophone(retro_microphone_t *microphone) {
  if (!microphone || microphone != m_mic.get()) {
    return;
  }
  if (microphone->source) {
    microphone->source->stop();
  }
  m_mic.reset();
}

bool QtMicrophone::getMicrophoneParameters(const retro_microphone_t *microphone,
                                           retro_microphone_params_t *params) {
  if (!microphone || !params) {
    return false;
  }
  params->rate = microphone->rate;
  return true;
}

bool QtMicrophone::setMicrophoneState(retro_microphone_t *microphone,
                                      bool state) {
  if (!microphone || !microphone->source) {
    return false;
  }
  if (state == microphone->active) {
    return true;
  }
  if (state) {
    microphone->io = microphone->source->start();
    microphone->active = microphone->io != nullptr;
    return microphone->active;
  }
  microphone->source->stop();
  microphone->io = nullptr;
  microphone->active = false;
  return true;
}

bool QtMicrophone::getMicrophoneState(const retro_microphone_t *microphone) {
  return microphone && microphone->active;
}

int QtMicrophone::readMicrophone(retro_microphone_t *microphone,
                                 int16_t *samples, size_t num_samples) {
  if (!microphone || !microphone->active || !microphone->io || !samples) {
    return 0;
  }
  const qint64 wanted = static_cast<qint64>(num_samples) * sizeof(int16_t);
  const qint64 read =
      microphone->io->read(reinterpret_cast<char *>(samples), wanted);
  if (read < 0) {
    return -1;
  }
  return static_cast<int>(read / sizeof(int16_t));
}

} // namespace firelight::audio
