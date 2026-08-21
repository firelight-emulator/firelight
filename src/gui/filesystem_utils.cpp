#include "filesystem_utils.hpp"

#include <QFile>
#include <QUrl>
#include <qdir.h>
#include <qprocess.h>

namespace firelight::gui {

QString FilesystemUtils::getFileURI() {
  // See https://en.wikipedia.org/wiki/File_URI_scheme for a vague explanation
#ifdef __APPLE__
  return "file://";
#elif __linux__
  return "file://";
#elif _WIN32
  return "file:///";
#endif
}

QString FilesystemUtils::readTextFile(const QString &url) {
  // Resolve qrc:/ and file:// URLs to something QFile understands; a plain path
  // (including a ":/..." resource path) is used as-is
  QString path = url;
  if (const QUrl parsed(url); parsed.scheme() == "qrc") {
    path = ":" + parsed.path();
  } else if (parsed.isLocalFile()) {
    path = parsed.toLocalFile();
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }
  return QString::fromUtf8(file.readAll());
}

bool FilesystemUtils::isFile(const QString &path) {
  if (path.isEmpty()) {
    return false;
  }
  auto fileURI = getFileURI();
  return path.startsWith(fileURI);
}

QString FilesystemUtils::prependFileURI(const QString &path) {
  auto fileURI = getFileURI();
  return fileURI + path;
}

QString FilesystemUtils::removeFileURI(const QString &path) {
  auto fileURI = getFileURI();
  return QString(path).replace(fileURI, "");
}

bool FilesystemUtils::showInFilesystem(const QString &path) {
  const QFileInfo fileInfo(path);
#ifdef _WIN32
  QStringList param;
  if (!fileInfo.isDir()) {
    param += QLatin1String("/select,");
  }

  param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
  QProcess::startDetached("explorer.exe", param);
#endif

#ifdef __APPLE__
  QStringList scriptArgs;
  scriptArgs << QLatin1String("-e")
             << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                    .arg(fileInfo.canonicalFilePath());
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
  scriptArgs.clear();
  scriptArgs << QLatin1String("-e") << QLatin1String("tell application \"Finder\" to activate");
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#endif
  return true;
}
} // namespace firelight::gui
