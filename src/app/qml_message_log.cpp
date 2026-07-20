#include "qml_message_log.hpp"

#include <array>

namespace firelight::gui {

namespace {
// Message fragments that mean a screen is broken. QML type errors and failed
// bindings arrive as QtWarningMsg, so severity alone does not identify them
constexpr std::array FATAL_PATTERNS = {
    "is not a type",   "Cannot assign",         "Unable to assign", "TypeError:",
    "ReferenceError:", "Binding loop detected", "File not found",   "is not defined",
};
} // namespace

void QmlMessageLog::install() {
  std::lock_guard lock(m_mutex);
  if (m_previous == nullptr) {
    m_previous = qInstallMessageHandler(&QmlMessageLog::handle);
  }
}

void QmlMessageLog::uninstall() {
  std::lock_guard lock(m_mutex);
  if (m_previous != nullptr) {
    qInstallMessageHandler(m_previous);
    m_previous = nullptr;
  }
}

void QmlMessageLog::setFatalWarnings(const bool fatal) {
  std::lock_guard lock(m_mutex);
  m_fatalWarnings = fatal;
}

void QmlMessageLog::setCurrentRoute(const QString &route) {
  std::lock_guard lock(m_mutex);
  m_currentRoute = route;
}

std::vector<QmlMessageLog::Entry> QmlMessageLog::getEntries() {
  std::lock_guard lock(m_mutex);
  return m_entries;
}

std::vector<QmlMessageLog::Entry> QmlMessageLog::getEntriesForRoute(const QString &route) {
  std::lock_guard lock(m_mutex);

  std::vector<Entry> matching;
  for (const auto &entry : m_entries) {
    if (entry.route == route) {
      matching.push_back(entry);
    }
  }

  return matching;
}

void QmlMessageLog::clear() {
  std::lock_guard lock(m_mutex);
  m_entries.clear();
  m_fatalCount = 0;
}

int QmlMessageLog::getFatalCount() {
  std::lock_guard lock(m_mutex);
  return m_fatalCount;
}

bool QmlMessageLog::isFatal(const QtMsgType type, const QString &text) {
  if (type == QtCriticalMsg || type == QtFatalMsg) {
    return true;
  }

  if (type != QtWarningMsg) {
    return false;
  }

  if (m_fatalWarnings) {
    return true;
  }

  for (const auto *pattern : FATAL_PATTERNS) {
    if (text.contains(QLatin1String(pattern))) {
      return true;
    }
  }

  return false;
}

void QmlMessageLog::handle(const QtMsgType type, const QMessageLogContext &context, const QString &message) {
  QtMessageHandler previous = nullptr;
  {
    std::lock_guard lock(m_mutex);
    const auto fatal = isFatal(type, message);
    m_entries.push_back({type, message, m_currentRoute, fatal});
    if (fatal) {
      ++m_fatalCount;
    }
    previous = m_previous;
  }

  if (previous != nullptr) {
    previous(type, context, message);
  }
}

} // namespace firelight::gui
