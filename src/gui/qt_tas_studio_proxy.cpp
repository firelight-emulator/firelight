#include "qt_tas_studio_proxy.hpp"

#include "emulator_item.hpp"

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
  if (m_liveEmulator) {
    return m_liveFrame;
  }
  return m_session ? static_cast<int>(m_session->currentFrame()) : 0;
}

int QtTasStudioProxy::frameCount() const {
  if (m_liveEmulator) {
    return static_cast<int>(m_liveMovie.size()); // recorded frames
  }
  return m_session ? static_cast<int>(m_session->movieLength()) : 0;
}

void QtTasStudioProxy::bindLiveEmulator(QObject *emulatorItem) {
  auto *item = qobject_cast<EmulatorItem *>(emulatorItem);
  if (item == m_liveEmulator) {
    return;
  }
  // Release the previous live game (stop recording/replay, resume normal play).
  if (m_liveEmulator) {
    disconnect(m_liveEmulator, &EmulatorItem::tasFrameRecorded, this,
               &QtTasStudioProxy::onFrameRecorded);
    disconnect(m_liveEmulator, &EmulatorItem::tasReplayAdvanced, this,
               &QtTasStudioProxy::onReplayAdvanced);
    disconnect(m_liveEmulator, &EmulatorItem::tasReplayFinished, this,
               &QtTasStudioProxy::onReplayFinished);
    disconnect(m_liveEmulator, &EmulatorItem::tasSeekFinished, this,
               &QtTasStudioProxy::onSeekFinished);
    m_liveEmulator->tasStopReplay();
    m_liveEmulator->tasStopRecording();
    m_liveEmulator->setTasActive(false);
  }
  m_liveEmulator = item;
  m_liveFrame = 0;
  m_liveMovie.clear();
  m_liveRerecords = 0;
  if (m_recording) {
    m_recording = false;
    emit recordingChanged();
  }
  if (m_replaying) {
    m_replaying = false;
    emit replayingChanged();
  }
  setPlaying(false);
  if (m_liveEmulator) {
    // Engage TAS control (pause + gate the pacer), drop the demo session, and show
    // this game's recording (empty until you Record) in the piano-roll.
    m_liveEmulator->setTasActive(true);
    m_session.reset();
    m_demoEmu.reset();
    // tasFrameRecorded is emitted from the RENDER thread; force a queued connection
    // so onFrameRecorded (which mutates the GUI-thread model) runs on the GUI thread.
    connect(m_liveEmulator, &EmulatorItem::tasFrameRecorded, this,
            &QtTasStudioProxy::onFrameRecorded, Qt::QueuedConnection);
    connect(m_liveEmulator, &EmulatorItem::tasReplayAdvanced, this,
            &QtTasStudioProxy::onReplayAdvanced, Qt::QueuedConnection);
    connect(m_liveEmulator, &EmulatorItem::tasReplayFinished, this,
            &QtTasStudioProxy::onReplayFinished, Qt::QueuedConnection);
    connect(m_liveEmulator, &EmulatorItem::tasSeekFinished, this,
            &QtTasStudioProxy::onSeekFinished, Qt::QueuedConnection);
    m_model->setLiveMovie(&m_liveMovie);
  } else {
    m_model->setLiveMovie(nullptr);
    m_model->setSession(nullptr);
  }
  emit liveModeChanged();
  emit playheadChanged();
  emit movieChanged();
}

void QtTasStudioProxy::onFrameRecorded(int /*frame*/, int buttons) {
  input::InputFrame f;
  f.buttons = static_cast<uint16_t>(buttons);
  m_liveMovie.push_back(f); // queued in order from the render thread
  m_liveFrame = static_cast<int>(m_liveMovie.size());
  // Throttle the view/model to ~10 Hz: committing a row (and re-evaluating the
  // QML bindings) on every one of 60 emulated frames/sec floods the GUI thread.
  if ((m_liveMovie.size() % 6) == 0) {
    m_model->syncLiveMovie();
    emit playheadChanged();
    emit movieChanged();
  }
}

void QtTasStudioProxy::startRecording() {
  if (!m_liveEmulator) {
    return;
  }
  m_liveMovie.clear();
  m_liveFrame = 0;
  m_liveRerecords = 0;
  m_model->resetLiveMovie();
  m_liveEmulator->tasStartRecording(); // unpauses + runs at 1x, capturing frames
  // NOTE: keyboard input follows QML focus, so while the studio is on top a keyboard
  // can't play into the recording (a gamepad works — SDL reads it globally). Giving
  // the game keyboard focus pulls the view back to gameplay, so proper keyboard
  // record needs a split layout (game playable beside the piano-roll) — a follow-up.
  if (!m_recording) {
    m_recording = true;
    emit recordingChanged();
  }
  setPlaying(false);
  emit playheadChanged();
  emit movieChanged();
}

void QtTasStudioProxy::stopRecording() {
  if (m_liveEmulator) {
    m_liveEmulator->tasStopRecording();
    m_liveEmulator->setTasActive(true); // pause so the recording can be reviewed
  }
  if (m_recording) {
    m_recording = false;
    emit recordingChanged();
  }
  m_model->syncLiveMovie(); // commit any frames since the last throttled update
  emit playheadChanged();
  emit movieChanged();
}

void QtTasStudioProxy::startReplay() {
  if (!m_liveEmulator || m_liveMovie.empty() || m_recording) {
    return; // nothing recorded to play back (or still recording)
  }
  m_model->syncLiveMovie(); // make sure every recorded row is committed first
  m_liveFrame = 0;
  m_liveEmulator->tasStartReplay(); // restores anchor + drives recorded input
  if (!m_replaying) {
    m_replaying = true;
    emit replayingChanged();
  }
  setPlaying(false);
  emit playheadChanged();
}

void QtTasStudioProxy::stopReplay() {
  if (m_liveEmulator) {
    m_liveEmulator->tasStopReplay();
    m_liveEmulator->setTasActive(true); // pause where replay left off
  }
  if (m_replaying) {
    m_replaying = false;
    emit replayingChanged();
  }
  emit playheadChanged();
}

void QtTasStudioProxy::onReplayAdvanced(int frameIndex) {
  m_liveFrame = frameIndex;
  // Throttle the playhead/scroll + row highlight to ~10 Hz (as onFrameRecorded).
  if ((frameIndex % 6) == 0) {
    m_model->setLiveCurrentFrame(frameIndex); // highlight follows the replay position
    emit playheadChanged();
  }
}

void QtTasStudioProxy::onReplayFinished() {
  m_liveFrame = static_cast<int>(m_liveMovie.size());
  if (m_replaying) {
    m_replaying = false;
    emit replayingChanged();
  }
  m_model->setLiveCurrentFrame(m_liveFrame); // highlight the final frame's row
  if (m_liveEmulator) {
    m_liveEmulator->setTasActive(true); // pause on the movie's final frame
  }
  emit playheadChanged();
  emit movieChanged();
}

void QtTasStudioProxy::onSeekFinished(int framesEmulated) {
  m_liveFrame = framesEmulated;
  m_model->setLiveCurrentFrame(framesEmulated); // highlight row framesEmulated-1
  emit playheadChanged();
  emit movieChanged();
}

int QtTasStudioProxy::rerecordCount() const {
  if (m_liveEmulator) {
    return m_liveRerecords;
  }
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
  if (m_session || m_liveEmulator) {
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
  if (m_liveEmulator) {
    if (!m_liveMovie.empty() && !m_recording && !m_replaying) {
      // Navigate the recorded movie forward via the greenzone (forward-continue in
      // the renderer makes a single step cost one frame). Stop at the movie's end.
      const int target = m_liveFrame + 1;
      if (target <= static_cast<int>(m_liveMovie.size())) {
        m_liveEmulator->tasSeekTo(target);
      }
      return;
    }
    // No recorded movie yet: nudge the live game forward one frame, reading the
    // controller, via the render-thread command queue.
    m_liveEmulator->tasStepFrame();
    ++m_liveFrame;
    emit playheadChanged();
    emit movieChanged();
    return;
  }
  if (!m_session || currentFrame() >= frameCount()) {
    return; // don't run past the movie's authored input
  }
  m_session->stepForward();
  afterEmulatorMove();
}

void QtTasStudioProxy::stepBackward() {
  if (m_liveEmulator) {
    // Rewind one frame over the live game via the greenzone. Floor at frame 1 (the
    // first displayable frame; frame 0 is the pre-input anchor).
    if (m_recording || m_replaying) {
      return;
    }
    const int target = m_liveFrame - 1;
    if (target < 1) {
      return;
    }
    m_liveEmulator->tasSeekTo(target);
    return;
  }
  if (!m_session || currentFrame() <= 0) {
    return;
  }
  seekTo(currentFrame() - 1);
}

void QtTasStudioProxy::seekTo(int frame) {
  if (m_liveEmulator) {
    if (m_recording || m_replaying || m_liveMovie.empty()) {
      return; // can't seek mid-capture / mid-replay or with nothing recorded
    }
    // Row `frame` -> framesEmulated `frame + 1` (the state that row's input
    // produces). This +1 row->framesEmulated mapping lives ONLY here; the render /
    // greenzone layer works purely in frames-emulated. Floor at 1 (frame 0 is the
    // pre-input anchor, which has no displayable frame on the HW path).
    const int target =
        std::clamp(frame + 1, 1, static_cast<int>(m_liveMovie.size()));
    m_liveEmulator->tasSeekTo(target);
    return;
  }
  if (!m_session) {
    return;
  }
  const int target = std::clamp(frame, 0, frameCount());
  m_session->seekTo(static_cast<uint64_t>(target));
  afterEmulatorMove();
}

void QtTasStudioProxy::tick() {
  if (!m_playing) {
    return;
  }
  if (m_liveEmulator) {
    stepForward(); // continuous live advance while playing
    return;
  }
  if (!m_session) {
    return;
  }
  if (currentFrame() < frameCount()) {
    stepForward();
  } else {
    pause();
  }
}

void QtTasStudioProxy::toggleInput(int frame, int buttonId) {
  if (m_liveEmulator) {
    // Edit the recorded live movie in place. Disallowed mid-capture/replay (the movie
    // is being written / played); a seek in flight is fine — the render side applies
    // the edit immediately and defers the re-simulation to the seek's completion.
    if (m_recording || m_replaying || frame < 0 ||
        frame >= static_cast<int>(m_liveMovie.size()) || buttonId < 0 ||
        buttonId >= 16) {
      return;
    }
    input::InputFrame &f = m_liveMovie[static_cast<size_t>(frame)];
    f.setButton(static_cast<unsigned>(buttonId), !f.button(static_cast<unsigned>(buttonId)));
    ++m_liveRerecords;
    m_model->liveRowChanged(frame); // refresh the toggled cell
    // Propagate to the render-side authoritative movie + greenzone + re-sim; the
    // landed frame comes back via onSeekFinished (updates the playhead/highlight).
    m_liveEmulator->tasEditFrame(frame, static_cast<int>(f.buttons));
    emit movieChanged(); // rerecord count changed
    return;
  }
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
