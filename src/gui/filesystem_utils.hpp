#pragma once
#include <QObject>

namespace firelight::gui {
class FilesystemUtils : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE static QString getFileURI();
  Q_INVOKABLE static bool isFile(const QString &path);
  Q_INVOKABLE static QString prependFileURI(const QString &path);
  Q_INVOKABLE static QString removeFileURI(const QString &path);
  Q_INVOKABLE bool showInFilesystem(const QString &path);
};
} // namespace firelight::gui
