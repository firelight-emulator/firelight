#pragma once
#include <QObject>

namespace firelight::gui {
class FilesystemUtils : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE bool showInFilesystem(const QString &path);
};
} // namespace firelight::gui