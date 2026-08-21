#pragma once

#include <QObject>
#include <QString>

namespace firelight::cli {

// Startup values derived from the command line, exposed to QML as a context
// property ("StartupOptions") so the root window can act on them once loaded
// Values are fixed at construction (set before the QML engine loads)
class StartupOptions : public QObject {
  Q_OBJECT
  // A library entry id to auto-launch on startup, or -1 for none
  Q_PROPERTY(int launchEntryId READ launchEntryId CONSTANT)
  // -1 = unset (use the saved preference), 0 = windowed, 1 = fullscreen
  Q_PROPERTY(int fullscreenOverride READ fullscreenOverride CONSTANT)
  // Quit the app when the launched game closes (external-launcher mode)
  Q_PROPERTY(bool exitOnClose READ exitOnClose CONSTANT)
  // A RetroAchievements login the QML coordinator should perform before the
  // game launches (from the `--ra-*` startup flags)
  Q_PROPERTY(bool raPendingLogin READ raPendingLogin CONSTANT)
  Q_PROPERTY(QString raUsername READ raUsername CONSTANT)
  Q_PROPERTY(QString raPassword READ raPassword CONSTANT)
  Q_PROPERTY(QString raToken READ raToken CONSTANT)
  // The route to open instead of the default, or empty for the default
  Q_PROPERTY(QString startupRoute READ startupRoute CONSTANT)

public:
  struct Data {
    int launchEntryId = -1;
    QString startupRoute;
    bool startMuted = false;
    bool startPaused = false;
    int fullscreenOverride = -1;
    bool exitOnClose = false;
    bool raPendingLogin = false;
    QString raUsername;
    QString raPassword;
    QString raToken;
  };

  explicit StartupOptions(Data data, QObject *parent = nullptr) : QObject(parent), m_data(std::move(data)) {}

  [[nodiscard]] int launchEntryId() const { return m_data.launchEntryId; }

  [[nodiscard]] int fullscreenOverride() const { return m_data.fullscreenOverride; }

  [[nodiscard]] bool exitOnClose() const { return m_data.exitOnClose; }

  // One-shot getters for the per-game start knobs: they return the requested
  // value the first time and false afterwards, so only the CLI-launched game
  // (the first NewEmulatorPage created) picks them up
  Q_INVOKABLE bool consumeStartMuted() {
    const bool v = m_data.startMuted;
    m_data.startMuted = false;
    return v;
  }

  Q_INVOKABLE bool consumeStartPaused() {
    const bool v = m_data.startPaused;
    m_data.startPaused = false;
    return v;
  }

  [[nodiscard]] bool raPendingLogin() const { return m_data.raPendingLogin; }

  [[nodiscard]] QString raUsername() const { return m_data.raUsername; }

  [[nodiscard]] QString raPassword() const { return m_data.raPassword; }

  [[nodiscard]] QString raToken() const { return m_data.raToken; }

  [[nodiscard]] QString startupRoute() const { return m_data.startupRoute; }

private:
  Data m_data;
};

} // namespace firelight::cli
