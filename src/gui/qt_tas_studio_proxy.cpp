#include "qt_tas_studio_proxy.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace firelight::gui {

namespace {
// A trivial deterministic emulator backing loadDemo(): state is a frame counter, so
// the greenzone can serialize/restore keyframes. There is no real game — this only
// lets the piano-roll show frames during UI development.
class DemoEmulator final : public tas::ITasEmulator {
public:
  void setRetropadProvider(libretro::IRetropadProvider *) override {}
  void runFrame() override { ++m_counter; }
  std::vector<uint8_t> serializeState() override {
    std::vector<uint8_t> s(sizeof(m_counter));
    std::memcpy(s.data(), &m_counter, sizeof(m_counter));
    return s;
  }
  bool deserializeState(const std::vector<uint8_t> &s) override {
    if (s.size() != sizeof(m_counter)) {
      return false;
    }
    std::memcpy(&m_counter, s.data(), sizeof(m_counter));
    return true;
  }

private:
  uint64_t m_counter = 0;
};
} // namespace

QtTasStudioProxy::QtTasStudioProxy(QObject *parent)
    : QObject(parent), m_model(new PianoRollModel(this)) {}

QtTasStudioProxy::~QtTasStudioProxy() = default;

int QtTasStudioProxy::currentFrame() const {
  return m_session ? static_cast<int>(m_session->currentFrame()) : 0;
}

int QtTasStudioProxy::frameCount() const {
  return m_session ? static_cast<int>(m_session->movieLength()) : 0;
}

int QtTasStudioProxy::rerecordCount() const {
  return m_session ? static_cast<int>(m_session->rerecordCount()) : 0;
}

void QtTasStudioProxy::attach(tas::ITasEmulator *emu,
                              std::vector<input::InputFrame> movie) {
  if (emu == nullptr) {
    return;
  }
  m_emu = emu;
  m_session = std::make_unique<tas::TasSession>(*emu);
  m_session->loadMovie(std::move(movie));
  m_model->setSession(m_session.get());
  setPlaying(false);
  emit playheadChanged();
  emit movieChanged();
}

void QtTasStudioProxy::setPlaying(bool playing) {
  if (m_playing == playing) {
    return;
  }
  m_playing = playing;
  emit playingChanged();
}

void QtTasStudioProxy::play() {
  if (m_session) {
    setPlaying(true);
  }
}

void QtTasStudioProxy::pause() { setPlaying(false); }

void QtTasStudioProxy::togglePlay() {
  if (m_playing) {
    pause();
  } else {
    play();
  }
}

void QtTasStudioProxy::afterEmulatorMove() {
  m_model->refresh();
  emit playheadChanged();
}

void QtTasStudioProxy::stepForward() {
  if (!m_session || currentFrame() >= frameCount()) {
    return; // don't run past the movie's authored input
  }
  m_session->stepForward();
  afterEmulatorMove();
}

void QtTasStudioProxy::stepBackward() {
  if (!m_session || currentFrame() <= 0) {
    return;
  }
  seekTo(currentFrame() - 1);
}

void QtTasStudioProxy::seekTo(int frame) {
  if (!m_session) {
    return;
  }
  const int target = std::clamp(frame, 0, frameCount());
  m_session->seekTo(static_cast<uint64_t>(target));
  afterEmulatorMove();
}

void QtTasStudioProxy::tick() {
  if (!m_playing || !m_session) {
    return;
  }
  if (currentFrame() < frameCount()) {
    stepForward();
  } else {
    pause();
  }
}

void QtTasStudioProxy::toggleInput(int frame, int buttonId) {
  if (!m_session) {
    return;
  }
  m_model->toggleButton(frame, buttonId);
  // The edit may have pulled the playhead back to the edit point.
  emit playheadChanged();
  emit movieChanged();
}

void QtTasStudioProxy::paintInput(int firstFrame, int lastFrame, int buttonId,
                                  bool pressed) {
  if (!m_session) {
    return;
  }
  m_model->paintButton(firstFrame, lastFrame, buttonId, pressed);
  emit playheadChanged();
  emit movieChanged();
}

void QtTasStudioProxy::loadDemo(int frames) {
  const int n = std::max(1, frames);
  m_demoEmu = std::make_unique<DemoEmulator>();
  // A recognizable pattern: a D-pad "staircase" plus periodic A presses, so the
  // grid clearly shows distinct rows.
  std::vector<input::InputFrame> movie;
  movie.reserve(static_cast<size_t>(n));
  for (int k = 0; k < n; ++k) {
    input::InputFrame f;
    f.setButton(static_cast<unsigned>(4 + (k / 8) % 4), true); // UP/DOWN/LEFT/RIGHT
    if (k % 16 == 0) {
      f.setButton(8, true); // A
    }
    movie.push_back(f);
  }
  attach(m_demoEmu.get(), std::move(movie));
}

} // namespace firelight::gui
