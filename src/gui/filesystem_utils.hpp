#pragma once
#include <QObject>

namespace firelight::gui {
class FilesystemUtils : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE QString getFileURI();
  Q_INVOKABLE bool isFile(const QString &path);
  Q_INVOKABLE QString prependFileURI(const QString &path);
  Q_INVOKABLE QString removeFileURI(const QString &path);
  Q_INVOKABLE bool showInFilesystem(const QString &path);
};
} // namespace firelight::gui
