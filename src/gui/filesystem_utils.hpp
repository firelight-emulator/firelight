#pragma once
#include <QObject>

namespace firelight::gui {
class FilesystemUtils : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE static QString getFileURI();
  // TODO
  // Reads a text file into a UTF-8 string. Accepts a plain path, a file:// URL,
  // or a qrc:/ resource URL; returns "" on failure. Used to load bundled help
  // articles for Markdown rendering
  Q_INVOKABLE static QString readTextFile(const QString &url);
  Q_INVOKABLE static bool isFile(const QString &path);
  Q_INVOKABLE static QString prependFileURI(const QString &path);
  Q_INVOKABLE static QString removeFileURI(const QString &path);
  Q_INVOKABLE bool showInFilesystem(const QString &path);
};
} // namespace firelight::gui
