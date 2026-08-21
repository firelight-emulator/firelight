#pragma once

#include <QString>
#include <QtGlobal>
#include <mutex>
#include <string>
#include <vector>

namespace firelight::gui {

// TODO
/** A Qt message-handler shim that records QML errors and warnings */
class QmlMessageLog {
public:
  // TODO
  /** One recorded Qt message */
  struct Entry {
    QtMsgType type;
    QString text;
    QString route; // the route being mounted when this arrived; may be empty
    bool fatal;
  };

  // TODO
  /** Installs the handler, chaining to whatever handler is already set */
  static void install();

  // TODO
  /** Removes the handler and restores the previous one */
  static void uninstall();

  // TODO
  /** Treats every warning as fatal, not just the known-bad patterns */
  static void setFatalWarnings(bool fatal);

  // TODO
  /** Tags subsequent messages with this route until it is changed again */
  static void setCurrentRoute(const QString &route);

  // TODO
  /** Every message recorded so far */
  static std::vector<Entry> getEntries();

  // TODO
  /** Messages recorded while the given route was current */
  static std::vector<Entry> getEntriesForRoute(const QString &route);

  // TODO
  /** Drops every recorded message */
  static void clear();

  // TODO
  /** How many recorded messages were classified fatal */
  static int getFatalCount();

private:
  // TODO
  /** Decides whether a message means the UI is broken */
  static bool isFatal(QtMsgType type, const QString &text);

  // TODO
  /** Receives every Qt message, records it, then forwards to the previous handler */
  static void handle(QtMsgType type, const QMessageLogContext &context, const QString &message);

  static inline std::mutex m_mutex;
  static inline std::vector<Entry> m_entries;
  static inline QString m_currentRoute;
  static inline QtMessageHandler m_previous = nullptr;
  static inline bool m_fatalWarnings = false;
  static inline int m_fatalCount = 0;
};

} // namespace firelight::gui
