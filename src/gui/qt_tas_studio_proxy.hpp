#pragma once

#include "gui/models/piano_roll_model.hpp"

#include <firelight/input/input_frame.hpp>
#include <firelight/tas/tas_emulator.hpp>
#include <firelight/tas/tas_session.hpp>

#include <QObject>

#include <memory>
#include <vector>

class EmulatorItem;

namespace firelight::gui {

// The QML-facing facade for the TAS Studio. Owns a TasSession and its PianoRollModel
// and exposes transport (play / pause / step / seek), input editing, and playhead /
// movie state to QML. The QML TableView binds to `model`; the control bar calls the
// invokables here.
//
// SCAFFOLD SCOPE: so the studio is unit-testable and previewable without a running
// game, this drives a TasSession over an injected ITasEmulator — `attach()` for tests,
// or a self-contained counter emulator via `loadDemo()` for a UI preview. Driving the
// LIVE game means running EmulatorInstance::runFrame through the render-thread
// EmulatorCommand queue; that wiring is deferred (see libs/firelight/tas/README.md).
// Not `final`: qmlRegisterType<> subclasses the registered type internally.
class QtTasStudioProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(firelight::gui::PianoRollModel *model READ model CONSTANT)
  Q_PROPERTY(int currentFrame READ currentFrame NOTIFY playheadChanged)
  Q_PROPERTY(int frameCount READ frameCount NOTIFY movieChanged)
  Q_PROPERTY(int rerecordCount READ rerecordCount NOTIFY movieChanged)
  Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
  Q_PROPERTY(bool hasMovie READ hasMovie NOTIFY movieChanged)
  // True when driving a live game (bindLiveEmulator) rather than the demo session.
  Q_PROPERTY(bool liveMode READ isLiveMode NOTIFY liveModeChanged)
  // True while recording the live game's input into the piano-roll.
  Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
  // True while replaying the recorded movie back through the live game.
  Q_PROPERTY(bool replaying READ isReplaying NOTIFY replayingChanged)

public:
  explicit QtTasStudioProxy(QObject *parent = nullptr);
  ~QtTasStudioProxy() override;

  [[nodiscard]] PianoRollModel *model() const { return m_model; }
  [[nodiscard]] int currentFrame() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int rerecordCount() const;
  [[nodiscard]] bool isPlaying() const { return m_playing; }
  [[nodiscard]] bool hasMovie() const { return m_session != nullptr; }
  [[nodiscard]] bool isLiveMode() const { return m_liveEmulator != nullptr; }
  [[nodiscard]] bool isRecording() const { return m_recording; }
  [[nodiscard]] bool isReplaying() const { return m_replaying; }

  // --- live-game recording (captures real input into the piano-roll) ---
  Q_INVOKABLE void startRecording();
  Q_INVOKABLE void stopRecording();

  // --- live-game replay (plays the recorded movie back through the core) ---
  Q_INVOKABLE void startReplay();
  Q_INVOKABLE void stopReplay();

  // Bind the running game's EmulatorItem so the transport drives it (frame-step via
  // the render-thread command queue) instead of the demo session; binding engages
  // TAS control (pause + pacer gate). Pass null to release control and resume play.
  Q_INVOKABLE void bindLiveEmulator(QObject *emulatorItem);

  // C++ wiring: drive a session over `emu` (borrowed — the caller keeps it alive)
  // replaying `movie`. Used by tests and, later, the live-game integration.
  void attach(tas::ITasEmulator *emu, std::vector<input::InputFrame> movie);

  // --- transport ---
  Q_INVOKABLE void play();
  Q_INVOKABLE void pause();
  Q_INVOKABLE void togglePlay();
  Q_INVOKABLE void stepForward();
  Q_INVOKABLE void stepBackward();
  Q_INVOKABLE void seekTo(int frame);
  // Called once per emulated frame by the driver: advances while playing, auto-pausing
  // at the movie's end.
  Q_INVOKABLE void tick();

  // --- editing (delegates to the model, which routes through the session) ---
  Q_INVOKABLE void toggleInput(int frame, int buttonId);
  Q_INVOKABLE void paintInput(int firstFrame, int lastFrame, int buttonId,
                              bool pressed);

  // A self-contained demo movie so the UI has content without a running game.
  Q_INVOKABLE void loadDemo(int frames = 240);

signals:
  void playheadChanged();
  void movieChanged();
  void playingChanged();
  void liveModeChanged();
  void recordingChanged();
  void replayingChanged();

private:
  void setPlaying(bool playing);
  void afterEmulatorMove(); // refresh model + notify playhead after a step/seek
  // Slot for EmulatorItem::tasFrameRecorded — appends one recorded frame.
  void onFrameRecorded(int frame, int buttons);
  // Slots for EmulatorItem replay signals — move the playhead / end replay.
  void onReplayAdvanced(int frameIndex);
  void onReplayFinished();

  PianoRollModel *m_model; // child QObject
  std::unique_ptr<tas::TasSession> m_session;
  tas::ITasEmulator *m_emu = nullptr;            // borrowed (attach)
  std::unique_ptr<tas::ITasEmulator> m_demoEmu;  // owned (loadDemo)
  bool m_playing = false;

  // Live-game driving (bindLiveEmulator). When set, the transport drives this real
  // game via EmulatorItem::tasStepFrame()/setTasActive() and m_liveFrame counts the
  // frames stepped since binding (there is no authored movie in this mode yet).
  EmulatorItem *m_liveEmulator = nullptr;
  int m_liveFrame = 0;
  bool m_recording = false;
  bool m_replaying = false;
  // The live-game recording shown in the piano-roll (GUI-side mirror; the render
  // thread emits each frame via tasFrameRecorded).
  std::vector<input::InputFrame> m_liveMovie;
};

} // namespace firelight::gui
