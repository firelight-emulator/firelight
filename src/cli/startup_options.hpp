#pragma once

#include <QObject>

namespace firelight::cli {

// Startup values derived from the command line, exposed to QML as a context
// property ("StartupOptions") so the root window can act on them once loaded.
// Values are fixed at construction (set before the QML engine loads).
class StartupOptions : public QObject {
  Q_OBJECT
  // A library entry id to auto-launch on startup, or -1 for none.
  Q_PROPERTY(int launchEntryId READ launchEntryId CONSTANT)

public:
  explicit StartupOptions(int launchEntryId, QObject *parent = nullptr)
      : QObject(parent), m_launchEntryId(launchEntryId) {}

  [[nodiscard]] int launchEntryId() const { return m_launchEntryId; }

private:
  int m_launchEntryId;
};

} // namespace firelight::cli
