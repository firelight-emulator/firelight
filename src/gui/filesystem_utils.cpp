#include "filesystem_utils.hpp"

#include <qdir.h>
#include <qprocess.h>

namespace firelight::gui {

QString FilesystemUtils::getFileURI() {
  // See https://en.wikipedia.org/wiki/File_URI_scheme for a vague explanation
#ifdef Q_OS_UNIX
  return "file://";
#elif Q_OS_WIN
  return "file:///";
#endif
}

bool FilesystemUtils::isFile(const QString &path) {
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
#ifdef Q_OS_WIN
  QStringList param;
  if (!fileInfo.isDir()) {
    param += QLatin1String("/select,");
  }

  param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
  QProcess::startDetached("explorer.exe", param);
#endif

#ifdef Q_OS_MACOS
  QStringList scriptArgs;
  scriptArgs << QLatin1String("-e")
             << QString::fromLatin1(
                    "tell application \"Finder\" to reveal POSIX file \"%1\"")
                    .arg(fileInfo.canonicalFilePath());
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
  scriptArgs.clear();
  scriptArgs << QLatin1String("-e")
             << QLatin1String("tell application \"Finder\" to activate");
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#endif
  // else {
  //   // we cannot select a file here, because no file browser really supports
  //   // it...
  //   const QString folder =
  //       fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
  //   const QString app = UnixUtils::fileBrowser(ICore::settings());
  //   QProcess browserProc;
  //   const QString browserArgs =
  //       UnixUtils::substituteFileBrowserParameters(app, folder);
  //   bool success = browserProc.startDetached(browserArgs);
  //   const QString error =
  //       QString::fromLocal8Bit(browserProc.readAllStandardError());
  //   success = success && error.isEmpty();
  //   if (!success)
  //     showGraphicalShellError(parent, app, error);
  return true;
}
} // namespace firelight::gui
